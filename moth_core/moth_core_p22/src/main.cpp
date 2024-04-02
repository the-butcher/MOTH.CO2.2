#include <Arduino.h>
#include <Wire.h>
#include <driver/rtc_io.h>
#include <esp_wifi.h>

#include "buttons/ButtonAction.h"
#include "modules/ModuleDisplay.h"
#include "modules/ModuleSdcard.h"
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

// device state
RTC_DATA_ATTR device_t device;  // size: ~108

// configuration and display state
RTC_DATA_ATTR config_t config;  // size: ~128

// last hour of measurements and associated indices
RTC_DATA_ATTR values_t values;  // size: ~1212

// does not need to be RTC_DATA_ATTR, reset to 0 does not matter
uint16_t actionNum = 0;

const gpio_num_t PIN_PKK2_A = GPIO_NUM_16;

// uint16_t sc = sizeof(device);

/**
 * OK wifi
 *    OK clock sync, proper interval
 *    -- mqtt (+autoconnect for mqtt)
 *    OK async server
 *       -- full set of functions, upload, update
 * -- restore full configuration
 * -- calibration of SCD41
 * -- factory reset of SCD41
 * OK dat to csv for file download
 * -- reimplement significant change for shorter display intervals
 * -- reimplement OTA
 * -- actual beep (situations other than button press too)
 */

void scheduleDeviceActionSetting() {
    // schedule a display action
    device.actionIndexMax = DEVICE_ACTION_DEPOWER + 1;  // allow actions display and depower by index
    uint32_t secondstime = SensorTime::getSecondstime();
    uint32_t secondswait = 60 - secondstime % 60;
    if (device.actionIndexCur == DEVICE_ACTION_MEASURE && secondswait > WAITTIME_DISPLAY_AND_DEPOWER) {
        device.actionIndexCur = DEVICE_ACTION_SETTING;
        device.deviceActions[DEVICE_ACTION_SETTING].secondsNext = SensorTime::getSecondstime();  // assign current time as due time
    } else {
        // not index 0 -> having reassigned actionIndexMax to 4 will take care of rendering on the next regular cycle
        // index 0, but not enough time  -> having reassigned actionIndexMax to 4 will take care of renderingg on the next regular cycle
    }
}

void scheduleDeviceActionMeasure() {
    // TODO :: only if nothing else (calibration, ... ) is pending
    device.actionIndexCur = DEVICE_ACTION_MEASURE;
    device.deviceActions[DEVICE_ACTION_MEASURE].secondsNext = SensorTime::getSecondstime();
}

/**
 * handle a detected button action (pin and type)
 */
void handleButtonActionComplete(std::function<void(config_t* config)> actionFunction) {
    if (actionFunction != nullptr) {
#ifdef USE___SERIAL
        Serial.println("handling button action complete");
#endif
        ModuleSignal::beep();
        actionFunction(&config);        // execute the action (which, by convention, only alters the config to not interfere with program flow)
        scheduleDeviceActionSetting();  // schedule DEVICE_ACTION_SETTING and DEVICE_ACTION_DISPLAY
        actionNum++;
    }
}

void scheduleDeviceActionDepower() {
    device.actionIndexCur = DEVICE_ACTION_DEPOWER;
    device.deviceActions[DEVICE_ACTION_DEPOWER].secondsNext = SensorTime::getSecondstime();  // need to reset, or it will start waiting for DEPOWER again due to secondsNext
    actionNum++;                                                                             // interrupt delay loop
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

        SensorTime::setup();

        ModuleWifi::begin();  // only to check if the networks data file is present, this will also call the initial ModuleSdCard begin

        device = Device::load();
        config = Config::load();
        values = Values::load();
        if (SensorScd041::configure(&config)) {
            ModuleSignal::setPixelColor(COLOR______RED);  // indicate that a configuration just took place (involving a write to the scd41's eeprom)
        }

        // have a battery reading for the entry screen
        SensorEnergy::powerup();
        SensorEnergy::measure();
        SensorEnergy::depower();

        setupMode = SETUP_MAIN;
    }

    // enable global access to values
    Values::begin(&values);

    // show the correct button hints and have callback in place
    ButtonAction::begin(handleButtonActionComplete);
    ButtonAction::configure(&config);

    // only sets the callback, but does not powerup anything
    ModuleDisplay::begin();

    // did it wake up from a button press?
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    if (wakeupReason == ESP_SLEEP_WAKEUP_EXT0) {
        scheduleDeviceActionDepower();
    } else if (wakeupReason == ESP_SLEEP_WAKEUP_EXT1) {
        uint64_t wakeupStatus = esp_sleep_get_ext1_wakeup_status();
        gpio_num_t wakeupPin = (gpio_num_t)(log(wakeupStatus) / log(2));
        if (wakeupPin == PIN_RTC_SQW) {  // wakeup from pin other than the rtc pulse, must be a button press
            scheduleDeviceActionMeasure();
        } else {
            ButtonAction::createButtonAction(wakeupPin);
        }
    }
}

/**
 * check if the device needs to stay awake
 * - while a button is pressed
 * - while actionPin is set (action is still active)
 * - while the WiFi is on
 * - for debugging purposes
 */
bool isDelayRequired() {
#ifdef USE____DELAY
    return true;
#endif
    return ButtonAction::getActionPin() > 0 || ButtonAction::getPressedPin() > 0 || ModuleWifi::isPowered();
}

void secondsSleep(uint32_t seconds, wakeup_action_e wakeupType) {
    // convert to microseconds
    uint64_t sleepMicros = seconds * MICROSECONDS_PER______SECOND;

    esp_deep_sleep_disable_rom_logging();  // seems to have no effect
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

    ModuleSignal::prepareSleep();            // this may hold the neopixel power, depending on color debug setting
    ButtonAction::prepareSleep(wakeupType);  // establishes gpio holds, ... , returns ext1 bitmask
    SensorTime::prepareSleep(wakeupType);    // establishes gpio holds, ... , returns ext1 bitmask
    ModuleDisplay::prepareSleep(wakeupType);

    if (wakeupType == WAKEUP_ACTION_BUTN) {
        uint64_t ext1Bitmask = 1ULL << 12 | 1ULL << 11 | 1ULL << 8 | 1ULL << 6;  // button pins and SQW pin, ordered, TODO, sort pins and keep calculated bitmask
        esp_sleep_enable_ext1_wakeup(ext1Bitmask, ESP_EXT1_WAKEUP_ANY_LOW);
    }

    // Serial.flush();
    // Serial.end();
    if (device.actionIndexCur == DEVICE_ACTION_MEASURE) {  // next action is DEVICE_ACTION_MEASURE, no timer wakeup, RTC pulse will wake the device up to the full minute
        pinMode(I2C_POWER, OUTPUT);
        digitalWrite(I2C_POWER, LOW);
        gpio_hold_dis((gpio_num_t)I2C_POWER);
    } else {  // any action other than measure pending -> I2C on and defined sleep time
        esp_sleep_enable_timer_wakeup(sleepMicros);
        gpio_hold_en((gpio_num_t)I2C_POWER);  // power needs to be help or sensors will not measure
    }

    Wire.end();
    // https://github.com/espressif/arduino-esp32/issues/3363
    // pinMode(SDA, INPUT);  // needed because Wire.end() enables pullups, power Saving
    // pinMode(SCL, INPUT);

    digitalWrite(PIN_PKK2_A, LOW);

    // adc_power_release();
    // esp_wifi_stop();
    ModuleSignal::setPixelColor(COLOR_____BLUE);
    esp_deep_sleep_start();  // go to sleep
}

/**
 * wait with short delays until any of the conditions below become true
 * - the specified seconds have elapsed
 * - isDelayRequired() becomes false
 * - actionNumEntry has changed (some button action may have completed)
 */
void secondsDelay(uint32_t seconds, wakeup_action_e wakeupType) {
    uint32_t millisEntry = millis();
    uint32_t millisBreak = millisEntry + seconds * MILLISECONDS_PER______SECOND;
    uint16_t actionNumEntry = actionNum;

    ButtonAction::attachWakeup(wakeupType);   // button interrupts
    ModuleDisplay::attachWakeup(wakeupType);  // wait for busy pin
    SensorTime::attachWakeup(wakeupType);     // SQW interrupt

    ModuleSignal::setPixelColor(COLOR_____CYAN);
    while (isDelayRequired() && millis() < millisBreak && actionNumEntry == actionNum) {
        delay(50);
        if (SensorTime::isInterrupted()) {
            scheduleDeviceActionMeasure();
            break;
        }
        if (ModuleDisplay::isInterrupted()) {
            scheduleDeviceActionDepower();
            break;
        }
        if (SensorTime::getSecondsUntil(ModuleWifi::getSecondstimeExpiry()) == WAITTIME________________NONE && config.wifi.wifiValPower == WIFI____VAL_P_CUR_Y) {
// only checking for the flag, not actual WiFi status, the flag will then cause appropriate action in the settings action
#ifdef USE___SERIAL
            Serial.println("break for wifi off");
#endif
            config.wifi.wifiValPower = WIFI____VAL_P_PND_N;  // set flag to pending off
            scheduleDeviceActionSetting();
            break;
        }
        if (ModuleServer::requestedCalibrationReference > 400) {
#ifdef USE___SERIAL
            Serial.println("break for calibration");
#endif
            // TODO :: beep, implemented in a proper place
            scheduleDeviceActionSetting();  // TODO :: does not work for unknown reasons, wifi however is turned of
            break;
        }
    }

    ButtonAction::detachWakeup(wakeupType);
    ModuleDisplay::detachWakeup(wakeupType);
    SensorTime::detachWakeup(wakeupType);
}

void loop() {
    device_action_t action = device.deviceActions[device.actionIndexCur];
    if (SensorTime::getSecondsUntil(action.secondsNext) == WAITTIME________________NONE) {  // action is due
        ModuleSignal::setPixelColor(action.color);
        Device::getFunctionByAction(action.type)(&config);  // find and excute the function associated with this action
        device.actionIndexCur++;
        if (device.actionIndexCur < device.actionIndexMax) {  // more executable actions?
            device.deviceActions[device.actionIndexCur].secondsNext = SensorTime::getSecondstime() + action.secondsWait;
        } else {  // no more executable actions, rollover to zero
            device.actionIndexCur = 0;
            device.actionIndexMax = (values.nextMeasureIndex == values.nextDisplayIndex ? DEVICE_ACTION_DEPOWER : DEVICE_ACTION_READVAL) + 1;
            device.deviceActions[DEVICE_ACTION_MEASURE].secondsNext = SensorTime::getSecondstime() + SECONDS_PER_____________HOUR;
        }
        delay(10);  // TODO :: experimental, trying to find the cause for sporadic 10s@1mA after readval
    }
    uint32_t secondsWait = SensorTime::getSecondsUntil(device.deviceActions[device.actionIndexCur].secondsNext);
    if (secondsWait > WAITTIME________________NONE) {
        if (isDelayRequired()) {
            secondsDelay(secondsWait, action.wakeupType);
        } else {
            secondsSleep(secondsWait, action.wakeupType);
        }
    }
}
