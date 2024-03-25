#include "types/Device.h"

#include <Arduino.h>

#include "modules/ModuleDisplay.h"
#include "modules/ModuleSdcard.h"
#include "modules/ModuleSignal.h"
#include "modules/ModuleWifi.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"
#include "sensors/SensorTime.h"

device_t Device::load() {
    uint32_t secondstime = SensorTime::getSecondstime();
    // secondsCycleBase = SensorTime.getDate().secondstime();
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
        secondstime             // initially set, so the entry screen will render right after start
    };
    device.deviceActions[DEVICE_ACTION_DEPOWER] = {
        DEVICE_ACTION_DEPOWER,  // depower display
        COLOR______RED,         // red while depowering
        WAKEUP_ACTION_BUTN,     // allow wakeup after depower, while waiting for a new measurement
        0                       // no delay required after this action
    };
    // device.actionIndexCur = 0;
    device.actionIndexCur = DEVICE_ACTION_DISPLAY;  // start with DEVICE_ACTION_DISPLAY for entry screen
    device.actionIndexMax = 4;
    // deviceActions[DEVICE_ACTION_MEASURE].secondsNext = getMeasureNextSeconds();
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
    SensorScd041::powerup();
    SensorScd041::measure();
    // pressure
    SensorBme280::measure();
    // battery
    if (values->nextMeasureIndex % 3 == 0) {  // only measure battery every three minutes to save some power
        SensorEnergy::powerup();
        SensorEnergy::measure();
        SensorEnergy::depower();
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
        SensorTime::getSecondstime(),  // secondstime as of RTC
        measurementCo2,                // sensorScd041 values
        measurementBme,                // sensorBme280 values
        measurementNrg,                // battery values
        true                           // publishable
    };
    // power down sensors
    SensorScd041::depower();
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

        if (config->wifi.powered && !ModuleWifi::isPowered()) {
            ModuleWifi::powerup(config);
        } else if (!config->wifi.powered && ModuleWifi::isPowered()) {
            ModuleWifi::depower(config);
        }

        bool autoConnect = values->nextConnectIndex <= values->nextDisplayIndex;
        bool autoShutoff = false;
        if (autoConnect) {
            if (!ModuleWifi::isPowered()) {
                autoShutoff = ModuleWifi::powerup(config);  // if the connection was successful, it also needs to be autoShutoff
            }
            SensorTime::configure(config);                     // apply timezone
            values->nextConnectIndex = currMeasureIndex + 60;  // TODO :: add config, then choose either MQTT update interval or NTP update interval
            if (autoShutoff) {
                for (int i = 0; i < 20; i++) {
                    if (!SensorTime::isNtpWait()) {
                        break;
                    }
                    delay(50);
                }
                ModuleWifi::depower(config);
            }
        }

        // leaving the WiFi on longer than necessary (i.e. a full display render duration) for autoConnect will consume a lot of energy

        values_all_t measurement = values->measurements[(currMeasureIndex + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE];
        if (config->disp.displayValModus == DISPLAY_VAL_M_TABLE) {
            ModuleDisplay::renderTable(&measurement, config);
        } else {
            values_all_t history[HISTORY_____BUFFER_SIZE];
            ModuleSdcard::historyValues(values, config, history);  // will fill history with values from file or current measurements
            ModuleDisplay::renderChart(history, config);
        }
        values->nextDisplayIndex = currMeasureIndex + config->disp.displayUpdateMinutes;

    } else {
        ModuleDisplay::renderEntry(config);  // splash screen
    }
}

void Device::handleActionDepower(values_t* values, config_t* config) {
    ModuleDisplay::depower();
}
