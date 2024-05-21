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
RTC_DATA_ATTR config_t config;  // size: ~140

// last hour of measurements and associated indices
RTC_DATA_ATTR values_t values;  // size: ~1220

// device state
RTC_DATA_ATTR device_t device;  // size: ~136

// does not need to be RTC_DATA_ATTR, reset to 0 does not matter
uint16_t actionNum = 0;

// uint16_t sc = sizeof(device);

/**
 * -- MQTT:
 *    -- occasional timeouts in mosquitto log
 * -- WIFI:
 *    -- wifi sometimes turns off for unknown reasons (more likely when multiple requests are pending),
 * -- MISC:
 *    -- be sure data is written exactly every 60 minutes (not 59 or 61)
 *    -- ideally data would also be flushed with the full hour
 *    -- save data on HTTP reset (have shorter no-data periods)
 *    -- do not allow button calibration when the variance is too large (just do not instantiate a button action)
 */

// schedule setting and display
void scheduleDeviceActionSetting() {
    device.actionIndexMax = DEVICE_ACTION_DEPOWER;  // allow actions display and depower by index
    uint32_t secondstime = SensorTime::getSecondstime();
    uint32_t secondswait = 60 - secondstime % 60;  // CHECK_MEASURE_INTERVAL
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
 * handle a detected button action or a wifi event
 */
void handleActionComplete(std::function<bool(config_t& config)> actionFunction) {
    if (actionFunction != nullptr) {
        bool actionResult = actionFunction(config);  // execute the action (which, by convention, only alters the config to not interfere with program flow)
        if (actionResult) {
            ModuleSignal::beep();           // indicate complete
            scheduleDeviceActionSetting();  // schedule DEVICE_ACTION_SETTING and DEVICE_ACTION_DISPLAY
        }
    }
    actionNum++;  // interrupt delay loop
}

void scheduleDeviceActionDepower() {
    device.actionIndexCur = DEVICE_ACTION_DEPOWER;
    device.deviceActions[DEVICE_ACTION_DEPOWER].secondsNext = SensorTime::getSecondstime();
    actionNum++;  // interrupt delay loop
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

    esp_log_level_set("*", ESP_LOG_NONE);

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

        SensorTime::configure(config);    // enables the 1 minute timer
        ModuleWifi::configure(config);    // recreate the wifi data file, this will also call the initial ModuleSdCard begin
        ModuleMqtt::configure(config);    // recreate the mqtt data file
        ModuleDisp::configure(config);    // load display config and apply to config, must be configured prior to SensorScd041 to have correct temperature offset
        SensorScd041::configure(config);  // starts periodic measurement

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
    ButtonAction::begin(handleActionComplete);
    ButtonAction::adapt(config);

    // only sets the callback, but does not powerup anything
    ModuleDisp::begin();
    ModuleWifi::begin(handleActionComplete);

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

    esp_sleep_enable_timer_wakeup(sleepMicros);
    gpio_hold_en((gpio_num_t)I2C_POWER);  // power needs to be help or sensors will not measure

    Wire.end();

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
