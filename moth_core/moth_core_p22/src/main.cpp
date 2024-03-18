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
#include "types/Values.h"

typedef enum {
    SETUP_BOOT,
    SETUP_MAIN
} setup_mode_t;

// device state
RTC_DATA_ATTR setup_mode_t setupMode = SETUP_BOOT;
RTC_DATA_ATTR uint32_t secondsSetupBase;  // secondstime at boot time plus some buffer
RTC_DATA_ATTR action_t actions[4];
RTC_DATA_ATTR uint32_t actionIndexCur;
RTC_DATA_ATTR uint32_t actionIndexMax;

// device config and display state
RTC_DATA_ATTR config_t config;

// recent measurements
const uint8_t MEASUREMENT_BUFFER_SIZE = 30;
RTC_DATA_ATTR values_all_t measurements[MEASUREMENT_BUFFER_SIZE];
RTC_DATA_ATTR uint32_t nextMeasureIndex;
RTC_DATA_ATTR uint32_t nextDisplayIndex;

BoxBeep boxBeep;
BoxTime boxTime;
BoxData boxData;
SensorScd041 sensorScd041;
SensorBme280 sensorBme280;
SensorEnergy sensorEnergy;
BoxDisplay boxDisplay;
ButtonHandlers buttonHandlers;

/**
 * thoughs on file handling (with the speciality of measurement history display in chart)
 * 60 measurements
 * -  1h ->  1min resolution
 * -  3h ->  3min resolution
 * ...
 * - 24h -> 24min resolution
 *
 * therefore it will be necessary to open stored csv files and find the appropiate measrements
 * it can be assumed that finding data will will always involve 1 or two files
 *
 * -- 60 slots of measurements searched for could filled
 * -- starting with the oldest, open file, if not already open
 * -- iterate through file until a good enough match is found (less than 30 seconds off)
 *
 * -- poc has been implemented in esp32_csvtest (not on github yet)
 *    -- let there be a "value-provider" that will find 60 measurements from file, or in the special case if 1h from the RTC memory values (likely for the sake of power usage)
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
        DISPLAY_VAL_M_TABLE,
        DISPLAY_VAL_T___CO2,  // value shown when rendering a measurement
        false,                // c2f celsius to fahrenheit
        false,                // beep
        1.5,                  // temperature offset
        0.0,                  // calculated sealevel pressure, 0.0 = needs recalculation
        153                   // the altitude that the seonsor was configured to (or set by the user)
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

    sensorScd041.begin();
    sensorBme280.begin();
    sensorEnergy.begin();

    if (setupMode == SETUP_BOOT) {
        delay(1000);  // boxTime appears to take some time to initialize, especially on boot
        // secondsCycleBase = boxTime.getDate().secondstime();
        // secondsCycleBase = secondsCycleBase + 60 + SECONDS_BOOT_BUFFER - (secondsCycleBase + SECONDS_BOOT_BUFFER) % 60;  // first full minute after boot in secondstime
        secondsSetupBase = boxTime.getDate().secondstime() + 10;
        setupMode = SETUP_MAIN;

        boxData.begin();

        populateConfig();
        if (sensorScd041.configure(&config)) {
            boxBeep.setPixelColor(COLOR______RED);  // indicate that a configuration just took place (with a write to the scd41's eeprom)
        }

        populateActions();
        actions[ACTION_MEASURE].secondsNext = getMeasureNextSeconds();
        actionIndexCur = 0;  // start with high index to trigger primary wait
        actionIndexMax = 4;

        nextMeasureIndex = 0;
        nextDisplayIndex = 0;
    }

    // did it wake up from a button press?
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    if (wakeupReason == ESP_SLEEP_WAKEUP_EXT1) {
        // get the pin that it woke up from
        uint64_t wakeupStatus = esp_sleep_get_ext1_wakeup_status();
        gpio_num_t wakeupPin = (gpio_num_t)(log(wakeupStatus) / log(2));

        if (digitalRead(wakeupPin) == HIGH) {  // has already been released

            // alter state as of action
            // TODO :: there also needs to be a way to execute actions like calibration
            // -- would have to pause the action cycle until complete, and resume after completion
            // -- if running long, there could be a pattern of inserting "void" measurements or incrementing the start seconds

            actionIndexMax = 4;  // allow actions display and depower by index
            // waiting for ACTION_MEASURE and enough time to render
            if (actionIndexCur == ACTION_MEASURE && getSecondsWait(actions[ACTION_MEASURE].secondsNext) >= BoxTime::WAITTIME_DISPLAY_AND_DEPOWER) {  // action is due
                actionIndexCur = ACTION_DISPLAY;                                                                                                     // ACTION_DISPLAY
                actions[ACTION_DISPLAY].secondsNext = boxTime.getDate().secondstime();                                                               // assign current time as due time
            } else {
                // not index 0 -> having reassigned actionIndexMax to 4 will take care of rendering on the next regular cycle
                // index 0, but not enough time  -> having reassigned actionIndexMax to 4 will take care of renderingg on the next regular cycle
            }

        } else {
            // still pressed, TODO :: take care of long press
            // could spawn a task that establishes an interrupt, then keeps checking for button state and assigns some fallrise_t state to the button handler
        }
    }
}

void handleActionMeasure() {
    // co2
    sensorScd041.powerUp();
    sensorScd041.measure();
    // pressure
    sensorBme280.measure();
    // battery
    if (nextMeasureIndex % 3 == 0) {  // only measure pressure and battery every three minutes
        sensorEnergy.powerUp();
        sensorEnergy.measure();
    }
    nextMeasureIndex++;
}

/**
 * reads values from sensors
 */
void handleActionReadval() {
    // read values from the sensors
    values_co2_t measurementCo2 = sensorScd041.readval();
    values_bme_t measurementBme = sensorBme280.readval();
    values_nrg_t measurementNrg = sensorEnergy.readval();
    // store values
    int currMeasureIndex = nextMeasureIndex - 1;
    measurements[currMeasureIndex % MEASUREMENT_BUFFER_SIZE] = {
        boxTime.getDate().secondstime(),  // secondstime as of RTC
        measurementCo2,                   // sensorScd041 values
        measurementBme,                   // sensorBme280 values
        measurementNrg,                   // battery values
        true                              // publishable
    };
    // power down sensors
    sensorScd041.powerDown();
    sensorEnergy.powerDown();
    // upon rollover, write measurements to SD card
    if (nextMeasureIndex % MEASUREMENT_BUFFER_SIZE == 0) {  // when the next measurement index is dividable by MEASUREMENT_BUFFER_SIZE, measurements need to be written to sd
        boxData.begin();
        boxData.persistValues(measurements, MEASUREMENT_BUFFER_SIZE);
    }
    // when pressureZerolevel == 0.0 it means that pressure at sealevel needs to be recalculated
    if (config.pressureZerolevel == 0.0) {
        config.pressureZerolevel = sensorBme280.getPressureZerolevel(config.altitudeBaselevel, measurementBme.pressure);
    }
}

void handleActionDisplay() {
    int currMeasureIndex = nextMeasureIndex - 1;
    values_all_t measurement = measurements[(currMeasureIndex + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE];
    boxDisplay.renderMeasurement(&measurement, &config);
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
