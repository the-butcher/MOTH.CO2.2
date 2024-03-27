#ifndef ModuleWifi_h
#define ModuleWifi_h

#include <WiFi.h>

#include "modules/ModuleServer.h"
#include "types/Config.h"
#include "types/Define.h"

const String JSON_KEY___MINUTES = "min";
const String JSON_KEY__NETWORKS = "ntw";
const String JSON_KEY_______KEY = "key";
const String JSON_KEY_______PWD = "pwd";

const String FILE_WIFI_CONFIG_JSON = "/config/wifi.json";
const String FILE_WIFI_CONFIG__DAT = "/config/wifi.dat";
const String WIFI_AP_KEY = "mothbox";
const String WIFI_AP_PWD = "CO2@420PPM";

typedef struct {
    char key[64];
    char pwd[64];
} network_t;

class ModuleWifi {
   private:
    static String networkName;
    static network_t configuredNetworks[10];
    static uint32_t secondstimeExpiry;
    static uint8_t expiryMinutes;

    static bool connectToNetwork(network_t* network);
    static bool enableSoftAP();

   public:
    static void begin();  // loads json configuration and creates a dat version for faster future accessibility
    static bool powerup(config_t* config, bool allowApMode);
    static bool isPowered();
    static void depower(config_t* config);
    static void access();
    static void expire();
    static uint32_t getSecondstimeExpiry();
    static String getRootUrl();
    static String getAddress();
    static String getNetworkName();
    static String getNetworkPass();
};

#endif