#ifndef ModuleWifi_h
#define ModuleWifi_h

#include <WiFi.h>

#include "types/Config.h"
#include "types/Define.h"

const String JSON_KEY___MINUTES = "min";
const String JSON_KEY__NETWORKS = "ntw";
const String JSON_KEY_______KEY = "key";
const String JSON_KEY_______PWD = "pwd";

typedef struct {
    char key[64];
    char pwd[64];
} network_t;

class ModuleWifi {
   private:
    static String apNetworkConn;
    static network_t configuredNetworks[10];
    static bool connectToNetwork(network_t* network);
    static bool enableSoftAP();

   public:
    static void begin();  // loads json configuration and creates a dat version for faster future accessibility
    static bool connect(config_t* config);
    static bool isConnected();
    static void shutoff();
    static String getAddress();
};

#endif