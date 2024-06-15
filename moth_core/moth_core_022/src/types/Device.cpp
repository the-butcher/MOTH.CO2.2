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

co2cal______t Device::calibrationResult;
uint32_t Device::secondstimeBoot;

device_t Device::load() {

    uint32_t secondstime = SensorTime::getSecondstime();
    device_t device;
    device.deviceActions[DEVICE_ACTION_POWERUP] = {
        DEVICE_ACTION_POWERUP,  // turn on anything that needs to be turned on
        COLOR____GREEN,         // green to indicate powerup
        WAKEUP_ACTION_BUTN,     // allow wakeup while measurement is active
        0                       // 0 for no warmup
    };
    device.deviceActions[DEVICE_ACTION_MEASURE] = {
        DEVICE_ACTION_MEASURE,  // trigger measurements
        COLOR__MAGENTA,         // magenta while measuring
        WAKEUP_ACTION_BUTN,     // allow wakeup while measurement is active
#ifdef USE_PERIODIC
        0  // no delay in case of periodic measurement
#else
        5  // 5 seconds delay to complete measurement
#endif
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
        3,                      // 3 seconds delay which is a rather long and conservative estimation (but the busy wakeup should take care of things at the best possible moment)
        secondstime             // initially set, so the entry screen will render right after start
    };
    device.deviceActions[DEVICE_ACTION_DEPOWER] = {
        DEVICE_ACTION_DEPOWER,  // depower display
        COLOR___ORANGE,         // red while depowering
        WAKEUP_ACTION_BUSY,     // when waiting for this action, the display's busy pin must become high
        0                       // no delay required after this action
    };
    device.actionIndexCur = DEVICE_ACTION_SETTING;  // start with DEVICE_ACTION_SETTING for entry screen (allows for NTP update before first measurements)
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
    // TODO :: should never be called, handle this condition
    return DEVICE_ACTION_POWERUP;
}

device_action_e Device::handleActionPowerup(config_t& config, device_action_e maxDeviceAction) {

    SensorScd041::powerup(config);
    if (Values::isEnergyCycle()) {
        SensorEnergy::powerup();
    }

    return DEVICE_ACTION_MEASURE;  // measure after powering up
}

device_action_e Device::handleActionMeasure(config_t& config, device_action_e maxDeviceAction) {

    // use previous measurement for pressure compensation
    if (Values::values->nextMeasureIndex > 0) {
        SensorScd041::setCompensationPressure(Values::latest().valuesBme.pressure);
    }

    SensorScd041::measure();
    SensorBme280::measure();
    SensorEnergy::measure();

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

    // ok to use raw pressure value since the first value is unfiltered anyways
    if (currMeasureIndex == 0) {
        // when pressureZerolevel == 0.0 pressure at sealevel needs to be recalculated (should only happen once at startup)
        if (config.sbme.pressureZerolevel == 0.0) {
            config.sbme.pressureZerolevel = SensorBme280::getPressureZerolevel(config.sbme.altitudeBaselevel, measurementBme.pressure);
        }
    }

    Values::values->nextMeasureIndex = nextMeasureIndex;

    // is storing the measurement needed? (will be replaced a few lines further down)
    Values::values->measurements[currStorageIndex] = {
        SensorTime::getSecondstime(),  // secondstime as of RTC
        measurementCo2,                // sensorScd041 values
        measurementBme,                // sensorBme280 values
        measurementNrg,                // battery values
        true                           // publishable
    };

    // early power down of sensors (will save around 2 sensor power seconds while the display is redrawing)
    SensorScd041::depower(config);
    SensorEnergy::depower();

    // co2 low pass filter
    float co2LpfPrev = Values::values->measurements[prevStorageIndex].valuesCo2.co2Lpf / VALUE_SCALE_CO2LPF;
    float co2RawCurr = measurementCo2.co2Raw;
    float co2LpfCurr = co2LpfPrev * (1 - config.sco2.lpFilterRatioCurr) + co2RawCurr * config.sco2.lpFilterRatioCurr;

    // pressure low pass filter
    float pressureLpfPrev = Values::values->measurements[prevStorageIndex].valuesBme.pressure;
    float pressureRawCurr = measurementBme.pressure;
    float pressureLpfCurr = pressureLpfPrev * (1 - config.sbme.lpFilterRatioCurr) + pressureRawCurr * config.sbme.lpFilterRatioCurr;
    if (pressureLpfCurr > 0) {
        SensorScd041::setCompensationPressure(pressureLpfCurr);
    }

    // replace with a measurement containing the low pass value
    Values::values->measurements[currStorageIndex] = {
        SensorTime::getSecondstime(),  // secondstime as of RTC
        {
            (uint16_t)round(co2LpfCurr * VALUE_SCALE_CO2LPF),  // filtered
            measurementCo2.deg,                                // temperature
            measurementCo2.hum,                                // humidity
            measurementCo2.co2Raw                              // original co2 value
        },
        {
            pressureLpfCurr  // lowpass pressure
        },                   //
        measurementNrg,      // battery values
        true                 // publishable
    };

    if (config.sign.signalValSound == SIGNAL__VAL______ON && co2LpfCurr >= config.disp.thresholdsCo2.rHi) {
        ModuleSignal::beep();
    }

    // upon rollover, write measurements to SD card
    if (Values::values->nextMeasureIndex % MEASUREMENT_BUFFER_SIZE == 0) {  // when the next measurement index is dividable by MEASUREMENT_BUFFER_SIZE, measurements need to be written to sd
        ModuleCard::persistValues();
    }

    if (maxDeviceAction == DEVICE_ACTION_READVAL) {
        if (config.disp.displayValCycle == DISPLAY_VAL_Y____SIG) {
            float lastCo2Lpf = Values::values->measurements[Values::values->lastDisplayIndex % MEASUREMENT_BUFFER_SIZE].valuesCo2.co2Lpf / VALUE_SCALE_CO2LPF;
            float currCo2Lpf = Values::latest().valuesCo2.co2Lpf / VALUE_SCALE_CO2LPF;
            if (Values::isSignificantChange(lastCo2Lpf, currCo2Lpf)) {
                return DEVICE_ACTION_DISPLAY;  // skip settings and advance directly to display
            }
        }
        return DEVICE_ACTION_POWERUP;  // meant to measure AND not displayable, NO significant change
    } else {
        return DEVICE_ACTION_SETTING;  // advance to settings, where another decision will be made whether to continue with display or not
    }
}

device_action_e Device::handleActionSetting(config_t& config, device_action_e maxDeviceAction) {

    uint32_t currMeasureIndex = Values::values->nextMeasureIndex - 1;

    // turn on wifi, if required and adapt display modus
    bool isOrWasWifiPowered = ModuleWifi::isPowered();
    if (config.wifi.wifiValPower == WIFI____VAL_P__PND_Y && !ModuleWifi::isPowered()) {  // to be turned on, but currently off
        if (ModuleWifi::powerup(config, true)) {
            config.disp.displayValSetng = DISPLAY_VAL_S_____QR;  // let the display render qr next time
        }
        isOrWasWifiPowered = true;
    } else if (config.wifi.wifiValPower == WIFI____VAL_P__PND_N) {  // to be turned off, no check for powered state since the device could have silently lost connection
        ModuleWifi::depower(config);
        isOrWasWifiPowered = true;
    }

    bool autoDepower = false;
    if (!isOrWasWifiPowered) {  // only auto connect while wifi is not on (or was recently on) to prevent memory race conditions

        bool autoNtpConn = Values::values->nextAutoNtpIndex <= Values::values->nextMeasureIndex;
        bool autoPubConn = Values::values->nextAutoPubIndex <= Values::values->nextMeasureIndex;

        if (autoNtpConn || autoPubConn) {
#ifdef USE___SERIAL
            Serial.printf("autoNtpConn: %u, autoPubConn: %u\n", autoNtpConn, autoPubConn);
#endif
            autoDepower = ModuleWifi::powerup(config, false);  // if the connection was successful, it also needs to be autoShutoff
            if (autoDepower) {                                 // was connected

                if (autoPubConn) {
                    // try to publish (call with config, so mqtt gets the opportunity to )
                    ModuleMqtt::publish(config);
                    Values::values->nextAutoPubIndex = config.mqtt.mqttPublishMinutes == MQTT_PUBLISH___NEVER ? MQTT_PUBLISH___NEVER : currMeasureIndex + config.mqtt.mqttPublishMinutes;
                }
                if (autoNtpConn) {
#ifdef USE___SERIAL
                    Serial.printf("init ntp update\n");
#endif
                    SensorTime::setupNtpUpdate(config);                                                  // apply timezone
                    Values::values->nextAutoNtpIndex = currMeasureIndex + config.time.ntpUpdateMinutes;  // TODO :: add config, then choose either MQTT update interval or NTP update interval
                    for (int i = 0; i < 100; i++) {                                                      // wait 10 secs max for time sync
                        if (!SensorTime::isNtpWait()) {
#ifdef USE___SERIAL
                            Serial.printf("done ntp update, dutc: %u\n", SensorTime::secondstimeOffsetUtc);
#endif
                            break;
                        }
                        delay(100);
                    }
#ifdef USE___SERIAL
                    Serial.printf("exit ntp update, dutc: %u\n", SensorTime::secondstimeOffsetUtc);
#endif
                }
                ModuleWifi::depower(config);
            }
        }
    }

    if (config.sco2.requestedCo2Ref >= 400) {
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
        SensorScd041::configure(config);  // re-apply temperature offset (or it would be lost by resetting the sensor)
        SensorScd041::depower(config);
    }

    if (maxDeviceAction == DEVICE_ACTION_SETTING) {
        if (config.disp.displayValCycle == DISPLAY_VAL_Y____SIG) {
            float lastCo2Lpf = Values::values->measurements[Values::values->lastDisplayIndex % MEASUREMENT_BUFFER_SIZE].valuesCo2.co2Lpf / VALUE_SCALE_CO2LPF;
            float currCo2Lpf = Values::latest().valuesCo2.co2Lpf / VALUE_SCALE_CO2LPF;
            if (Values::isSignificantChange(lastCo2Lpf, currCo2Lpf)) {
                return DEVICE_ACTION_DISPLAY;  // continue with display due to significant change
            }
        }
        return DEVICE_ACTION_POWERUP;  // meant to end with settings (i.e. mqtt publish)
    } else {
        return DEVICE_ACTION_DISPLAY;  // advance to display
    }
}

device_action_e Device::handleActionDisplay(config_t& config, device_action_e maxDeviceAction) {
    uint32_t currMeasureIndex = Values::values->nextMeasureIndex - 1;
    if (config.disp.displayValSetng == DISPLAY_VAL_S__ENTRY) {
        ModuleDisp::renderEntry(config);  // splash screen
    } else if (config.disp.displayValSetng == DISPLAY_VAL_S_____QR) {
        ModuleDisp::renderQRCodes(config);
        Values::values->nextDisplayIndex = currMeasureIndex + 1;  // wait a minute before next update
    } else if (config.disp.displayValSetng == DISPLAY_VAL_S____CO2) {
        ModuleDisp::renderCo2Cal(calibrationResult, config);
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
    } else if (config.disp.displayValModus == DISPLAY_VAL_M__CALIB) {
        co2cal______t co2cal = Values::getCo2Cal();
        ModuleDisp::renderCo2Cal(co2cal, config);
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
    SensorEnergy::depower();       // redundant, but battery monitor does not seem to depower properly after some display cycles
    return DEVICE_ACTION_POWERUP;  // after redrawing pause, then measure
}
