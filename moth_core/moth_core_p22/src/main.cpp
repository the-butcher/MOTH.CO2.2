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
 * OK wifi
 *    OK clock sync, proper interval
 *    -- mqtt (+autoconnect for mqtt)
 *    OK async server
 *       OK full set of functions, upload, update
 * -- restore full configuration
 * OK calibration of SCD41
 *    -- must run 3 minutes in idle before calibrating or it will fail (not turning off i2c power and not powering off scd41 basicall means idle)
 * OK factory reset of SCD41, TODO :: test
 * OK reimplement OTA, TODO :: test
 * -- adapt server.html for updated api names, TODO :: upload updated build
 * -- co2 cal and rst could be handled through requested configuration
 * -- configurable mode for scd41
 */

// schedule setting and display
void scheduleDeviceActionSetting() {
    // schedule a display action
    device.actionIndexMax = DEVICE_ACTION_DEPOWER;  // allow actions display and depower by index
    uint32_t secondstime = SensorTime::getSecondstime();
    uint32_t secondswait = 60 - secondstime % 60;
    if (device.actionIndexCur == DEVICE_ACTION_MEASURE && secondswait > WAITTIME_DISPLAY_AND_DEPOWER) {
        device.actionIndexCur = DEVICE_ACTION_SETTING;
        device.deviceActions[DEVICE_ACTION_SETTING].secondsNext = SensorTime::getSecondstime();  // assign current time as due time
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

        SensorTime::configure();

        ModuleWifi::begin();  // only to check if the networks data file is present, this will also call the initial ModuleSdCard begin

        device = Device::load();
        config = Config::load();
        values = Values::load();
        if (SensorScd041::configure(config)) {
            ModuleSignal::setPixelColor(COLOR______RED);  // indicate that a configuration just took place (involving a write to the scd41's eeprom)
        }

        // have a battery reading for the entry screen
        SensorEnergy::powerup();
        SensorEnergy::measure();
        SensorEnergy::depower();

        setupMode = SETUP_MAIN;
        secondstimeBoot = SensorTime::getSecondstime();
    }

    // enable global access to values
    Values::begin(&values);
    Device::begin(secondstimeBoot);

    // show the correct button hints and have callback in place
    ButtonAction::begin(handleButtonActionComplete);
    ButtonAction::adapt(config);

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

    ModuleSignal::prepareSleep();             // this may hold the neopixel power, depending on color debug setting
    ButtonAction::prepareSleep(wakeupType);   // establishes gpio holds, ...
    SensorTime::prepareSleep(wakeupType);     // establishes gpio holds, ...
    ModuleDisplay::prepareSleep(wakeupType);  // adds ext0Wakeup (wait for busy pin high)

    if (wakeupType == WAKEUP_ACTION_BUTN) {
        esp_sleep_enable_ext1_wakeup(device.ext1Bitmask, ESP_EXT1_WAKEUP_ANY_LOW);
    }

    // 1) depending on action establish timer wakeup or not
    // 2) depending on action AND co2 sensor mode (TODO :: configurable and/or by user interaction or wifi call) depower i2c or not
    // 2a) co2 sensor powerup and depower depending on co2 sensor mode

    if (device.actionIndexCur == DEVICE_ACTION_MEASURE) {  // next action is DEVICE_ACTION_MEASURE, no timer wakeup, RTC pulse will wake the device up to the full minute
        // no timer wakeup for measure, the RTC will send a pin low signal every full minute
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

        // the 1 minute measure pin
        if (SensorTime::isInterrupted()) {
            scheduleDeviceActionMeasure();
            break;
        }

        // the display's busy pin
        if (ModuleDisplay::isInterrupted()) {
            scheduleDeviceActionDepower();
            break;
        }

        // wifi expiry
        uint32_t secondsdest = ModuleWifi::getSecondstimeExpiry();
        uint32_t secondstime = SensorTime::getSecondstime();
        uint32_t secondswait = secondsdest > secondstime ? secondsdest - secondstime : WAITTIME________________NONE;
        if (secondswait == WAITTIME________________NONE && config.wifi.wifiValPower == WIFI____VAL_P_CUR_Y) {
            // #ifdef USE___SERIAL
            //             Serial.printf("secondsdest: %d, secondstime: %d\n", secondsdest, secondstime);
            // #endif
            ModuleSignal::beep();
            config.wifi.wifiValPower = WIFI____VAL_P_PND_N;  // set flag to pending off
            scheduleDeviceActionSetting();
            break;
        }

        if (ModuleServer::requestedReconfiguration != nullptr) {  // display properties
            ModuleSignal::beep();
            ModuleServer::requestedReconfiguration(config);
            ButtonAction::adapt(config);
            scheduleDeviceActionSetting();
            ModuleServer::requestedReconfiguration = nullptr;
            break;
        } else if (ModuleServer::requestedCo2Ref > 400) {
#ifdef USE___SERIAL
            Serial.println("break for co2cal");
#endif
            // TODO :: beep, implemented in a proper place
            scheduleDeviceActionSetting();  // TODO :: does not work for unknown reasons, wifi however is turned of
            break;
        } else if (ModuleServer::requestedCo2Rst) {
#ifdef USE___SERIAL
            Serial.println("break for co2rst");
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
        device.actionIndexCur = Device::getFunctionByAction(action.type)(config, device.actionIndexMax);  // execute the action and see whats coming next
        if (device.actionIndexCur == DEVICE_ACTION_MEASURE) {
            device.actionIndexMax = values.nextMeasureIndex == values.nextDisplayIndex ? DEVICE_ACTION_DEPOWER : DEVICE_ACTION_READVAL;
            device.deviceActions[DEVICE_ACTION_MEASURE].secondsNext = SensorTime::getSecondstime() + SECONDS_PER_____________HOUR;
        } else {
            device.deviceActions[device.actionIndexCur].secondsNext = SensorTime::getSecondstime() + action.secondsWait;
        }
        delay(10);
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
