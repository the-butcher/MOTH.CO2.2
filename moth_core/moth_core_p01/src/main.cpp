#include <Arduino.h>
#include <Wire.h>
#include <driver/rtc_io.h>
#include <esp_wifi.h>

#include "buttons/ButtonAction.h"
#include "modules/ModuleDisp.h"
#include "modules/ModuleHttp.h"
#include "modules/ModuleMqtt.h"
#include "modules/ModuleSignal.h"
#include "modules/ModuleWifi.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorPms003.h"
#include "sensors/SensorTime.h"
#include "types/Config.h"
#include "types/Define.h"
#include "types/Device.h"
#include "types/Values.h"

config_t config;
values_t values;
device_t device;
uint32_t secondstimeBoot;
uint16_t actionNum = 0;

/**
 * TODO :: device              :: prerequisite for WiFi config
 * TODO :: complete ModuleWifi :: prerequisite for SensorTime
 * TODO :: complete SensorTime :: especially NTP update
 */

// schedule setting and display
void scheduleDeviceActionSetting() {
    device.actionIndexMax = DEVICE_ACTION_DISPLAY;  // allow actions display and depower by index
    uint32_t secondstime = SensorTime::getSecondstime();
    uint32_t secondswait = 60 - secondstime % 60;  // CHECK_MEASURE_INTERVAL
    device.actionIndexCur = DEVICE_ACTION_SETTING;
    device.deviceActions[DEVICE_ACTION_SETTING].secondsNext = SensorTime::getSecondstime();  // assign current time as due time
}

/**
 * handle a detected button action or a wifi event
 */
void handleActionComplete(std::function<bool(config_t& config)> actionFunction) {
    if (actionFunction != nullptr) {
        bool actionResult = actionFunction(config);  // execute the action (which, by convention, only alters the config to not interfere with program flow)
        if (actionResult) {
            ModuleSignal::beep();           // indicate complete
            scheduleDeviceActionSetting();  // schedule DEVICE_ACTION_SETTING and DEVICE_ACTION_DISPLAY
        }
    }
    actionNum++;  // interrupt delay loop
}

void setup() {

    Serial.begin(115200);
    delay(3000);
    Wire.begin();

    // must have begun to set color
    ModuleSignal::begin();
    ModuleSignal::setPixelColor(COLOR___YELLOW);

    // must have begun before calling setup
    SensorTime::begin();

    // must have begun to be configured
    SensorPms003::begin();
    SensorBme280::begin();

    config = Config::load();
    values = Values::load();
    device = Device::load();

    ModuleMqtt::configure(config);
    ModuleDisp::configure(config);
    SensorBme280::configure(config);

    Config::begin(&config);
    Values::begin(&values);
    Device::begin(secondstimeBoot);

    ButtonAction::begin(handleActionComplete);
    ButtonAction::adapt(config);

    ModuleDisp::begin();
    delay(4000);
}

String getActionName(device_action_e action) {
    if (action == DEVICE_ACTION_MEASURE) {
        return "measure";
    } else if (action == DEVICE_ACTION_READVAL) {
        return "readval";
    } else if (action == DEVICE_ACTION_SETTING) {
        return "setting";
    } else if (action == DEVICE_ACTION_DISPLAY) {
        return "display";
    } else {
        return "unknown";
    }
}

void secondsDelay(uint32_t seconds) {

    uint32_t millisEntry = millis();
    uint32_t millisBreak = millisEntry + seconds * MILLISECONDS_PER______SECOND;
    uint16_t actionNumEntry = actionNum;

    ButtonAction::attachWakeup();  // button interrupts

    ModuleSignal::setPixelColor(COLOR_____CYAN);
    while (millis() < millisBreak && actionNumEntry == actionNum) {

        delay(50);

        // wifi expiry
        uint32_t secondsdest = ModuleWifi::getSecondstimeExpiry();
        uint32_t secondstime = SensorTime::getSecondstime();
        uint32_t secondswait = secondsdest > secondstime ? secondsdest - secondstime : WAITTIME________________NONE;
        if (secondswait == WAITTIME________________NONE && config.wifi.wifiValPower == WIFI____VAL_P__CUR_Y) {
            ModuleSignal::beep();
            config.wifi.wifiValPower = WIFI____VAL_P__PND_N;  // set flag to pending off
            scheduleDeviceActionSetting();
            break;
        }

        // anything ModuleHttp wants to configure (display aspects, co2 calibration, co2 reset, co2 power mode)
        if (ModuleHttp::requestedReconfiguration != nullptr) {
            ModuleSignal::beep();
            ModuleHttp::requestedReconfiguration(config, values);
            ButtonAction::adapt(config);  // be sure buttons reflect potentially altered configuration
            scheduleDeviceActionSetting();
            ModuleHttp::requestedReconfiguration = nullptr;
            break;
        }
    }

    ButtonAction::detachWakeup();
}

device_action_e getActionIndexMax() {
    return DEVICE_ACTION_DISPLAY;
}

void loop() {
    device_action_t action = device.deviceActions[device.actionIndexCur];
    if (SensorTime::getSecondsUntil(action.secondsNext) == WAITTIME________________NONE) {  // action is due
        ModuleSignal::setPixelColor(action.color);
        // Serial.printf("device.actionIndexCur (a): %s\n", getActionName(device.actionIndexCur));
        device.actionIndexCur = Device::getFunctionByAction(action.type)(config, device.actionIndexMax);  // execute the action and see whats coming next
        // Serial.printf("device.actionIndexCur (b): %s\n", getActionName(device.actionIndexCur));
        device.deviceActions[device.actionIndexCur].secondsNext = SensorTime::getSecondstime() + action.secondsWait;
    }
    uint32_t secondsWait = SensorTime::getSecondsUntil(device.deviceActions[device.actionIndexCur].secondsNext);
    // Serial.printf("secondsWait: %u\n", secondsWait);
    if (secondsWait > WAITTIME________________NONE) {
        secondsDelay(secondsWait);
    }
}