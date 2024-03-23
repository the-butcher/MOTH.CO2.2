#include <Arduino.h>
#include <Wire.h>
#include <driver/rtc_io.h>
#include <esp_wifi.h>

#include "buttons/ButtonAction.h"
#include "modules/ModuleScreen.h"
#include "modules/ModuleSdcard.h"
#include "modules/ModuleSignal.h"
#include "modules/ModuleTicker.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"
#include "types/Action.h"
#include "types/Config.h"
#include "types/Define.h"
#include "types/Device.h"
#include "types/Values.h"

RTC_DATA_ATTR setup_mode_t setupMode = SETUP_BOOT;

// device state
RTC_DATA_ATTR device_t device;

// configuration and display state
RTC_DATA_ATTR config_t config;

// last hour of measurements
RTC_DATA_ATTR values_t values;

// does not need to be RTC_DATA_ATTR
uint16_t actionNum = 0;

/**
 * -- wifi
 *    -- clock sync
 *    -- mqtt (+autoconnect)
 * -- configuration
 * -- calibration of SCD41
 * -- factory reset of SCD41
 * -- dat to csv
 * -- actual beep
 *
 * -- would be nice if device-actions could be moved to a separate type, like button-actions
 */

uint32_t getMeasureNextSeconds() {
    return device.secondsSetupBase + values.nextMeasureIndex * 60;  // add one to index to be one measurement ahead
}

uint32_t getSecondsWait(uint32_t secondsNext) {
    uint32_t secondsTime = ModuleTicker::getSecondstime();
    return secondsNext > secondsTime ? secondsNext - secondsTime : WAITTIME________________NONE;
}

/**
 * handle a detected button action (pin and type)
 */
void handleButtonActionComplete(std::function<bool(config_t* config)> actionFunction) {
    if (actionFunction != nullptr) {
        bool redisplay = actionFunction(&config);
        ButtonAction::configure(&config);
        if (redisplay) {
            // schedule a display action
            device.actionIndexMax = 4;  // allow actions display and depower by index
            // waiting for DEVICE_ACTION_MEASURE and enough time to render
            uint32_t secondsNext = device.deviceActions[DEVICE_ACTION_MEASURE].secondsNext;
            uint32_t secondsWait = getSecondsWait(secondsNext);
            if (device.actionIndexCur == DEVICE_ACTION_MEASURE && secondsWait >= WAITTIME_DISPLAY_AND_DEPOWER) {
                device.actionIndexCur = DEVICE_ACTION_DISPLAY;
                device.deviceActions[DEVICE_ACTION_DISPLAY].secondsNext = ModuleTicker::getSecondstime();  // assign current time as due time
            } else {
                // not index 0 -> having reassigned actionIndexMax to 4 will take care of rendering on the next regular cycle
                // index 0, but not enough time  -> having reassigned actionIndexMax to 4 will take care of renderingg on the next regular cycle
            }
        }
        actionNum++;
    }
}

void handleBusyHigh() {
    device.deviceActions[DEVICE_ACTION_DEPOWER].secondsNext = ModuleTicker::getSecondstime();  // need to reset, or it will start waiting for DEPOWER again due to secondsNext
    actionNum++;                                                                               // interrupt delay loop
}

void setup() {
    // a debug pin usable in the PKK2 tool
    pinMode(GPIO_NUM_16, OUTPUT);
    digitalWrite(GPIO_NUM_16, LOW);
    rtc_gpio_deinit(GPIO_NUM_16);

    // turn on I2C power
    pinMode(I2C_POWER, OUTPUT);
    digitalWrite(I2C_POWER, HIGH);
    rtc_gpio_deinit((gpio_num_t)I2C_POWER);

    Wire.begin();
    ModuleTicker::begin();
    ModuleSignal::begin();

#ifdef USE___SERIAL
    Serial.begin(115200);
    delay(2000);
#endif
    ModuleSignal::setPixelColor(COLOR___YELLOW);

    SensorScd041::begin();
    SensorBme280::begin();
    SensorEnergy::begin();

    if (setupMode == SETUP_BOOT) {
        delay(1000);  // ModuleTicker appears to take some time to initialize, especially on boot

        ModuleSdcard::begin();

        device = Device::load();
        config = Config::load();
        if (SensorScd041::configure(&config)) {
            ModuleSignal::setPixelColor(COLOR______RED);  // indicate that a configuration just took place (with a write to the scd41's eeprom)
        }

        values.nextMeasureIndex = 0;
        values.nextDisplayIndex = 0;

        // have a battery reading for the entry screen
        SensorEnergy::powerUp();
        SensorEnergy::measure();
        SensorEnergy::powerDown();

        setupMode = SETUP_MAIN;
    }

    ButtonAction::begin(handleButtonActionComplete);
    ButtonAction::configure(&config);

    ModuleScreen::begin(handleBusyHigh);

    // did it wake up from a button press?
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    if (wakeupReason == ESP_SLEEP_WAKEUP_EXT0) {
        device.deviceActions[DEVICE_ACTION_DEPOWER].secondsNext = ModuleTicker::getSecondstime();  // has woken up from busy, no need to wait any longer, depower now
    } else if (wakeupReason == ESP_SLEEP_WAKEUP_EXT1) {
        uint64_t wakeupStatus = esp_sleep_get_ext1_wakeup_status();
        ButtonAction::createButtonAction((gpio_num_t)(log(wakeupStatus) / log(2)));
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
    return true || ButtonAction::getActionPin() > 0 || ButtonAction::getPressedPin() > 0;
}

void secondsSleep(uint32_t seconds, wakeup_action_e wakeupType) {
    // convert to microseconds
    uint64_t sleepMicros = seconds * MICROSECONDS_PER______SECOND;

    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

    ModuleSignal::prepareSleep();            // this may hold the neopixel power, depending on color debug setting
    ButtonAction::prepareSleep(wakeupType);  // if the pending action allows ext1 wakeup, ext1 wakeup will be established
    ModuleScreen::prepareSleep(wakeupType);  // if the pending action defines busy wakeup, it will be established
    esp_sleep_enable_timer_wakeup(sleepMicros);

    // Serial.flush();
    // Serial.end();
    if (device.actionIndexCur != DEVICE_ACTION_MEASURE) {  // any action other than measure pending -> I2C on
        gpio_hold_en((gpio_num_t)I2C_POWER);               // power needs to be help or sensors will not measure
    } else {
        pinMode(I2C_POWER, OUTPUT);
        digitalWrite(I2C_POWER, LOW);
        gpio_hold_dis((gpio_num_t)I2C_POWER);
    }

    Wire.end();  // https://github.com/espressif/arduino-esp32/issues/3363
    // pinMode(SDA, INPUT);  // needed because Wire.end() enables pullups, power Saving
    // pinMode(SCL, INPUT);

    digitalWrite(GPIO_NUM_16, HIGH);  // put signal pin high for PPK2
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
    digitalWrite(GPIO_NUM_16, HIGH);
    uint32_t millisEntry = millis();
    uint32_t millisBreak = millisEntry + seconds * MILLISECONDS_PER______SECOND;
    uint16_t actionNumEntry = actionNum;
    ButtonAction::attachWakeup(wakeupType);
    ModuleScreen::attachWakeup(wakeupType);
    ModuleSignal::setPixelColor(COLOR_____CYAN);
    while (isDelayRequired() && millis() < millisBreak && actionNumEntry == actionNum) {
        delay(50);
    }
    ButtonAction::detachWakeup(wakeupType);
    ModuleScreen::detachWakeup(wakeupType);
    digitalWrite(GPIO_NUM_16, LOW);
}

void loop() {
    device_action_t action = device.deviceActions[device.actionIndexCur];
    if (getSecondsWait(action.secondsNext) == WAITTIME________________NONE) {  // action is due
        ModuleSignal::setPixelColor(action.color);
        Device::getFunctionByAction(action.type)(&values, &config);  // find and excute the function associated with this action
        delay(1);                                                    // TODO :: experimental, trying to find the cause for 10s@1mA after readval
        device.actionIndexCur++;
        if (device.actionIndexCur < device.actionIndexMax) {  // more executable actions
            device.deviceActions[device.actionIndexCur].secondsNext = ModuleTicker::getSecondstime() + action.secondsWait;
        } else {  // no more executable actions, rollover to zero
            device.actionIndexCur = 0;
            device.actionIndexMax = values.nextMeasureIndex == values.nextDisplayIndex ? 4 : 2;
            device.deviceActions[DEVICE_ACTION_MEASURE].secondsNext = getMeasureNextSeconds();
        }
    }
    uint32_t secondsWait = getSecondsWait(device.deviceActions[device.actionIndexCur].secondsNext);
    if (secondsWait > WAITTIME________________NONE) {
        if (isDelayRequired()) {
            secondsDelay(secondsWait, action.wakeupType);
        } else {
            secondsSleep(secondsWait, action.wakeupType);
        }
    }
}
