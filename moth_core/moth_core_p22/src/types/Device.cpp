#include "types/Device.h"

#include "buttons/ButtonAction.h"
#include "modules/ModuleCard.h"
#include "modules/ModuleDisp.h"
#include "modules/ModuleHttp.h"
#include "modules/ModuleMqtt.h"
#include "modules/ModuleSignal.h"
#include "modules/ModuleWifi.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"
#include "sensors/SensorTime.h"
#include "types/Define.h"

calibration_t Device::calibrationResult;
uint32_t Device::secondstimeBoot;

device_t Device::load() {

    uint32_t secondstime = SensorTime::getSecondstime();
    device_t device;
    device.deviceActions[DEVICE_ACTION_POWERUP] = {
        DEVICE_ACTION_POWERUP,  // trigger measurements
        COLOR____GREEN,         // grenn to indicate powerup
        WAKEUP_ACTION_BUTN,     // allow wakeup while measurement is active
        3                       // 5 seconds delay until measurement
    };
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
        WAKEUP_ACTION_BUTN,     // do NOT allow wakeup while display is redrawing
        3,                      // 3 seconds delay which is a rather long and conservative estimation (buut the busy wakeup should take care of things at the best possible moment)
        secondstime             // initially set, so the entry screen will render right after start
    };
    device.deviceActions[DEVICE_ACTION_DEPOWER] = {
        DEVICE_ACTION_DEPOWER,  // depower display
        COLOR___ORANGE,         // red while depowering
        WAKEUP_ACTION_BUSY,     // when waiting for this action, the display's busy pin must become high
        0                       // no delay required after this action
    };
    device.actionIndexCur = DEVICE_ACTION_DISPLAY;  // start with DEVICE_ACTION_DISPLAY for entry screen
    device.actionIndexMax = DEVICE_ACTION_DEPOWER;

    // calculate ext1Bitmask, sorting the pins is a precaution to ensure descending order
    gpio_num_t ext1Pins[4] = {ButtonAction::A.gpin, ButtonAction::B.gpin, PIN_RTC_SQW, ButtonAction::C.gpin};
    qsort(ext1Pins, 4, sizeof(gpio_num_t), Device::cmpfunc);
    uint64_t ext1Bitmask = 0;
    for (uint8_t i = 0; i < 4; i++) {
        ext1Bitmask |= 1ULL << ext1Pins[i];
    }
    device.ext1Bitmask = ext1Bitmask;

    return device;
}

int Device::cmpfunc(const void* a, const void* b) {
    return (*(gpio_num_t*)b - *(gpio_num_t*)a);
}

void Device::begin(uint32_t secondstimeBoot) {
    Device::secondstimeBoot = secondstimeBoot;
}

std::function<device_action_e(config_t& config, device_action_e maxDeviceAction)> Device::getFunctionByAction(device_action_e action) {
    if (action == DEVICE_ACTION_POWERUP) {
        return handleActionPowerup;
    } else if (action == DEVICE_ACTION_MEASURE) {
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
    return DEVICE_ACTION_POWERUP;
}

/**
 * check if this cycle should be used to take a battery measurement
 */
bool Device::isEnergyCycle() {
    return (Values::values->nextMeasureIndex) % 5 == 0;
}

device_action_e Device::handleActionPowerup(config_t& config, device_action_e maxDeviceAction) {

    SensorScd041::powerup(config);
    if (Device::isEnergyCycle()) {
        SensorEnergy::powerup();
    }

    return DEVICE_ACTION_MEASURE;  // measure after powering up
}

device_action_e Device::handleActionMeasure(config_t& config, device_action_e maxDeviceAction) {

    SensorScd041::measure();
    SensorBme280::measure();
    if (Device::isEnergyCycle()) {
        SensorEnergy::measure();
    }

    return DEVICE_ACTION_READVAL;  // read values after measuring
}

device_action_e Device::handleActionReadval(config_t& config, device_action_e maxDeviceAction) {

    // read values from the sensors
    values_co2_t measurementCo2 = SensorScd041::readval();
    values_bme_t measurementBme = SensorBme280::readval();
    values_nrg_t measurementNrg = SensorEnergy::readval();

    // store values
    uint32_t currMeasureIndex = Values::values->nextMeasureIndex;  // not yet incremented
    uint32_t nextMeasureIndex = currMeasureIndex + 1;
    uint32_t currStorageIndex = currMeasureIndex % MEASUREMENT_BUFFER_SIZE;  // the index of this measurement in the data
    uint32_t prevStorageIndex = currMeasureIndex > 0 ? (currMeasureIndex - 1) % MEASUREMENT_BUFFER_SIZE : 0;

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
    Values::values->nextMeasureIndex = nextMeasureIndex;

    float pressure = measurementBme.pressure;
    if (pressure > 0) {
        uint16_t compensationAltitude = (uint16_t)round(SensorBme280::getAltitude(PRESSURE_ZERO, pressure));
        SensorScd041::setCompensationAltitude(compensationAltitude);
    }

    Values::values->measurements[currStorageIndex] = {
        SensorTime::getSecondstime(),  // secondstime as of RTC
        measurementCo2,                // sensorScd041 values
        measurementBme,                // sensorBme280 values
        measurementNrg,                // battery values
        true                           // publishable
    };

    // power down early sensors (will save around 2 sensor power seconds while the display is redrawing)
    SensorScd041::depower(config);
    SensorEnergy::depower();

    // some filtering (see excel in nonrepo folder)
    uint16_t co2LpfPrev = Values::values->measurements[prevStorageIndex].valuesCo2.co2Lpf;
    uint16_t co2RawCurr = measurementCo2.co2Raw;

    // #ifdef USE___SERIAL
    //     Serial.printf("co2LpfPrev: %d, co2RawCurr: %d\n", co2LpfPrev, co2RawCurr);
    // #endif

    float EXP = 3.0f;
    float co2CurrRatio = min(1.0f, 1 / EXP * pow(1 + abs(co2RawCurr - co2LpfPrev) * 1.0f / co2LpfPrev, EXP));  // when there is a large delta, ratio must be limited
    float co2PrevRatio = 1 - co2CurrRatio;

    uint16_t co2LpfCurr = (uint16_t)round(co2LpfPrev * co2PrevRatio + co2RawCurr * co2CurrRatio);

    // #ifdef USE___SERIAL
    //     Serial.printf("co2LpfCurr: %d\n", co2LpfCurr);
    // #endif

    // replace with a measurement containing the low pass value
    Values::values->measurements[currStorageIndex] = {
        SensorTime::getSecondstime(),  // secondstime as of RTC
        {
            co2LpfCurr,            // lowpass
            measurementCo2.deg,    // temperature
            measurementCo2.hum,    // humidity
            measurementCo2.co2Raw  // original co2 value
        },
        measurementBme,  // sensorBme280 values
        measurementNrg,  // battery values
        true             // publishable
    };

    if (config.sign.signalValSound == SIGNAL__VAL______ON && co2LpfCurr >= config.disp.thresholdsCo2.rHi) {
        ModuleSignal::beep();
    }

    // upon rollover, write measurements to SD card
    if (Values::values->nextMeasureIndex % MEASUREMENT_BUFFER_SIZE == 0) {  // when the next measurement index is dividable by MEASUREMENT_BUFFER_SIZE, measurements need to be written to sd
        ModuleCard::begin();
        ModuleCard::persistValues();
    }

    if (maxDeviceAction == DEVICE_ACTION_READVAL) {
        if (config.disp.displayValCycle == DISPLAY_VAL_Y____SIG) {
            uint16_t lastCo2Lpf = Values::values->measurements[Values::values->lastDisplayIndex % MEASUREMENT_BUFFER_SIZE].valuesCo2.co2Lpf;
            uint16_t currCo2Lpf = Values::values->measurements[(Values::values->nextMeasureIndex - 1) % MEASUREMENT_BUFFER_SIZE].valuesCo2.co2Lpf;
            if (Values::isSignificantChange(lastCo2Lpf, currCo2Lpf)) {
                return DEVICE_ACTION_DISPLAY;  // skip settings and advance directly to display
            }
        }
        return DEVICE_ACTION_POWERUP;  // meant to measure AND not displayable, NO significant change
    } else {
        return DEVICE_ACTION_SETTING;  // setting will trigger a redraw, therefore no need to check for significant change
    }
}

device_action_e Device::handleActionSetting(config_t& config, device_action_e maxDeviceAction) {
    uint32_t currMeasureIndex = Values::values->nextMeasureIndex - 1;

    // turn on wifi, if required and adapt display modus
    if (config.wifi.wifiValPower == WIFI____VAL_P__PND_Y && !ModuleWifi::isPowered()) {  // to be turned on, but currently off
        if (ModuleWifi::powerup(config, true)) {
            config.disp.displayValSetng = DISPLAY_VAL_S_____QR;
        }
    } else if (config.wifi.wifiValPower == WIFI____VAL_P__PND_N && ModuleWifi::isPowered()) {  // to be turned off, but current on
        ModuleWifi::depower(config);
    }

    bool autoNtpConn = Values::values->nextAutoNtpIndex <= Values::values->nextMeasureIndex;
    bool autoPubConn = Values::values->nextAutoPubIndex <= Values::values->nextMeasureIndex;
    bool autoDepower = false;
    if (autoNtpConn || autoPubConn) {
        if (!ModuleWifi::isPowered()) {                        // is not on already
            autoDepower = ModuleWifi::powerup(config, false);  // if the connection was successful, it also needs to be autoShutoff
        }
        if (autoNtpConn) {
            SensorTime::setupNtpUpdate(config);                                                  // apply timezone
            Values::values->nextAutoNtpIndex = currMeasureIndex + config.time.ntpUpdateMinutes;  // TODO :: add config, then choose either MQTT update interval or NTP update interval
        }
        if (autoPubConn) {
            // try to publish (call with config, so mqtt gets the opportunity to )
            ModuleMqtt::publish(config);
            Values::values->nextAutoPubIndex = config.mqtt.mqttPublishMinutes == MQTT_PUBLISH___NEVER ? MQTT_PUBLISH___NEVER : currMeasureIndex + config.mqtt.mqttPublishMinutes;
        }
        if (autoDepower) {
            for (int i = 0; i < 25; i++) {
                if (!SensorTime::isNtpWait()) {
                    break;
                }
                delay(100);
            }
            ModuleWifi::depower(config);
        }
    }

    if (config.sco2.requestedCo2Ref > 400) {
        SensorScd041::powerup(config);
        Device::calibrationResult = SensorScd041::forceCalibration(config.sco2.requestedCo2Ref);  // calibrate and store result
        config.disp.displayValSetng = DISPLAY_VAL_S____CO2;                                       // next display should show the calibration result
        config.sco2.requestedCo2Ref = 0;                                                          // reset requested calibration value
        SensorScd041::depower(config);
    } else if (config.sco2.requestedCo2Rst) {
        SensorScd041::powerup(config);
        Device::calibrationResult = SensorScd041::forceReset();  // reset and store result
        config.disp.displayValSetng = DISPLAY_VAL_S____CO2;      // next display should show the calibration result
        config.sco2.requestedCo2Rst = false;
        SensorScd041::depower(config);
    } else if (config.sco2.requestedCo2Tst) {
        SensorScd041::powerup(config);
        Device::calibrationResult = SensorScd041::forceSelfTest();  // reset and store result
        config.disp.displayValSetng = DISPLAY_VAL_S____CO2;         // next display should show the calibration result
        config.sco2.requestedCo2Tst = false;
        SensorScd041::depower(config);
    }

    return DEVICE_ACTION_DISPLAY;  // when settings runs, there should always be a redraw
}

device_action_e Device::handleActionDisplay(config_t& config, device_action_e maxDeviceAction) {
    uint32_t currMeasureIndex = Values::values->nextMeasureIndex - 1;
    if (config.disp.displayValSetng == DISPLAY_VAL_S__ENTRY) {
        ModuleDisp::renderEntry(config);  // splash screen
    } else if (config.disp.displayValSetng == DISPLAY_VAL_S_____QR) {
        ModuleDisp::renderQRCodes(config);
        Values::values->nextDisplayIndex = currMeasureIndex + 1;  // wait a minute before next update
    } else if (config.disp.displayValSetng == DISPLAY_VAL_S____CO2) {
        ModuleDisp::renderCo2(config, calibrationResult);
        Values::values->nextDisplayIndex = currMeasureIndex + 1;  // wait a minute before next update
    } else if (config.disp.displayValModus == DISPLAY_VAL_M__TABLE) {
        values_all_t measurement = Values::values->measurements[(currMeasureIndex + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE];
        ModuleDisp::renderTable(measurement, config);
        Values::values->lastDisplayIndex = currMeasureIndex;
        Values::values->nextDisplayIndex = currMeasureIndex + config.disp.displayUpdateMinutes;
    } else if (config.disp.displayValModus == DISPLAY_VAL_M__CHART) {
        values_all_t history[HISTORY_____BUFFER_SIZE];
        ModuleCard::historyValues(config, history);  // will fill history with values from file or current measurements
        ModuleDisp::renderChart(history, config);
        Values::values->lastDisplayIndex = currMeasureIndex;
        Values::values->nextDisplayIndex = currMeasureIndex + config.disp.displayUpdateMinutes;
    } else {
        // TODO :: handle this
    }
    config.disp.displayValSetng = DISPLAY_VAL_S___NONE;
    return DEVICE_ACTION_DEPOWER;  // display must be depowered after drawing
}

device_action_e Device::handleActionDepower(config_t& config, device_action_e maxDeviceAction) {
    ModuleDisp::depower();
    SensorEnergy::depower();       // redundant, but battery monitor does not seem to depower properly after display cycles
    return DEVICE_ACTION_POWERUP;  // after redrawing pause, then measure
}
