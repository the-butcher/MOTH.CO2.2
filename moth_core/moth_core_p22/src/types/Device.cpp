#include "types/Device.h"

#include <Arduino.h>

#include "modules/ModuleScreen.h"
#include "modules/ModuleSdcard.h"
#include "modules/ModuleSignal.h"
#include "modules/ModuleTicker.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"

device_t Device::load() {
    uint32_t secondstime = ModuleTicker::getSecondstime();
    // secondsCycleBase = ModuleTicker.getDate().secondstime();
    // secondsCycleBase = secondsCycleBase + 60 + SECONDS_BOOT_BUFFER - (secondsCycleBase + SECONDS_BOOT_BUFFER) % 60;  // first full minute after boot in secondstime

    device_t device;
    device.secondsSetupBase = secondstime + 10;
    device.deviceActions[DEVICE_ACTION_MEASURE] = {
        DEVICE_ACTION_MEASURE,  // trigger measurements
        COLOR__MAGENTA,         // magenta while measuring
        WAKEUP_ACTION_BUTN,     // allow wakeup while measurement is active
        5                       // 5 seconds delay to complete measurement
    };
    device.deviceActions[DEVICE_ACTION_READVAL] = {
        DEVICE_ACTION_READVAL,  // read any values that the sensors may have produced
        COLOR__MAGENTA,         // magenta while reading values
        WAKEUP_ACTION_BUTN,     // allow wakeup while measurement is active (but wont ever happen due to 0 wait)
        0                       // no delay required after readval
    };
    device.deviceActions[DEVICE_ACTION_DISPLAY] = {
        DEVICE_ACTION_DISPLAY,  // display refresh
        COLOR______RED,         // red while refreshing display
        WAKEUP_ACTION_BUSY,     // do NOT allow wakeup while display is redrawing
        3,                      // 3 seconds delay which is a rather long and conservative estimation (buut the busy wakeup should take care of things)
        secondstime             // initially set, so the entry screen can render right away
    };
    device.deviceActions[DEVICE_ACTION_DEPOWER] = {
        DEVICE_ACTION_DEPOWER,  // depower display
        COLOR______RED,         // red while depowering
        WAKEUP_ACTION_BUTN,     // allow wakeup after depower, while waiting for a new measurement
        0                       // no delay required after this action
    };
    device.actionIndexCur = 0;
    device.actionIndexMax = 4;
    // deviceActions[DEVICE_ACTION_MEASURE].secondsNext = getMeasureNextSeconds();
    // actionIndexCur = DEVICE_ACTION_DISPLAY; // start with DEVICE_ACTION_DISPLAY for entry screen
    return device;
}

std::function<void(values_t* values, config_t* config)> Device::getFunctionByAction(device_action_e action) {
    if (action == DEVICE_ACTION_MEASURE) {
        return handleActionMeasure;
    } else if (action == DEVICE_ACTION_READVAL) {
        return handleActionReadval;
    } else if (action == DEVICE_ACTION_DISPLAY) {
        return handleActionDisplay;
    } else if (action == DEVICE_ACTION_DEPOWER) {
        return handleActionDepower;
    } else {
        return handleActionInvalid;
    }
}

void Device::handleActionInvalid(values_t* values, config_t* config) {
    // TODO :: should not be called, handle this condition
}

void Device::handleActionMeasure(values_t* values, config_t* config) {
    // co2
    SensorScd041::powerUp();
    SensorScd041::measure();
    // pressure
    SensorBme280::measure();
    // battery
    if (values->nextMeasureIndex % 3 == 0) {  // only measure pressure and battery every three minutes
        SensorEnergy::powerUp();
        SensorEnergy::measure();
    }
    values->nextMeasureIndex++;
}

void Device::handleActionReadval(values_t* values, config_t* config) {
    // read values from the sensors
    values_co2_t measurementCo2 = SensorScd041::readval();
    values_bme_t measurementBme = SensorBme280::readval();
    values_nrg_t measurementNrg = SensorEnergy::readval();
    // store values
    int currMeasureIndex = values->nextMeasureIndex - 1;
    values->measurements[currMeasureIndex % MEASUREMENT_BUFFER_SIZE] = {
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
    if (values->nextMeasureIndex % MEASUREMENT_BUFFER_SIZE == 0) {  // when the next measurement index is dividable by MEASUREMENT_BUFFER_SIZE, measurements need to be written to sd
        ModuleSdcard::begin();
        ModuleSdcard::persistValues(values);
    }
    // when pressureZerolevel == 0.0 pressure at sealevel needs to be recalculated (should only happen once at startup)
    if (config->pressureZerolevel == 0.0) {
        config->pressureZerolevel = SensorBme280::getPressureZerolevel(config->altitudeBaselevel, measurementBme.pressure);
    }
}

void Device::handleActionDisplay(values_t* values, config_t* config) {
    if (values->nextMeasureIndex > 0) {
        uint32_t currMeasureIndex = values->nextMeasureIndex - 1;
        values_all_t measurement = values->measurements[(currMeasureIndex + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE];
        if (config->disp.displayValModus == DISPLAY_VAL_M_TABLE) {
            ModuleScreen::renderTable(&measurement, config);
        } else {
            values_all_t history[HISTORY_____BUFFER_SIZE];
            ModuleSdcard::historyValues(values, config, history);  // will fill history with values from file or current measurements
            ModuleScreen::renderChart(history, config);
        }
        values->nextDisplayIndex = currMeasureIndex + config->disp.displayUpdateMinutes;
    } else {
        ModuleScreen::renderEntry(config);
    }
}

void Device::handleActionDepower(values_t* values, config_t* config) {
    ModuleScreen::hibernate();
    SensorEnergy::powerDown();  // redundant power down on battery monitor, seems to help with power reduction after redisplay
}
