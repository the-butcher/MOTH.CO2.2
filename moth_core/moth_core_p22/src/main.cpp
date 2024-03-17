#include <Arduino.h>
#include <Wire.h>
#include <driver/rtc_io.h>
#include <esp_wifi.h>

#include "BoxBeep.h"
#include "BoxData.h"
#include "BoxDisplay.h"
#include "BoxTime.h"
#include "buttons/ButtonHandlers.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"
#include "types/Action.h"
#include "types/Config.h"
#include "types/Measurement.h"

typedef enum {
    SETUP_BOOT,
    SETUP_MAIN
} setup_mode_t;

RTC_DATA_ATTR setup_mode_t setupMode = SETUP_BOOT;
RTC_DATA_ATTR uint32_t secondsSetupBase;  // secondstime at boot time plus some buffer
RTC_DATA_ATTR action_t actions[4];
RTC_DATA_ATTR config_t config;
RTC_DATA_ATTR uint32_t actionIndexCur;
RTC_DATA_ATTR uint32_t actionIndexMax;

RTC_DATA_ATTR measurement_t measurements[60];
RTC_DATA_ATTR uint32_t nextMeasureIndex;
RTC_DATA_ATTR uint32_t nextDisplayIndex;

// std::function<void(void)> displayFunc = nullptr;

BoxBeep boxBeep;
BoxTime boxTime;
BoxData boxData;
SensorScd041 sensorScd041;
SensorBme280 sensorBme280;
SensorEnergy sensorEnergy;
BoxDisplay boxDisplay;
ButtonHandlers buttonHandlers;

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
        DISPLAY_VAL_T___CO2,  // value shown when rendering a measurement
        false,                // c2f celsius to fahrenheit
        false                 // beep
    };
}

void populateActions() {
    actions[ACTION_MEASURE] = {
        ACTION_MEASURE,  // trigger measurements
        COLOR__MAGENTA,  // magenta while measuring
        true,            // allow wakeup while measurement is active
        5                // 5 seconds delay to complete measurement
    };
    actions[ACTION_READVAL] = {
        ACTION_READVAL,  // read any values that the sensors may have produced
        COLOR__MAGENTA,  // magenta while reading values
        true,            // allow wakeup while measurement is active (but wont ever happen due to 0 wait)
        0                // no delay required after readval
    };
    actions[ACTION_DISPLAY] = {
        ACTION_DISPLAY,  // display refresh
        COLOR______RED,  // red while refreshing display
        false,           // do NOT allow wakeup while display is redrawing
        2                // 2 seconds delay to complete redraw (TODO :: verify that the display actually gets hibernated)
    };
    actions[ACTION_DEPOWER] = {
        ACTION_DEPOWER,  // depower display
        COLOR______RED,  // red while depowering
        true,            // allow wakeup after depower, while waiting for a new measurement
        0                // no delay required after this action
    };
}

uint32_t getSecondsWait(uint32_t secondsNext) {
    uint32_t secondsTime = boxTime.getDate().secondstime();
    return secondsNext > secondsTime ? secondsNext - secondsTime : BoxTime::WAITTIME________________NONE;
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
    boxTime.begin();
    boxBeep.begin();
    buttonHandlers.begin();
    boxBeep.setPixelColor(COLOR_____GRAY);
    // delay(50);  // find out why this is needed and if there could be a more efficient way to wait only for as long as neededd
    // while (boxTime.getDate().secondstime() < 500000000) {
    //     delay(5);
    // }

    // Serial.begin(115200);
    // delay(2000);

    if (setupMode == SETUP_BOOT) {
        delay(1000);  // boxTime appears to take some time to initialize, especially on boot
        // secondsCycleBase = boxTime.getDate().secondstime();
        // secondsCycleBase = secondsCycleBase + 60 + SECONDS_BOOT_BUFFER - (secondsCycleBase + SECONDS_BOOT_BUFFER) % 60;  // first full minute after boot in secondstime
        secondsSetupBase = boxTime.getDate().secondstime() + 10;
        setupMode = SETUP_MAIN;

        boxData.begin();

        // TODO :: load configuration like display interval, temperature offset, ...

        populateConfig();
        populateActions();
        actions[ACTION_MEASURE].secondsNext = getMeasureNextSeconds();
        actionIndexCur = 0;  // start with high index to trigger primary wait
        actionIndexMax = 4;

        nextMeasureIndex = 0;
        nextDisplayIndex = 0;
    }

    sensorScd041.begin();
    sensorBme280.begin();
    sensorEnergy.begin();

    // did it wake up from a button press?
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    if (wakeupReason == ESP_SLEEP_WAKEUP_EXT1) {
        // get the pin that it woke up from
        uint64_t wakeupStatus = esp_sleep_get_ext1_wakeup_status();
        gpio_num_t wakeupPin = (gpio_num_t)(log(wakeupStatus) / log(2));

        if (digitalRead(wakeupPin) == HIGH) {  // has already been released

            // alter state as of action

            actionIndexMax = 4;  // allow actions display and depower by index
            // waiting for ACTION_MEASURE and enough time to render
            if (actionIndexCur == ACTION_MEASURE && getSecondsWait(actions[ACTION_MEASURE].secondsNext) >= BoxTime::WAITTIME_DISPLAY_AND_DEPOWER) {  // action is due
                actionIndexCur = ACTION_DISPLAY;                                                                                                     // ACTION_DISPLAY
                actions[ACTION_DISPLAY].secondsNext = boxTime.getDate().secondstime();                                                               // assign current time as due time
            } else {
                // not index 0 -> having reassigned actionIndexMax to 4 will take care of rendering
                // index 0, but not enough time  -> having reassigned actionIndexMax to 4 will take care of rendering
            }

        } else {
            // still pressed, TODO :: take care of long press
        }
    }
}

void handleActionMeasure() {
    // co2
    sensorScd041.powerUp();
    sensorScd041.measure();
    if (nextMeasureIndex % 3 == 0) {  // only measure pressure and battery every three minutes
        // pressure
        sensorBme280.measure();
        // battery
        sensorEnergy.powerUp();
        sensorEnergy.measure();
    }
    nextMeasureIndex++;
}

void handleActionReadval() {
    // measure and store
    measurement_co2_t measurementCo2 = sensorScd041.readval();
    measurement_bme_t measurementBme = sensorBme280.readval();
    measurement_nrg_t measurementNrg = sensorEnergy.readval();
    int currMeasureIndex = nextMeasureIndex - 1;
    measurements[currMeasureIndex % 60] = {
        boxTime.getDate().secondstime(),  // secondstime as of RTC
        measurementCo2,                   // sensorScd041 values
        measurementBme,                   // sensorBme280 values
        measurementNrg,                   // battery values
        true                              // publishable
    };
    sensorScd041.powerDown();
    sensorEnergy.powerDown();
}

void handleActionDisplay() {
    int currMeasureIndex = nextMeasureIndex - 1;
    measurement_t measurement = measurements[(currMeasureIndex + 60) % 60];
    boxDisplay.renderMeasurement(measurement, config);

    // String value1 = currMeasureIndex >= 2 ? String(measurements[(currMeasureIndex + 58) % 60].valuesCo2.co2) : "NA";
    // String value2 = currMeasureIndex >= 1 ? String(measurements[(currMeasureIndex + 59) % 60].valuesCo2.co2) : "NA";
    // String value3 = currMeasureIndex >= 0 ? String(measurements[(currMeasureIndex + 60) % 60].valuesCo2.co2) : "NA";
    // boxDisplay.renderTest(value1, value2, value3);
    nextDisplayIndex = nextMeasureIndex + 2;
}

void handleActionDepower() {
    boxDisplay.hibernate();
    sensorEnergy.powerDown();  // redundant power down on battery monitor, seems to help with power reduction after redisplay
}

/**
 * check if the device needs to stay awake
 * - while a button is pressed, to wait for final button action
 * - while the WiFi is on
 * - for debugging purposes
 */
bool isDelayRequired() {
    return buttonHandlers.isAnyPressed();
}

void secondsSleep(uint32_t seconds, bool requireI2C, bool isExt1Wakeup) {
    // convert to microseconds
    uint64_t sleepMicros = seconds * BoxTime::MICROSECONDS_PER______SECOND;

    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

    // ESP_PD_DOMAIN_RTC_SLOW_MEM

    // this may hold the neopixel power, depending on color debug setting
    boxBeep.prepareSleep();  // puts neopixel power low
    buttonHandlers.prepareSleep(isExt1Wakeup);
    esp_sleep_enable_timer_wakeup(sleepMicros);

    // Serial.flush();
    // Serial.end();
    if (requireI2C) {
        gpio_hold_en((gpio_num_t)I2C_POWER);  // power needs to be help or sensors will not measure
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
    boxBeep.setPixelColor(COLOR_____BLUE);
    esp_deep_sleep_start();  // go to sleep
}

/**
 * wait with short delays until any of the conditions below become true
 * - the specified seconds have elapsed
 * - isDelayRequired() becomes false
 */
void secondsDelay(uint32_t seconds) {
    uint32_t millisA = millis();
    uint32_t millisE = millisA + seconds * BoxTime::MILLISECONDS_PER______SECOND;
    boxBeep.setPixelColor(COLOR_____CYAN);
    while (isDelayRequired() && millis() < millisE) {
        delay(250);
    }
}

void loop() {
    action_t action = actions[actionIndexCur];
    if (getSecondsWait(action.secondsNext) == BoxTime::WAITTIME________________NONE) {  // action is due
        boxBeep.setPixelColor(action.color);
        switch (action.type) {
            case ACTION_MEASURE:
                handleActionMeasure();
                break;
            case ACTION_READVAL:
                handleActionReadval();
                break;
            case ACTION_DISPLAY:
                handleActionDisplay();
                break;
            case ACTION_DEPOWER:
                handleActionDepower();
                break;
            default:
                break;
        }
        delay(1);  // TODO :: experimental, trying to find the cause for 10s@1mA after readval
        actionIndexCur++;
        if (actionIndexCur < actionIndexMax) {  // more executable actions
            actions[actionIndexCur].secondsNext = boxTime.getDate().secondstime() + action.secondsWait;
        } else {  // no more executable actions, rollover to zero
            actionIndexCur = 0;
            actionIndexMax = nextMeasureIndex == nextDisplayIndex ? 4 : 2;
            actions[ACTION_MEASURE].secondsNext = getMeasureNextSeconds();
        }
    }
    uint32_t secondsWait = getSecondsWait(actions[actionIndexCur].secondsNext);
    if (secondsWait > BoxTime::WAITTIME________________NONE) {
        // stay awake if i.e. a pressed button or WiFi requires it, but no longer than required
        if (isDelayRequired()) {
            secondsDelay(secondsWait);
        } else {
            secondsSleep(secondsWait, actionIndexCur > 0, actions[actionIndexCur].isExt1Wakeup);  // initiate sleep, I2C required when action other than zero
        }
    }
}
