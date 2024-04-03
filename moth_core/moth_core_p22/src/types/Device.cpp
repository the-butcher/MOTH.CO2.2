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
#include "types/Define.h"

calibration_t Device::calibrationResult;

device_t Device::load() {
    uint32_t secondstime = SensorTime::getSecondstime();
    // secondsCycleBase = SensorTime.getDate().secondstime();
    // secondsCycleBase = secondsCycleBase + 60 + SECONDS_BOOT_BUFFER - (secondsCycleBase + SECONDS_BOOT_BUFFER) % 60;  // first full minute after boot in secondstime

    device_t device;
    device.secondstimeBoot = secondstime;
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
    device.actionIndexMax = DEVICE_ACTION_DEPOWER;

    return device;
}

std::function<device_action_e(config_t& config, device_action_e maxDeviceAction)> Device::getFunctionByAction(device_action_e action) {
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

device_action_e Device::handleActionInvalid(config_t& config, device_action_e maxDeviceAction) {
    // TODO :: should not be called, handle this condition
    return DEVICE_ACTION_MEASURE;
}

device_action_e Device::handleActionMeasure(config_t& config, device_action_e maxDeviceAction) {

    SensorScd041::powerup();

    // TODO :: find out if compensationAltitude survives the powerDown and i2c off commands
    float pressure = SensorBme280::readval().pressure;
    if (pressure > 0) {
        uint16_t compensationAltitude = (uint16_t)round(SensorBme280::getAltitude(PRESSURE_ZERO, pressure));
        SensorScd041::setCompensationAltitude(compensationAltitude);
    }

    SensorScd041::measure();
    SensorBme280::measure();
    if (Values::values->nextMeasureIndex % 3 == 0) {  // only measure battery every three minutes to save some power
        SensorEnergy::powerup();
        SensorEnergy::measure();
        SensorEnergy::depower();
    }
    Values::values->nextMeasureIndex++;

    return DEVICE_ACTION_READVAL;  // read values after measuring
}

device_action_e Device::handleActionReadval(config_t& config, device_action_e maxDeviceAction) {

    // read values from the sensors
    values_co2_t measurementCo2 = SensorScd041::readval();
    values_bme_t measurementBme = SensorBme280::readval();
    values_nrg_t measurementNrg = SensorEnergy::readval();

    // store values
    uint32_t nextMeasureIndex = Values::values->nextMeasureIndex;
    uint32_t currMeasureIndex = nextMeasureIndex - 1;

    if (currMeasureIndex == 0) {
        // when pressureZerolevel == 0.0 pressure at sealevel needs to be recalculated (should only happen once at startup)
        if (config.pressureZerolevel == 0.0) {
            config.pressureZerolevel = SensorBme280::getPressureZerolevel(config.altitudeBaselevel, measurementBme.pressure);
        }
        // prefill the lowpass values
        for (uint8_t i = 0; i < MEASUREMENT_BUFFER_SIZE; i++) {
            Values::values->measurements[i].valuesCo2.co2Raw = measurementCo2.co2Raw;
        }
    }

    uint32_t currStorageIndex = currMeasureIndex % MEASUREMENT_BUFFER_SIZE;
    // #ifdef USE___SERIAL
    //     Serial.printf("currStorageIndex: %d\n", currStorageIndex);
    // #endif
    Values::values->measurements[currStorageIndex] = {
        SensorTime::getSecondstime(),  // secondstime as of RTC
        measurementCo2,                // sensorScd041 values
        measurementBme,                // sensorBme280 values
        measurementNrg,                // battery values
        true                           // publishable
    };

    // power down co2 sensor
    SensorScd041::depower();

    // apply low pass filtering to co2 values
    // https://github.com/LinnesLab/KickFilters/blob/master/KickFilters.h

    float lowpass[LOWPASS_____BUFFER_SIZE];
    uint8_t indexY = (nextMeasureIndex - LOWPASS_____BUFFER_SIZE + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE;
    lowpass[0] = Values::values->measurements[indexY].valuesCo2.co2Raw * LOWPASS___________ALPHA;
    for (uint8_t i = 1; i < LOWPASS_____BUFFER_SIZE; i++) {
        indexY = (nextMeasureIndex - LOWPASS_____BUFFER_SIZE + i + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE;
        lowpass[i] = lowpass[i - 1] + (Values::values->measurements[indexY].valuesCo2.co2Raw - lowpass[i - 1]) * LOWPASS___________ALPHA;
    }

    uint16_t co2Lpf = (uint16_t)round(lowpass[LOWPASS_____BUFFER_SIZE - 1]);

    // replace with a measurement containing the low pass value
    Values::values->measurements[currStorageIndex] = {
        SensorTime::getSecondstime(),  // secondstime as of RTC
        {
            co2Lpf,                // lowpass
            measurementCo2.deg,    // temperature
            measurementCo2.hum,    // humidity
            measurementCo2.co2Raw  // original co2 value
        },
        measurementBme,  // sensorBme280 values
        measurementNrg,  // battery values
        true             // publishable
    };

    if (config.sign.signalValSound == SIGNAL__VAL______ON && co2Lpf >= config.disp.thresholdsCo2.rHi) {
        ModuleSignal::beep();
    }

    // upon rollover, write measurements to SD card
    if (Values::values->nextMeasureIndex % MEASUREMENT_BUFFER_SIZE == 0) {  // when the next measurement index is dividable by MEASUREMENT_BUFFER_SIZE, measurements need to be written to sd
        ModuleSdcard::begin();
        ModuleSdcard::persistValues();
    }

    if (maxDeviceAction == DEVICE_ACTION_READVAL) {
        if (config.disp.displayValCycle == DISPLAY_VAL_Y____SIG) {
            uint16_t lastCo2Lpf = Values::values->measurements[Values::values->lastDisplayIndex % MEASUREMENT_BUFFER_SIZE].valuesCo2.co2Lpf;
            uint16_t currCo2Lpf = Values::values->measurements[(Values::values->nextMeasureIndex - 1) % MEASUREMENT_BUFFER_SIZE].valuesCo2.co2Lpf;
            if (Values::isSignificantChange(lastCo2Lpf, currCo2Lpf)) {
                return DEVICE_ACTION_DISPLAY;  // skip settings and advance directly to display
            }
        }
        return DEVICE_ACTION_MEASURE;  // meant to measure AND no displayable, significant change
    } else {
        return DEVICE_ACTION_SETTING;  // setting will trigger a redraw, therefore no need to check for significant change
    }
}

device_action_e Device::handleActionSetting(config_t& config, device_action_e maxDeviceAction) {

    // turn on wifi, if required and adapt display modus
    if (config.wifi.wifiValPower == WIFI____VAL_P_PND_Y && !ModuleWifi::isPowered()) {  // to be turned on, but currently off
        if (ModuleWifi::powerup(config, true)) {
            config.disp.displayValSetng = DISPLAY_VAL_S_____QR;
        }
    } else if (config.wifi.wifiValPower == WIFI____VAL_P_PND_N && ModuleWifi::isPowered()) {  // to be turned off, but current on
        ModuleWifi::depower(config);
    }

    bool autoConnect = Values::values->nextAutoConIndex <= Values::values->nextMeasureIndex;
    bool autoShutoff = false;
    if (autoConnect) {
        if (!ModuleWifi::isPowered()) {
            autoShutoff = ModuleWifi::powerup(config, false);  // if the connection was successful, it also needs to be autoShutoff
        }
        SensorTime::configure(config);                                                                           // apply timezone
        Values::values->nextAutoConIndex = Values::values->nextMeasureIndex - 1 + config.time.ntpUpdateMinutes;  // TODO :: add config, then choose either MQTT update interval or NTP update interval
        if (autoShutoff) {
            for (int i = 0; i < 25; i++) {
                if (!SensorTime::isNtpWait()) {
                    break;
                }
                delay(100);
            }
            ModuleWifi::depower(config);
        }
    }

    if (ModuleServer::requestedCo2Ref > 400) {
        SensorScd041::powerup();
        Device::calibrationResult = SensorScd041::forceCalibration(ModuleServer::requestedCo2Ref);  // calibrate and store result
        config.disp.displayValSetng = DISPLAY_VAL_S____CO2;                                         // next display should show the calibration result
        ModuleServer::requestedCo2Ref = 0;                                                          // reset requested calibration value
        SensorScd041::depower();
    } else if (ModuleServer::requestedCo2Rst) {
        SensorScd041::powerup();
        Device::calibrationResult = SensorScd041::forceReset();  // reset and store result
        config.disp.displayValSetng = DISPLAY_VAL_S____CO2;      // next display should show the calibration result
        ModuleServer::requestedCo2Rst = false;
        SensorScd041::depower();
    }

    return DEVICE_ACTION_DISPLAY;  // when settings runs, there should always be a redraw
}

device_action_e Device::handleActionDisplay(config_t& config, device_action_e maxDeviceAction) {
    uint32_t currMeasureIndex = Values::values->nextMeasureIndex - 1;
    if (config.disp.displayValSetng == DISPLAY_VAL_S__ENTRY) {
        ModuleDisplay::renderEntry(config);  // splash screen
    } else if (config.disp.displayValSetng == DISPLAY_VAL_S_____QR) {
        ModuleDisplay::renderQRCodes(config);
        Values::values->nextDisplayIndex = currMeasureIndex + 1;  // wait a minute before next update
    } else if (config.disp.displayValSetng == DISPLAY_VAL_S____CO2) {
        ModuleDisplay::renderCo2(config, calibrationResult);
        Values::values->nextDisplayIndex = currMeasureIndex + 1;  // wait a minute before next update
    } else if (config.disp.displayValModus == DISPLAY_VAL_M__TABLE) {
        values_all_t measurement = Values::values->measurements[(currMeasureIndex + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE];
        ModuleDisplay::renderTable(measurement, config);
        Values::values->lastDisplayIndex = currMeasureIndex;
        Values::values->nextDisplayIndex = currMeasureIndex + config.disp.displayUpdateMinutes;
    } else if (config.disp.displayValModus == DISPLAY_VAL_M__CHART) {
        values_all_t history[HISTORY_____BUFFER_SIZE];
        ModuleSdcard::historyValues(config, history);  // will fill history with values from file or current measurements
        ModuleDisplay::renderChart(history, config);
        Values::values->lastDisplayIndex = currMeasureIndex;
        Values::values->nextDisplayIndex = currMeasureIndex + config.disp.displayUpdateMinutes;
    } else {
        // TODO :: handle this
    }
    config.disp.displayValSetng = DISPLAY_VAL_S___NONE;
    return DEVICE_ACTION_DEPOWER;  // display must be depowered after drawing
}

device_action_e Device::handleActionDepower(config_t& config, device_action_e maxDeviceAction) {
    ModuleDisplay::depower();
    return DEVICE_ACTION_MEASURE;  // after redrawing pause, then measure
}
