#include "ModuleWifi.h"

#include <ArduinoJson.h>
#include <SdFat.h>

#include "driver/adc.h"
#include "modules/ModuleSdcard.h"

String ModuleWifi::networkName = "";
uint32_t ModuleWifi::secondstimeExpiry = 0;
uint8_t ModuleWifi::expiryMinutes = 5;  // default

void ModuleWifi::begin() {
    ModuleSdcard::begin();
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


    }
}

bool ModuleWifi::powerup(config_t* config, bool allowApMode) {
    // adc_power_on();

    ModuleSdcard::begin();
    ModuleWifi::expiryMinutes = config->wifi.networkExpiryMinutes;

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

    bool powered = false;

    // connect to previous network, if defined
    if (config->wifi.networkConnIndexLast >= 0) {
        if (ModuleWifi::connectToNetwork(&configuredNetworks[config->wifi.networkConnIndexLast])) {
            powered = true;
        } else {
            config->wifi.networkConnIndexLast = -1;
        }
    }

    // if no connection could be made through the default network id -> find more networks
    if (!powered) {
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
                        powered = true;
                        break;
                    }
                }
            }
        }
    }

    // no connection through any of the configured networks, start in ap mode
    if (!powered && allowApMode) {
        ModuleWifi::depower(config);
        delay(500);
        powered = ModuleWifi::enableSoftAP();
    }

    if (powered) {
        ModuleServer::begin();
        ModuleWifi::access();
        config->wifi.powered = true;
    } else {
        ModuleWifi::depower(config);  // if no connection could be established
        ModuleWifi::expire();
        config->wifi.powered = false;
    }

    return powered;
}

bool ModuleWifi::isPowered() {
    wifi_mode_t wifiMode = WiFi.getMode();
    return wifiMode == WIFI_AP || (wifiMode == WIFI_STA && WiFi.isConnected());
}

bool ModuleWifi::connectToNetwork(network_t* network) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(network->key, network->pwd);
    for (int i = 0; i < 10; i++) {
        if (ModuleWifi::isPowered()) {
            char networkNameBuf[64];
            sprintf(networkNameBuf, "%s%s%s", "WIFI:T:WPA;S:", network->key, ";;;");
            ModuleWifi::networkName = String(networkNameBuf);
            return true;
        }
        delay(200);
    }
    return false;
}

bool ModuleWifi::enableSoftAP() {
    WiFi.mode(WIFI_AP);

    uint64_t _chipmacid = 0LL;
    esp_efuse_mac_get_default((uint8_t*)(&_chipmacid));

    // build a unique name for the ap, TODO :: is this correct
    char networkKeyBuf[32];
    sprintf(networkKeyBuf, "mothbox_%s", String(_chipmacid, HEX));

    char networkNameBuf[64];
    sprintf(networkNameBuf, "WIFI:T:WPA;S:mothbox_%s;P:CO2@420PPM;;", String(_chipmacid, HEX));
    ModuleWifi::networkName = String(networkNameBuf);

    return WiFi.softAP(networkKeyBuf, WIFI_AP_PWD.c_str());
}

void ModuleWifi::depower(config_t* config) {
    // adc_power_off();
    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    config->wifi.powered = false;
}

void ModuleWifi::access() {
    ModuleWifi::secondstimeExpiry = SensorTime::getSecondstime() + ModuleWifi::expiryMinutes * SECONDS_PER___________MINUTE;
}

void ModuleWifi::expire() {
    ModuleWifi::secondstimeExpiry = 0;
}

uint32_t ModuleWifi::getSecondstimeExpiry() {
    return ModuleWifi::secondstimeExpiry;
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

String ModuleWifi::getRootUrl() {
    wifi_mode_t wifiMode = WiFi.getMode();
    if (wifiMode == WIFI_AP) {
        return "http://" + WiFi.softAPIP().toString() + "/server/server.html";
    } else if (wifiMode == WIFI_STA) {
        return "http://" + WiFi.localIP().toString() + "/server/server.html";
    } else {
        return "";
    }
}

String ModuleWifi::getNetworkName() {
    wifi_mode_t wifiMode = WiFi.getMode();
    if (wifiMode == WIFI_AP || wifiMode == WIFI_STA) {
        return ModuleWifi::networkName;
    } else {
        return "";
    }
}

String ModuleWifi::getNetworkPass() {
    wifi_mode_t wifiMode = WiFi.getMode();
    if (wifiMode == WIFI_AP) {
        return WIFI_AP_PWD;
    } else {
        return "";
    }
}