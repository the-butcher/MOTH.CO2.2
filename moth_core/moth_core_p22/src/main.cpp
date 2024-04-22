#include <Arduino.h>
#include <Wire.h>
#include <driver/rtc_io.h>
#include <esp_wifi.h>

#include "buttons/ButtonAction.h"
#include "modules/ModuleCard.h"
#include "modules/ModuleDisp.h"
#include "modules/ModuleMqtt.h"
#include "modules/ModuleSignal.h"
#include "modules/ModuleWifi.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"
#include "sensors/SensorTime.h"
#include "types/Action.h"
#include "types/Config.h"
#include "types/Define.h"
#include "types/Device.h"
#include "types/Values.h"

RTC_DATA_ATTR setup_mode_t setupMode = SETUP_BOOT;
RTC_DATA_ATTR uint32_t secondstimeBoot;

// configuration and display state
RTC_DATA_ATTR config_t config;  // size: ~108

// last hour of measurements and associated indices
RTC_DATA_ATTR values_t values;  // size: ~1212

// device state
RTC_DATA_ATTR device_t device;  // size: ~108

// does not need to be RTC_DATA_ATTR, reset to 0 does not matter
uint16_t actionNum = 0;

const gpio_num_t PIN_PKK2_A = GPIO_NUM_16;

// uint16_t sc = sizeof(values);

/**
 * OK reimplement OTA update, TODO :: test
 * OK mqtt (+autoconnect for mqtt)
 *    !! can not reconnect after a number of connections, mosquitto or device problem, TODO :: analyze SSL error from mosquitto log?
 *    !! how to publish historic data from file?
 * -- add more info to status (maybe config needs to be made public after all)
 * ?? create series with 10sec, 5sec, 3sec, 0sec warmup and check for value deviation, pick an energy/precision tradeoff
 * ?? better server UI - offer the option to get data for a defined range (load file by file, then concat on the client and download)
 * -- ISSUE: regular crashes when powering wifi, rarely wifi is permantly broken afterwards (until wifi dat gets deleted)
 *    -- orruption of the wifi.dat may be a problem
 *    -- if no cause can be found for wifi.dat corruption -> delete dat upon connection failure
 *    -- it seems, even though strange, that this happens more often when the device was outside and is cold
 * -- ISSUE: status does not reliably provide correct SCD41 state (maybe depending on I2C power status)
 * -- update server files (co2tst to be removed, display modus calib needs to be added)
 *
 */

// schedule setting and display
void scheduleDeviceActionSetting() {
    device.actionIndexMax = DEVICE_ACTION_DEPOWER;  // allow actions display and depower by index
    uint32_t secondstime = SensorTime::getSecondstime();
    uint32_t secondswait = 60 - secondstime % 60;
    if (device.actionIndexCur == DEVICE_ACTION_POWERUP && secondswait > WAITTIME_DISPLAY_AND_DEPOWER) {
        device.actionIndexCur = DEVICE_ACTION_SETTING;
        device.deviceActions[DEVICE_ACTION_SETTING].secondsNext = SensorTime::getSecondstime();  // assign current time as due time
    }
}

void scheduleDeviceActionPowerup() {
    // TODO :: only if nothing else (calibration, ... ) is pending
    device.actionIndexCur = DEVICE_ACTION_POWERUP;
    device.deviceActions[DEVICE_ACTION_POWERUP].secondsNext = SensorTime::getSecondstime();
}

/**
 * handle a detected button action (pin and type)
 */
void handleButtonActionComplete(std::function<void(config_t& config)> actionFunction) {
    if (actionFunction != nullptr) {
        ModuleSignal::beep();           // indicate completed button action
        actionFunction(config);         // execute the action (which, by convention, only alters the config to not interfere with program flow)
        scheduleDeviceActionSetting();  // schedule DEVICE_ACTION_SETTING and DEVICE_ACTION_DISPLAY
    }
    actionNum++;  // interrupt delay loop
}

void scheduleDeviceActionDepower() {
    device.actionIndexCur = DEVICE_ACTION_DEPOWER;
    device.deviceActions[DEVICE_ACTION_DEPOWER].secondsNext = SensorTime::getSecondstime();  // need to reset, or it will start waiting for DEPOWER again due to secondsNext
    actionNum++;                                                                             // interrupt delay loop
}

void handleWakeupCause() {
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    if (wakeupReason == ESP_SLEEP_WAKEUP_EXT0) {
        scheduleDeviceActionDepower();
    } else if (wakeupReason == ESP_SLEEP_WAKEUP_EXT1) {
        uint64_t wakeupStatus = esp_sleep_get_ext1_wakeup_status();
        gpio_num_t wakeupPin = (gpio_num_t)(log(wakeupStatus) / log(2));
        if (wakeupPin == PIN_RTC_SQW) {  // wakeup from pin other than the rtc pulse, must be a button press
            scheduleDeviceActionPowerup();
        } else {
            ButtonAction::createButtonAction(wakeupPin);
        }
    }
}

void setup() {
    esp_log_level_set("*", ESP_LOG_ERROR);

    rtc_gpio_deinit(PIN_PKK2_A);
    pinMode(PIN_PKK2_A, OUTPUT);
    digitalWrite(PIN_PKK2_A, HIGH);

    // turn on I2C power
    rtc_gpio_deinit((gpio_num_t)I2C_POWER);
    pinMode(I2C_POWER, OUTPUT);
    digitalWrite(I2C_POWER, HIGH);

    Wire.begin();

#ifdef USE___SERIAL
    Serial.begin(115200);
    delay(3000);
#endif

    // must have begun to set color
    ModuleSignal::begin();
    ModuleSignal::setPixelColor(COLOR___YELLOW);

    // must have begun before calling setup
    SensorTime::begin();

    // must have begun to be configured
    SensorScd041::begin();
    SensorBme280::begin();
    SensorEnergy::begin();

    if (setupMode == SETUP_BOOT) {

        delay(1000);  // SensorTime appears to take some time to initialize, especially on boot

        config = Config::load();
        values = Values::load();
        device = Device::load();

        SensorTime::configure(config);  // enables the 1 minute timer
        ModuleWifi::configure(config);  // recreate the wifi data file, this will also call the initial ModuleSdCard begin
        ModuleMqtt::configure(config);  // recreate the mqtt data file
        ModuleDisp::configure(config);  // load display config and apply to config, must be configured prior to SensorScd041 to have correct temperature offset
        SensorScd041::configure(config);

        // have a battery measurement for the entry screen
        SensorEnergy::powerup();
        SensorEnergy::measure();
        SensorEnergy::depower();

        setupMode = SETUP_MAIN;
        secondstimeBoot = SensorTime::getSecondstime();
    }

    // enable global access to values
    Config::begin(&config);
    Values::begin(&values);
    Device::begin(secondstimeBoot);

    // show the correct button hints and have callback in place
    ButtonAction::begin(handleButtonActionComplete);
    ButtonAction::adapt(config);

    // only sets the callback, but does not powerup anything
    ModuleDisp::begin();

    // there can be multiple causes for wakeup, RTC_SQW pin, BUSY pin, Button Pins
    handleWakeupCause();
}

/**
 * check if the device needs to stay awake
 * - while a button is pressed
 * - while actionPin is set (action is still active)
 * - while WiFi is on
 * - for debugging purposes
 */
bool isDelayRequired() {
#ifdef USE____DELAY
    return true;
#endif
    return ButtonAction::getActionPin() > 0 || ButtonAction::getPressedPin() > 0 || ModuleWifi::isPowered();
}

void secondsSleep(uint32_t seconds) {
    // convert to microseconds
    uint64_t sleepMicros = seconds * MICROSECONDS_PER______SECOND;

    esp_deep_sleep_disable_rom_logging();  // seems to have no effect
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

    wakeup_action_e wakeupType = device.deviceActions[device.actionIndexCur].wakeupType;

    ModuleSignal::prepareSleep();            // this may hold the neopixel power, depending on color debug setting
    ButtonAction::prepareSleep(wakeupType);  // establishes gpio holds for the button pins, ...
    SensorTime::prepareSleep(wakeupType);    // establishes gpio hold for the RTC_SQW pin, ...
    ModuleDisp::prepareSleep(wakeupType);    // adds ext0Wakeup (wait for busy pin high)

    if (wakeupType == WAKEUP_ACTION_BUTN) {
        esp_sleep_enable_ext1_wakeup(device.ext1Bitmask, ESP_EXT1_WAKEUP_ANY_LOW);
    }

    if (device.actionIndexCur == DEVICE_ACTION_POWERUP) {  // next action is DEVICE_ACTION_POWERUP, therfore no timer wakeup, RTC pulse will take care of wakeup
        pinMode(I2C_POWER, OUTPUT);
        digitalWrite(I2C_POWER, LOW);
        gpio_hold_dis((gpio_num_t)I2C_POWER);
        // lowpowerperiodic
        // esp_sleep_enable_timer_wakeup(sleepMicros);
        // gpio_hold_en((gpio_num_t)I2C_POWER);  // power needs to be help or sensors will not measure
    } else {
        esp_sleep_enable_timer_wakeup(sleepMicros);
        gpio_hold_en((gpio_num_t)I2C_POWER);  // power needs to be help or sensors will not measure
    }

    Wire.end();
    digitalWrite(PIN_PKK2_A, LOW);

    // https://github.com/espressif/arduino-esp32/issues/3363
    // pinMode(SDA, INPUT);  // needed because Wire.end() enables pullups, power Saving
    // pinMode(SCL, INPUT);

    // adc_power_release();
    // esp_wifi_stop();

    ModuleSignal::setPixelColor(COLOR____OCEAN);
    esp_deep_sleep_start();  // waiting for powerup (up to ~50 seconds) -> deep sleep
}

/**
 * wait with short delays until any of the conditions below become true
 * - the specified seconds have elapsed
 * - isDelayRequired() becomes false
 * - actionNumEntry has changed (some button action may have completed)
 */
void secondsDelay(uint32_t seconds) {

    uint32_t millisEntry = millis();
    uint32_t millisBreak = millisEntry + seconds * MILLISECONDS_PER______SECOND;
    uint16_t actionNumEntry = actionNum;

    wakeup_action_e wakeupType = device.deviceActions[device.actionIndexCur].wakeupType;

    ButtonAction::attachWakeup(wakeupType);  // button interrupts
    ModuleDisp::attachWakeup(wakeupType);    // wait for busy pin
    SensorTime::attachWakeup(wakeupType);    // SQW interrupt

    ModuleSignal::setPixelColor(COLOR_____CYAN);
    while (isDelayRequired() && millis() < millisBreak && actionNumEntry == actionNum) {

        delay(50);

        // the 1 minute measure pin
        if (SensorTime::isInterrupted()) {
            scheduleDeviceActionPowerup();
            break;
        }

        // the display's busy pin
        if (ModuleDisp::isInterrupted()) {
            scheduleDeviceActionDepower();
            break;
        }

        // wifi expiry
        uint32_t secondsdest = ModuleWifi::getSecondstimeExpiry();
        uint32_t secondstime = SensorTime::getSecondstime();
        uint32_t secondswait = secondsdest > secondstime ? secondsdest - secondstime : WAITTIME________________NONE;
        if (secondswait == WAITTIME________________NONE && config.wifi.wifiValPower == WIFI____VAL_P__CUR_Y) {
            ModuleSignal::beep();
            config.wifi.wifiValPower = WIFI____VAL_P__PND_N;  // set flag to pending off
            scheduleDeviceActionSetting();
            break;
        }

        // anything ModuleHttp wants to configure (display aspects, co2 calibration, co2 reset, co2 power mode)
        if (ModuleHttp::requestedReconfiguration != nullptr) {
            ModuleSignal::beep();
            ModuleHttp::requestedReconfiguration(config, values);
            ButtonAction::adapt(config);  // be sure buttons reflect potentially altered configuration
            scheduleDeviceActionSetting();
            ModuleHttp::requestedReconfiguration = nullptr;
            break;
        }
    }

    ButtonAction::detachWakeup(wakeupType);
    ModuleDisp::detachWakeup(wakeupType);
    SensorTime::detachWakeup(wakeupType);
}

void loop() {
    device_action_t action = device.deviceActions[device.actionIndexCur];
    if (SensorTime::getSecondsUntil(action.secondsNext) == WAITTIME________________NONE) {  // action is due
        ModuleSignal::setPixelColor(action.color);
        device.actionIndexCur = Device::getFunctionByAction(action.type)(config, device.actionIndexMax);  // execute the action and see whats coming next
        if (device.actionIndexCur == DEVICE_ACTION_POWERUP) {
            device.actionIndexMax = values.nextMeasureIndex == values.nextDisplayIndex ? DEVICE_ACTION_DEPOWER : DEVICE_ACTION_READVAL;
            device.deviceActions[DEVICE_ACTION_POWERUP].secondsNext = SensorTime::getSecondstime() + SECONDS_PER_____________HOUR;
        } else {
            device.deviceActions[device.actionIndexCur].secondsNext = SensorTime::getSecondstime() + action.secondsWait;
        }
        delay(10);
    }
    uint32_t secondsWait = SensorTime::getSecondsUntil(device.deviceActions[device.actionIndexCur].secondsNext);
    if (secondsWait > WAITTIME________________NONE) {
        if (isDelayRequired()) {
            secondsDelay(secondsWait);
        } else {
            secondsSleep(secondsWait);
        }
    }
}
