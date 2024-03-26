#include "types/Device.h"

#include <Arduino.h>

#include "modules/ModuleDisplay.h"
#include "modules/ModuleSdcard.h"
#include "modules/ModuleServer.h"
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
    device.deviceActions[DEVICE_ACTION_SETTING] = {
        DEVICE_ACTION_SETTING,  // apply any settings (wifi on, calibration, ...)
        COLOR_____GRAY,         // gray while active
        WAKEUP_ACTION_BUTN,     // allow wakeup
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
    device.actionIndexCur = DEVICE_ACTION_DISPLAY;  // start with DEVICE_ACTION_DISPLAY for entry screen
    device.actionIndexMax = DEVICE_ACTION_DEPOWER + 1;

    return device;
}

std::function<void(values_t* values, config_t* config)> Device::getFunctionByAction(device_action_e action) {
    if (action == DEVICE_ACTION_MEASURE) {
        return handleActionMeasure;
    } else if (action == DEVICE_ACTION_READVAL) {
        return handleActionReadval;
    } else if (action == DEVICE_ACTION_SETTING) {
        return handleActionSetting;
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

    if (currMeasureIndex == 0) {
        // when pressureZerolevel == 0.0 pressure at sealevel needs to be recalculated (should only happen once at startup)
        if (config->pressureZerolevel == 0.0) {
            config->pressureZerolevel = SensorBme280::getPressureZerolevel(config->altitudeBaselevel, measurementBme.pressure);
        }
        // prefill the lowpass values
        for (uint8_t i = 0; i < MEASUREMENT_BUFFER_SIZE; i++) {
            values->measurements[i].valuesCo2.co2Raw = measurementCo2.co2Raw;
            // values->measurements[i].valuesBme.pressure = measurementBme.pressure;
        }
    }

    values->measurements[currMeasureIndex % MEASUREMENT_BUFFER_SIZE] = {
        SensorTime::getSecondstime(),  // secondstime as of RTC
        measurementCo2,                // sensorScd041 values
        measurementBme,                // sensorBme280 values
        measurementNrg,                // battery values
        true                           // publishable
    };
    // power down sensors
    SensorScd041::depower();
    // apply low pass filtering to co2 values
    // https://github.com/LinnesLab/KickFilters/blob/master/KickFilters.h
    float lowpass[LOWPASS_BUFFER_SIZE];

    uint8_t indexY = (values->nextMeasureIndex - LOWPASS_BUFFER_SIZE + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE;
    lowpass[0] = values->measurements[indexY].valuesCo2.co2Raw * LOWPASS_ALPHA;
    for (uint8_t i = 1; i < LOWPASS_BUFFER_SIZE; i++) {
        indexY = (values->nextMeasureIndex - LOWPASS_BUFFER_SIZE + i + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE;
        lowpass[i] = lowpass[i - 1] + (values->measurements[indexY].valuesCo2.co2Raw - lowpass[i - 1]) * LOWPASS_ALPHA;
    }
    values->measurements[currMeasureIndex].valuesCo2.co2Low = lowpass[LOWPASS_BUFFER_SIZE - 1];
    // #ifdef USE___SERIAL
    //     for (uint8_t i = 0; i < LOWPASS_BUFFER_SIZE; i++) {
    //         indexY = (values->nextMeasureIndex - LOWPASS_BUFFER_SIZE + i + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE;
    //         Serial.printf("idx: %02u, raw: %04u, low: %04u, lps: %f\n", indexY, values->measurements[indexY].valuesCo2.co2Raw, values->measurements[indexY].valuesCo2.co2Low, lowpass[i]);
    //     }
    // #endif
    // upon rollover, write measurements to SD card
    if (values->nextMeasureIndex % MEASUREMENT_BUFFER_SIZE == 0) {  // when the next measurement index is dividable by MEASUREMENT_BUFFER_SIZE, measurements need to be written to sd
        ModuleSdcard::begin();
        ModuleSdcard::persistValues(values);
    }
}

void Device::handleActionSetting(values_t* values, config_t* config) {
    // turn on wifi, if required and adapt display modus
    if (config->wifi.powered && !ModuleWifi::isPowered()) {
        if (ModuleWifi::powerup(config, true)) {
            config->disp.displayValSetng = DISPLAY_VAL_S____QR;
        }
    } else if (!config->wifi.powered && ModuleWifi::isPowered()) {
        ModuleWifi::depower(config);
    }
    bool autoConnect = values->nextConnectIndex <= values->nextMeasureIndex;
    bool autoShutoff = false;
    if (autoConnect) {
        if (!ModuleWifi::isPowered()) {
            autoShutoff = ModuleWifi::powerup(config, false);  // if the connection was successful, it also needs to be autoShutoff
        }
        SensorTime::configure(config);                                                            // apply timezone
        values->nextConnectIndex = values->nextMeasureIndex - 1 + config->time.ntpUpdateMinutes;  // TODO :: add config, then choose either MQTT update interval or NTP update interval
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
    if (ModuleServer::requestedCalibrationReference > 400) {
        SensorScd041::forceCalibration(ModuleServer::requestedCalibrationReference);
        // TODO :: reintroduce a renderCalibration display
        ModuleServer::requestedCalibrationReference = 0;
    }
}

void Device::handleActionDisplay(values_t* values, config_t* config) {
    uint32_t currMeasureIndex = values->nextMeasureIndex - 1;
    if (config->disp.displayValSetng == DISPLAY_VAL_S_ENTRY) {
        ModuleDisplay::renderEntry(config);  // splash screen
    } else if (config->disp.displayValSetng == DISPLAY_VAL_S____QR) {
        ModuleDisplay::renderQRCodes(config);
    } else if (config->disp.displayValModus == DISPLAY_VAL_M_TABLE) {
        values_all_t measurement = values->measurements[(currMeasureIndex + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE];
        ModuleDisplay::renderTable(&measurement, config);
        values->nextDisplayIndex = currMeasureIndex + config->disp.displayUpdateMinutes;
    } else if (config->disp.displayValModus == DISPLAY_VAL_M_CHART) {
        values_all_t history[HISTORY_____BUFFER_SIZE];
        ModuleSdcard::historyValues(values, config, history);  // will fill history with values from file or current measurements
        ModuleDisplay::renderChart(history, config);
        values->nextDisplayIndex = currMeasureIndex + config->disp.displayUpdateMinutes;
    } else {
        // TODO :: handle this
    }
    config->disp.displayValSetng = DISPLAY_VAL_S__NONE;
}

void Device::handleActionDepower(values_t* values, config_t* config) {
    ModuleDisplay::depower();
}
