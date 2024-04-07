#ifndef ModuleMqtt_h
#define ModuleMqtt_h

#include <Arduino.h>

#include "types/Config.h"


const String FILE_MQTT_CONFIG_JSON = "/config/mqtt.json";
const String FILE_MQTT_CONFIG__DAT = "/config/mqtt.dat";
const uint32_t MQTT_PUBLISH___NEVER = 0xFFFFFFFF;

typedef struct {
    uint8_t prt;
    uint8_t min;
    char srv[64];
    char usr[64];
    char pwd[64];
    char cli[16];
    char crt[16];
    char top[16];
} mqtt____t;

class ModuleMqtt {
   private:
   public:
    static void configure(config_t& config);  // loads json configuration, TODO :: is writing to dat a good idea here as well?
    static void publish(config_t& config);
};

#endif