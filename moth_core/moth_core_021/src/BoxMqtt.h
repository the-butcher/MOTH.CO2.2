#ifndef BoxMqtt_h
#define BoxMqtt_h 

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "BoxStatus.h"

#define MQTT_STATUS___________________OK 10
#define MQTT_STATUS_FAILURE______PUBLISH 11
#define MQTT_STATUS_FAILURE___CONNECTION 12
#define MQTT_STATUS_TIMEOUT___CONNECTION 13
#define MQTT_STATUS_LOST______CONNECTION 14
#define MQTT_STATUS_UNAVAIL___CONNECTION 15
#define MQTT_STATUS_INVALID_____PROTOCOL 16
#define MQTT_STATUS_INVALID_____CLIENTID 17
#define MQTT_STATUS_INVALID__CREDENTIALS 18
#define MQTT_STATUS_INVALID_UNAUTHORIZED 19
#define MQTT_STATUS_INVALID____WIFI_MODE 20
#define MQTT_STATUS_INVALID__WIFI_STATUS 21
#define MQTT_STATUS_INVALID_________ADDR 22
#define MQTT_STATUS_INVALID_________PORT 23
#define MQTT_STATUS______________UNKNOWN 24

class BoxMqtt {
  
  private:
    static int failureCount;
    static bool publishJson(JsonObject &root, char* mqid);
    static void checkClients();
    static bool checkConnect();
    static void callback(char* topic, byte* payload, unsigned int length);
    
  public:
    static int publishIntervalMinutes;
    static bool isWifiConnectionRequested;
    static String CONFIG_PATH;
    static config_status_t configStatus;
    static int status;
    static void begin();
    static bool loop();
    static void updateConfiguration();
    static bool isConfiguredToBeActive();
    static bool isPublishable();
    static void publish();
    static bool hasFailure();

};

#endif