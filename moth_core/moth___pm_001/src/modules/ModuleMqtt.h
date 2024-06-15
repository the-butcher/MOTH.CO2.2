#ifndef ModuleMqtt_h
#define ModuleMqtt_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <RTClib.h>

#include "types/Config.h"
#include "types/Values.h"

const String MQTT_CONFIG_JSON = "/config/mqtt.json";
const String MQTT_CONFIG__DAT = "/config/mqtt.dat";
const String MQTT_CONFIG__CRT = "/config/ca.crt";
const uint32_t MQTT_PUBLISH___NEVER = 0xFFFFFFFF;
const uint32_t MQTT_PUBLISH_RECOVER = 60;
const uint8_t MAX_PUB_COUNT = 20;
const uint8_t ERR_PUB_COUNT = 128;

typedef struct {
    bool use;
    bool hst;
    uint16_t prt;
    uint8_t min;
    char srv[64];
    char usr[64];
    char pwd[64];
    char cli[16];
} mqtt____t;

class ModuleMqtt {
   private:
   public:
    static void configure(config_t& config);  // loads json configuration, TODO :: is writing to dat a good idea here as well?
    static void createDat(config_t& config);
    static void publish(config_t& config);
    static bool publishMeasurement(config_t& config, values_all_t* value, char* client, PubSubClient* mqttClient);
    static mqtt____stat__e checkDatStat(mqtt____t& mqtt);
    static mqtt____stat__e checkCliStat(PubSubClient* mqttClient);
};

#endif