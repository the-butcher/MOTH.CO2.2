#include "types/Device.h"

#include "modules/ModuleCard.h"
#include "modules/ModuleDisp.h"
#include "modules/ModuleMqtt.h"
#include "modules/ModuleSignal.h"
#include "modules/ModuleWifi.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorPms003.h"
#include "sensors/SensorTime.h"
#include "types/Define.h"

uint32_t Device::secondstimeBoot;

device_t Device::load() {

    uint32_t secondstime = SensorTime::getSecondstime();
    device_t device;
    device.deviceActions[DEVICE_ACTION_MEASURE] = {
        DEVICE_ACTION_MEASURE,  // trigger measurements
        COLOR__MAGENTA,         // magenta while measuring
        2                       // 2 seconds delay to complete measurement
    };
    device.deviceActions[DEVICE_ACTION_READVAL] = {
        DEVICE_ACTION_READVAL,  // read any values that the sensors may have produced
        COLOR__MAGENTA,         // magenta while reading values
        0                       // no delay required after readval
    };
    device.deviceActions[DEVICE_ACTION_SETTING] = {
        DEVICE_ACTION_SETTING,  // apply any settings (wifi on, calibration, ...)
        COLOR_____GRAY,         // gray while active
        0                       // no delay required after readval
    };
    device.deviceActions[DEVICE_ACTION_DISPLAY] = {
        DEVICE_ACTION_DISPLAY,  // display refresh
        COLOR______RED,         // red while refreshing display
        3,                      // 3 seconds delay which is a rather long and conservative estimation (but the busy wakeup should take care of things at the best possible moment)
        secondstime             // initially set, so the entry screen will render right after start
    };
    device.actionIndexCur = DEVICE_ACTION_SETTING;  // start with DEVICE_ACTION_SETTING for entry screen (allows for NTP update before first measurements)
    device.actionIndexMax = DEVICE_ACTION_DISPLAY;

    return device;
}

void Device::begin(uint32_t secondstimeBoot) {
    Device::secondstimeBoot = secondstimeBoot;
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
    } else {
        return handleActionInvalid;
    }
}

device_action_e Device::handleActionInvalid(config_t& config, device_action_e maxDeviceAction) {
    // TODO :: should never be called, handle this condition
    return DEVICE_ACTION_MEASURE;
}

device_action_e Device::handleActionMeasure(config_t& config, device_action_e maxDeviceAction) {
    SensorPms003::measure();
    SensorBme280::measure();
    return DEVICE_ACTION_READVAL;  // read values after measuring
}

device_action_e Device::handleActionReadval(config_t& config, device_action_e maxDeviceAction) {

    // read values from the sensors
    values_pms_t measurementPms = SensorPms003::readval();
    values_bme_t measurementBme = SensorBme280::readval();

    uint32_t currSecondstime = SensorTime::getSecondstime();
    uint32_t currMeasureIndex = Values::values->nextMeasureIndex;            // not yet incremented
    uint32_t currStorageIndex = currMeasureIndex % MEASUREMENT_BUFFER_SIZE;  // the index of this measurement in the data
    uint32_t prevStorageIndex = currMeasureIndex > 0 ? (currMeasureIndex - 1) % MEASUREMENT_BUFFER_SIZE : 0;

    // TODO :: have a better time reference for when to step back (this way if continually increments the secondstime with the minute)
    uint32_t prevSecondstime = Values::values->measurements[prevStorageIndex].secondstime;
    if (currMeasureIndex > 0 && currSecondstime < prevSecondstime + SECONDS_PER___________MINUTE) {
        currSecondstime = prevSecondstime;  // need to use prev, or it would never not step back
        currMeasureIndex = currMeasureIndex - 1;
        currStorageIndex = currMeasureIndex % MEASUREMENT_BUFFER_SIZE;  // the index of this measurement in the data
        prevStorageIndex = currMeasureIndex > 0 ? (currMeasureIndex - 1) % MEASUREMENT_BUFFER_SIZE : 0;
    }
    uint32_t nextMeasureIndex = currMeasureIndex + 1;

    Values::values->measurements[currStorageIndex] = {
        currSecondstime,  // secondstime as of RTC
        measurementPms,   // sensorScd041 values
        measurementBme,   // sensorBme280 values
        true              // publishable
    };

    Values::values->nextMeasureIndex = nextMeasureIndex;

    return DEVICE_ACTION_SETTING;  // advance to settings, where another decision will be made whether to continue with display or not
}

device_action_e Device::handleActionSetting(config_t& config, device_action_e maxDeviceAction) {

    uint32_t currMeasureIndex = Values::values->nextMeasureIndex - 1;

    // turn on wifi, if required and adapt display modus
    bool isOrWasWifiPowered = ModuleWifi::isPowered();
    if (config.wifi.wifiValPower == WIFI____VAL_P__PND_Y && !ModuleWifi::isPowered()) {  // to be turned on, but currently off
        if (ModuleWifi::powerup(config, true)) {
            // do nothing (was renderQR)
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
            Serial.printf("autoNtpConn: %u, autoPubConn: %u, nextMeasureIndex: %u\n", Values::values->nextAutoNtpIndex, Values::values->nextAutoPubIndex, Values::values->nextMeasureIndex);
#endif
            autoDepower = ModuleWifi::powerup(config, false);  // if the connection was successful, it also needs to be autoShutoff
            if (autoDepower) {                                 // was connected

                if (autoPubConn) {
                    // try to publish (call with config, so mqtt gets the opportunity to ... (?))
                    ModuleMqtt::publish(config);
                    Values::values->nextAutoPubIndex = config.mqtt.mqttPublishMinutes == MQTT_PUBLISH___NEVER ? MQTT_PUBLISH___NEVER : currMeasureIndex + config.mqtt.mqttPublishMinutes;
                }
                if (autoNtpConn) {
                    SensorTime::setupNtpUpdate(config);                                                  // apply timezone
                    Values::values->nextAutoNtpIndex = currMeasureIndex + config.time.ntpUpdateMinutes;  // TODO :: add config, then choose either MQTT update interval or NTP update interval
                    for (int i = 0; i < 100; i++) {                                                      // wait 10 secs max for time sync
                        if (!SensorTime::isNtpWait()) {
                            break;
                        }
                        delay(100);
                    }
                }
                ModuleWifi::depower(config);
            } else {
                // connection failure, probably does not make sense to try every time (especially with the short PMS measurement intervals)
            }
        }
    }

    return DEVICE_ACTION_DISPLAY;  // advance to display
}

device_action_e Device::handleActionDisplay(config_t& config, device_action_e maxDeviceAction) {

    uint32_t currMeasureIndex = Values::values->nextMeasureIndex - 1;
    if (config.disp.displayValSetng == DISPLAY_VAL_S__ENTRY) {
        ModuleDisp::renderEntry(config);  // splash screen
    } else {
        values_all_t measurement = Values::values->measurements[(currMeasureIndex + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE];
        ModuleDisp::renderTable(measurement, config);
    }
    config.disp.displayValSetng = DISPLAY_VAL_S___NONE;

    return DEVICE_ACTION_MEASURE;  // display must be depowered after drawing
}
