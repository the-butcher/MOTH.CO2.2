#include "ModuleWifi.h"

#include <ArduinoJson.h>
#include <SdFat.h>

#include "driver/adc.h"
#include "modules/ModuleSdcard.h"

const String FILE_WIFI_CONFIG_JSON = "/config/wifi.json";
const String FILE_WIFI_CONFIG__DAT = "/config/wifi.dat";
const String WIFI_AP_KEY = "mothbox";
const String WIFI_AP_PWD = "CO2@420PPM";

String ModuleWifi::apNetworkConn = "";

void ModuleWifi::begin() {
    File32 wifiFileDat;
    if (!ModuleSdcard::existsPath(FILE_WIFI_CONFIG__DAT)) {
        File32 wifiFileJson;
        bool jsonSuccess = wifiFileJson.open(FILE_WIFI_CONFIG_JSON.c_str(), O_RDONLY);
        if (jsonSuccess) {
            StaticJsonBuffer<512> jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(wifiFileJson);
            if (root.success()) {
                wifiFileDat.open(FILE_WIFI_CONFIG__DAT.c_str(), O_RDWR | O_CREAT | O_AT_END);  // the file has been checked to not exist
                int jsonNetworkCount = root[JSON_KEY__NETWORKS].as<JsonArray>().size();
                String key;
                String pwd;
                network_t network;
                for (uint8_t jsonNetworkIndex = 0; jsonNetworkIndex < jsonNetworkCount; jsonNetworkIndex++) {
                    key = root[JSON_KEY__NETWORKS][jsonNetworkIndex][JSON_KEY_______KEY] | "";
                    pwd = root[JSON_KEY__NETWORKS][jsonNetworkIndex][JSON_KEY_______PWD] | "";
                    network = {};
                    key.toCharArray(network.key, 64);
                    pwd.toCharArray(network.pwd, 64);
                    wifiFileDat.write((byte*)&network, sizeof(network));
                }
                if (wifiFileDat) {
                    wifiFileDat.sync();
                    wifiFileDat.close();
                }

            } else {
                // TODO :: handle this condition
            }
        } else {
            // TODO :: handle this condition
        }

        //         bool fileSuccess = wifiFile.open(ModuleWifi::CONFIG_JSON.c_str(), O_RDONLY);
        //         if (fileSuccess) {
        //             uint16_t pdate;
        //             uint16_t ptime;
        //             wifiFile.getModifyDateTime(&pdate, &ptime);
        // #ifdef USE___SERIAL
        //             Serial.printf("LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", FS_YEAR(pdate), FS_MONTH(pdate), FS_DAY(pdate), FS_HOUR(ptime), FS_MINUTE(ptime), FS_SECOND(ptime));
        // #endif
        //         }
        //         if (wifiFile) {
        //             wifiFile.close();
        //         }
    }
}

bool ModuleWifi::connect(config_t* config) {
    adc_power_on();

    network_t configuredNetworks[10];
    uint8_t configuredNetworkCount = 0;
    // access the dat-file whether it existed or was just created
    File32 wifiFileDat;
    bool datSuccess = wifiFileDat.open(FILE_WIFI_CONFIG__DAT.c_str(), O_RDONLY);
    if (datSuccess) {
        network_t readValue;
        while (wifiFileDat.available()) {
            wifiFileDat.read((byte*)&readValue, sizeof(readValue));    // read the next value into readValue
            configuredNetworks[configuredNetworkCount++] = readValue;  // TODO put the network into an array of configured networks
        }
    } else {
        // TODO :: handle this condition
    }
    if (wifiFileDat) {
        wifiFileDat.close();
    }

    // connect to previous network, if defined
    if (config->wifi.networkConnIndexLast >= 0) {
        if (ModuleWifi::connectToNetwork(&configuredNetworks[config->wifi.networkConnIndexLast])) {
            return true;
        } else {
            config->wifi.networkConnIndexLast = -1;
        }
    }

    // find more networks (expensive)
    int ssidCount = WiFi.scanNetworks();

    String scanKey;
    String confKey;

    // iterate available networks
    for (int ssidIndex = 0; ssidIndex < ssidCount; ssidIndex++) {
        scanKey = WiFi.SSID(ssidIndex);
        for (int i = 0; i < configuredNetworkCount; i++) {
            confKey = String(configuredNetworks[i].key);
            if (i != config->wifi.networkConnIndexLast && confKey == scanKey) {  // found a configured network that matched one of the scanned networks
                if (ModuleWifi::connectToNetwork(&configuredNetworks[i])) {
                    config->wifi.networkConnIndexLast = i;
                    return true;
                }
            }
        }
    }

    ModuleWifi::shutoff();
    delay(500);

    if (ModuleWifi::enableSoftAP()) {
        return true;
    }

    ModuleWifi::shutoff();  // if no connection could be established
    return false;

    // TODO :: set wifi expiration seconds
}

bool ModuleWifi::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool ModuleWifi::connectToNetwork(network_t* network) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(network->key, network->pwd);
    for (int i = 0; i < 10; i++) {
        if (ModuleWifi::isConnected()) {
            return true;
        }
        delay(200);
    }
    return false;
}

bool ModuleWifi::enableSoftAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_KEY.c_str(), WIFI_AP_PWD.c_str());
    return true;
}

void ModuleWifi::shutoff() {
    adc_power_off();
    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

String ModuleWifi::getAddress() {
    wifi_mode_t wifiMode = WiFi.getMode();
    if (wifiMode == WIFI_STA) {
        return WiFi.localIP().toString();
    } else if (wifiMode == WIFI_AP) {
        return WiFi.softAPIP().toString();
    } else if (wifiMode == WIFI_OFF) {
        return "wifi off";
    } else {
        return "wifi unknown";
    }
}