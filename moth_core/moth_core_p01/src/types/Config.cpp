#include "types/Config.h"

#include <Arduino.h>

// #include "modules/ModuleMqtt.h"
// #include "sensors/SensorTime.h"

config_t* Config::config = nullptr;

config_t Config::load() {

    config_t config = {
        {
            CONFIG_STAT__DEFAULT,  // flag indicating the status if the display config
            {5, 15},               // pms
            {
                14,  // deg rLo
                19,  // deg wLo
                25,  // deg wHi
                30,  // deg rHi

            },
            {
                25,  // hum rLo
                30,  // hum wLo
                60,  // hum wHi
                65   // hum rHi
            },
            DISPLAY_VAL_S__ENTRY,  // entry screen first
            DISPLAY_VAL_T____025,  // value shown when rendering in table mode
            DISPLAY_VAL_D______C   // temperature scale celsius | fahrenheit
        },
        {
            CONFIG_STAT__DEFAULT,  // flag indicating the status if the wifi config
            WIFI____VAL_P__CUR_N,  // initial wifi state off
            10,                    // network expiry minutes
            0                      // last successful connection index
        },
        {
            0,   // utc offset seconds
            360  // ntp update interval, 6 hours, CHECK_MEASURE_INTERVAL (likely assumes 1 minute RTC interval)
            // [timezone char array [64]]
        },
        {
            0.5  // low pass filter alpha
        },
        {
            10.0,
            0.5  // low pass filter alpha
        },
        {
            SIGNAL__VAL_____OFF  // warn signal
        },
        {
            CONFIG_STAT__DEFAULT,  // flag indicating the status if the mqtt config
            0xFFFFFFFF,            // mqtt publish minutes (TODO :: replace with constant in ModuleMqtt.h)
            0,                     // failure count
            MQTT_________UNKNOWN   // mqtt status
        },
    };
    String timezone = "CET-1CEST,M3.5.0,M10.5.0/3";  // initial timezone
    timezone.toCharArray(config.time.timezone, 64);
    return config;
}

void Config::begin(config_t* config) {
    Config::config = config;
}

mqtt____stat__e Config::getMqttStatus() {
    return Config::config->mqtt.mqttStatus;
}

void Config::setUtcOffsetSeconds(int32_t utcOffsetSeconds) {
    Config::config->time.utcOffsetSeconds = utcOffsetSeconds;
}

int32_t Config::getUtcOffsetSeconds() {
    return Config::config->time.utcOffsetSeconds;
}
