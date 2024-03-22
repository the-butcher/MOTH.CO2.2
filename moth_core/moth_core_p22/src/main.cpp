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
#include "types/Values.h"

// device state
RTC_DATA_ATTR setup_mode_t setupMode = SETUP_BOOT;
RTC_DATA_ATTR uint32_t secondsSetupBase;  // secondstime at boot time plus some buffer
RTC_DATA_ATTR device_action_t deviceActions[4];
RTC_DATA_ATTR uint8_t actionIndexCur;
RTC_DATA_ATTR uint8_t actionIndexMax;

// device config and display state
RTC_DATA_ATTR config_t config;

// recent measurements
RTC_DATA_ATTR values_all_t measurements[MEASUREMENT_BUFFER_SIZE];
RTC_DATA_ATTR uint32_t nextMeasureIndex;
RTC_DATA_ATTR uint32_t nextDisplayIndex;

// does not need to be RTC_DATA_ATTR
uint16_t actionNum = 0;

/**
 * -- configuration
 * -- clock sync
 * -- wifi
 * -- mqtt (+autoconnect)
 * -- calibration of SCD41
 * -- factory reset of SCD41
 * -- dat to csv
 * -- actual beep
 *
 * -- would be nice if device-actions could be moved to a separate type, like button-actions
 */

uint32_t getMeasureNextSeconds() {
    return secondsSetupBase + nextMeasureIndex * 60;  // add one to index to be one measurement ahead
}

void populateConfig() {
    config = {
        {
            800,   // co2 warnHi
            1000,  // co2 riskHi
            425    // co2 reference
        },
        {
            14,  // deg riskLo
            19,  // deg warnLo
            25,  // deg warnHi
            30   // deg riskHi
        },
        {
            25,  // hum riskLo
            30,  // hum warnLo
            60,  // hum warnHi
            65   // hum riskHi
        },
        3,                    // display update minutes
        DISPLAY_VAL_M_CHART,  // chart | table
        DISPLAY_VAL_T___CO2,  // value shown when rendering a measurement (table)
        DISPLAY_VAL_C___CO2,  // value shown when rendering the chart
        DISPLAY_HRS_C____03,  // hours to be shown in chart display
        DISPLAY_THM___LIGHT,  // light theme
        false,                // c2f celsius to fahrenheit
        false,                // beep
        false,                // wifi
        1.5,                  // temperature offset
        0.0,                  // calculated sealevel pressure, 0.0 = needs recalculation
        153                   // the altitude that the seonsor was configured to (or set by the user)
    };
}

void populateActions() {
    deviceActions[DEVICE_ACTION_MEASURE] = {
        DEVICE_ACTION_MEASURE,  // trigger measurements
        COLOR__MAGENTA,         // magenta while measuring
        WAKEUP_BUTTONS,         // allow wakeup while measurement is active
        5                       // 5 seconds delay to complete measurement
    };
    deviceActions[DEVICE_ACTION_READVAL] = {
        DEVICE_ACTION_READVAL,  // read any values that the sensors may have produced
        COLOR__MAGENTA,         // magenta while reading values
        WAKEUP_BUTTONS,         // allow wakeup while measurement is active (but wont ever happen due to 0 wait)
        0                       // no delay required after readval
    };
    deviceActions[DEVICE_ACTION_DISPLAY] = {
        DEVICE_ACTION_DISPLAY,          // display refresh
        COLOR______RED,                 // red while refreshing display
        WAKEUP_BUSYPIN,                 // do NOT allow wakeup while display is redrawing
        3,                              // 3 seconds delay which is a rather long and conservative estimation (buut the busy wakeup should take care of things)
        ModuleTicker::getSecondstime()  // initially set, so the entry screen renders right away
    };
    deviceActions[DEVICE_ACTION_DEPOWER] = {
        DEVICE_ACTION_DEPOWER,  // depower display
        COLOR______RED,         // red while depowering
        WAKEUP_BUTTONS,         // allow wakeup after depower, while waiting for a new measurement
        0                       // no delay required after this action
    };
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
            actionIndexMax = 4;  // allow actions display and depower by index
            // waiting for DEVICE_ACTION_MEASURE and enough time to render
            uint32_t secondsNext = deviceActions[DEVICE_ACTION_MEASURE].secondsNext;
            uint32_t secondsWait = getSecondsWait(secondsNext);
            if (actionIndexCur == DEVICE_ACTION_MEASURE && secondsWait >= WAITTIME_DISPLAY_AND_DEPOWER) {
                actionIndexCur = DEVICE_ACTION_DISPLAY;
                deviceActions[DEVICE_ACTION_DISPLAY].secondsNext = ModuleTicker::getSecondstime();  // assign current time as due time
            } else {
                // not index 0 -> having reassigned actionIndexMax to 4 will take care of rendering on the next regular cycle
                // index 0, but not enough time  -> having reassigned actionIndexMax to 4 will take care of renderingg on the next regular cycle
            }
        }
        actionNum++;
    }
}

void handleBusyHigh() {
    deviceActions[DEVICE_ACTION_DEPOWER].secondsNext = ModuleTicker::getSecondstime();  // need to reset, or it will start waiting for DEPOWER again due to secondsNext
    actionNum++;                                                                        // interrupt delay loop
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
        // secondsCycleBase = ModuleTicker.getDate().secondstime();
        // secondsCycleBase = secondsCycleBase + 60 + SECONDS_BOOT_BUFFER - (secondsCycleBase + SECONDS_BOOT_BUFFER) % 60;  // first full minute after boot in secondstime
        secondsSetupBase = ModuleTicker::getSecondstime() + 10;

        ModuleSdcard::begin();

        populateConfig();
        if (SensorScd041::configure(&config)) {
            ModuleSignal::setPixelColor(COLOR______RED);  // indicate that a configuration just took place (with a write to the scd41's eeprom)
        }

        populateActions();
        // deviceActions[DEVICE_ACTION_MEASURE].secondsNext = getMeasureNextSeconds();
        // actionIndexCur = DEVICE_ACTION_DISPLAY; // start with DEVICE_ACTION_DISPLAY for entry screen
        actionIndexCur = 0;
        actionIndexMax = 4;

        nextMeasureIndex = 0;
        nextDisplayIndex = 0;

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
        handleBusyHigh();
        deviceActions[DEVICE_ACTION_DEPOWER].secondsNext = ModuleTicker::getSecondstime();  // has woken up from busy, no need to wait any longer for depowering
    } else if (wakeupReason == ESP_SLEEP_WAKEUP_EXT1) {
        uint64_t wakeupStatus = esp_sleep_get_ext1_wakeup_status();
        ButtonAction::createButtonAction((gpio_num_t)(log(wakeupStatus) / log(2)));
    }
}

void handleActionMeasure() {
    // co2
    SensorScd041::powerUp();
    SensorScd041::measure();
    // pressure
    SensorBme280::measure();
    // battery
    if (nextMeasureIndex % 3 == 0) {  // only measure pressure and battery every three minutes
        SensorEnergy::powerUp();
        SensorEnergy::measure();
    }
    nextMeasureIndex++;
}

/**
 * reads values from sensors
 */
void handleActionReadval() {
    // read values from the sensors
    values_co2_t measurementCo2 = SensorScd041::readval();
    values_bme_t measurementBme = SensorBme280::readval();
    values_nrg_t measurementNrg = SensorEnergy::readval();
    // store values
    int currMeasureIndex = nextMeasureIndex - 1;
    measurements[currMeasureIndex % MEASUREMENT_BUFFER_SIZE] = {
        ModuleTicker::getSecondstime(),  // secondstime as of RTC
        measurementCo2,                  // sensorScd041 values
        measurementBme,                  // sensorBme280 values
        measurementNrg,                  // battery values
        true                             // publishable
    };
    // power down sensors
    SensorScd041::powerDown();
    SensorEnergy::powerDown();
    // upon rollover, write measurements to SD card
    if (nextMeasureIndex % MEASUREMENT_BUFFER_SIZE == 0) {  // when the next measurement index is dividable by MEASUREMENT_BUFFER_SIZE, measurements need to be written to sd
        ModuleSdcard::begin();
        ModuleSdcard::persistValues(measurements);
    }
    // when pressureZerolevel == 0.0 pressure at sealevel needs to be recalculated (should only happen once at startup)
    if (config.pressureZerolevel == 0.0) {
        config.pressureZerolevel = SensorBme280::getPressureZerolevel(config.altitudeBaselevel, measurementBme.pressure);
    }
}

void handleActionDisplay() {
    if (nextMeasureIndex > 0) {
        uint32_t currMeasureIndex = nextMeasureIndex - 1;
        values_all_t measurement = measurements[(currMeasureIndex + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE];
        if (config.displayValModus == DISPLAY_VAL_M_TABLE) {
            ModuleScreen::renderTable(&measurement, &config);
        } else {
            values_all_t history[HISTORY_____BUFFER_SIZE];
            ModuleSdcard::historyValues(measurements, currMeasureIndex, history, &config);  // will fill history with values from file or current measurements
            ModuleScreen::renderChart(history, &config);
        }
        nextDisplayIndex = currMeasureIndex + config.displayUpdateMinutes;
    } else {
        ModuleScreen::renderEntry(&config);
    }
}

void handleActionDepower() {
    ModuleScreen::hibernate();
    SensorEnergy::powerDown();  // redundant power down on battery monitor, seems to help with power reduction after redisplay
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

void secondsSleep(uint32_t seconds, wakeup_e wakeupType) {
    // convert to microseconds
    uint64_t sleepMicros = seconds * MICROSECONDS_PER______SECOND;

    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

    ModuleSignal::prepareSleep();            // this may hold the neopixel power, depending on color debug setting
    ButtonAction::prepareSleep(wakeupType);  // if the pending action allows ext1 wakeup
    ModuleScreen::prepareSleep(wakeupType);
    esp_sleep_enable_timer_wakeup(sleepMicros);

    // Serial.flush();
    // Serial.end();
    if (actionIndexCur != DEVICE_ACTION_MEASURE) {  // any action other than measure pending -> I2C on
        gpio_hold_en((gpio_num_t)I2C_POWER);        // power needs to be help or sensors will not measure
    } else {
        // disable I2C power, shutdown wire and disable pullup on SDA and ACL
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
void secondsDelay(uint32_t seconds, wakeup_e wakeupType) {
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
    device_action_t action = deviceActions[actionIndexCur];
    if (getSecondsWait(action.secondsNext) == WAITTIME________________NONE) {  // action is due
        ModuleSignal::setPixelColor(action.color);
        switch (action.type) {
            case DEVICE_ACTION_MEASURE:
                handleActionMeasure();
                break;
            case DEVICE_ACTION_READVAL:
                handleActionReadval();
                break;
            case DEVICE_ACTION_DISPLAY:
                handleActionDisplay();
                break;
            case DEVICE_ACTION_DEPOWER:
                handleActionDepower();
                break;
            default:
                break;
        }
        delay(1);  // TODO :: experimental, trying to find the cause for 10s@1mA after readval
        actionIndexCur++;
        if (actionIndexCur < actionIndexMax) {  // more executable actions
            deviceActions[actionIndexCur].secondsNext = ModuleTicker::getSecondstime() + action.secondsWait;
        } else {  // no more executable actions, rollover to zero
            actionIndexCur = 0;
            actionIndexMax = nextMeasureIndex == nextDisplayIndex ? 4 : 2;
            deviceActions[DEVICE_ACTION_MEASURE].secondsNext = getMeasureNextSeconds();
        }
    }
    uint32_t secondsWait = getSecondsWait(deviceActions[actionIndexCur].secondsNext);
    if (secondsWait > WAITTIME________________NONE) {
        if (isDelayRequired()) {
            secondsDelay(secondsWait, action.wakeupType);
        } else {
            secondsSleep(secondsWait, action.wakeupType);
        }
    }
}
