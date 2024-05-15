#include "ModuleWifi.h"

#include <ArduinoJson.h>
#include <SdFat.h>

#include "driver/adc.h"
#include "modules/ModuleCard.h"
#include "sensors/SensorTime.h"

String ModuleWifi::networkName = "";
uint32_t ModuleWifi::secondstimeExpiry = 0;
uint8_t ModuleWifi::expiryMinutes = 5;  // default
network_t ModuleWifi::discoveredNetworks[NETWORKS_BUFFER_SIZE];

void ModuleWifi::configure(config_t& config) {
    ModuleCard::begin();
    // be sure the dat file is recreated from most current config
    ModuleCard::removeFile32(WIFI_CONFIG__DAT);
    ModuleWifi::createDat(config);
}

void ModuleWifi::createDat(config_t& config) {

    // ModuleSignal::beep(523);
    // delay(100);
    // ModuleSignal::beep(659);
    // delay(100);

    File32 wifiFileJson;
    bool jsonSuccess = wifiFileJson.open(WIFI_CONFIG_JSON.c_str(), O_RDONLY);
    if (jsonSuccess) {
        StaticJsonBuffer<512> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(wifiFileJson);
        if (root.success()) {

            config.wifi.configStatus = CONFIG_STAT__APPLIED;

            File32 wifiFileDat;
            bool datSuccess = wifiFileDat.open(WIFI_CONFIG__DAT.c_str(), O_RDWR | O_CREAT | O_AT_END);  // the file has been checked to not exist
            if (datSuccess) {

                int jsonNetworkCount = root[JSON_KEY__NETWORKS].as<JsonArray>().size();

                // problem: runs only when the dat file loads first
                config.wifi.networkExpiryMinutes = root[JSON_KEY___MINUTES] | 5;  // apply network expiry (independant from networks)

                String key;
                String pwd;
                network_t network;
                for (uint8_t jsonNetworkIndex = 0; jsonNetworkIndex < jsonNetworkCount; jsonNetworkIndex++) {
                    key = root[JSON_KEY__NETWORKS][jsonNetworkIndex][JSON_KEY_______KEY] | "";
                    pwd = root[JSON_KEY__NETWORKS][jsonNetworkIndex][JSON_KEY_______PWD] | "";
                    network = {0};
                    key.toCharArray(network.key, 64);
                    pwd.toCharArray(network.pwd, 64);
                    wifiFileDat.write((byte*)&network, sizeof(network));
                }
                wifiFileDat.close();
            } else {
                // TODO :: handle dat.open() failure
            }

        } else {
            // TODO :: handle root not createable error
        }
        wifiFileJson.close();
    }
}

bool ModuleWifi::powerup(config_t& config, bool allowApMode) {

    ModuleCard::begin();

    // recreate, when necessary
    if (!ModuleCard::existsPath(WIFI_CONFIG__DAT)) {
        ModuleWifi::createDat(config);
    }

    ModuleWifi::expiryMinutes = config.wifi.networkExpiryMinutes;  // make expiry minutes available in ModuleWifi

    network_t configuredNetworks[NETWORKS_BUFFER_SIZE];
    uint8_t configuredNetworkCount = 0;
    // access the dat-file whether it existed or was just created
    File32 wifiFileDat;
    bool datSuccess = wifiFileDat.open(WIFI_CONFIG__DAT.c_str(), O_RDONLY);
    if (datSuccess) {
        network_t readValue;
        while (wifiFileDat.available()) {
            wifiFileDat.read((byte*)&readValue, sizeof(readValue));    // read the next value into readValue
            configuredNetworks[configuredNetworkCount++] = readValue;  // TODO put the network into an array of configured networks
        }
        wifiFileDat.close();
    } else {
        // TODO :: handle this condition
    }

    bool powerupSuccess = false;

    // in case of auto connect, try to do a fast connect to the last network
    if (!allowApMode && config.wifi.networkConnIndexLast >= 0) {
        if (ModuleWifi::connectToNetwork(config, configuredNetworks[config.wifi.networkConnIndexLast])) {
            powerupSuccess = true;
        } else {
            config.wifi.networkConnIndexLast = -1;
        }
    }

    // either manual connect or failed auto connect
    if (!powerupSuccess) {

        int ssidCount = WiFi.scanNetworks();
        int ssidIndex = 0;
        for (; ssidIndex < min(ssidCount, (int)NETWORKS_BUFFER_SIZE); ssidIndex++) {
#ifdef USE___SERIAL
            Serial.printf("ssidIndex: %d\n", ssidIndex);
#endif
            String ssid = WiFi.SSID(ssidIndex);
            int32_t rssi = WiFi.RSSI(ssidIndex);
            ModuleWifi::discoveredNetworks[ssidIndex] = {rssi};
            ssid.toCharArray(ModuleWifi::discoveredNetworks[ssidIndex].key, 64);
        }
        for (; ssidIndex < NETWORKS_BUFFER_SIZE; ssidIndex++) {
            ModuleWifi::discoveredNetworks[ssidIndex] = {NETWORK_RSSI_INVALID};
        }

        // Ssort ModuleWifi::discoveredNetworks by network strength
        qsort(ModuleWifi::discoveredNetworks, NETWORKS_BUFFER_SIZE, sizeof(network_t), ModuleWifi::cmpfunc);

        network_t network;
        String scanKey;
        String confKey;
        for (int networkIndex = 0; networkIndex < NETWORKS_BUFFER_SIZE; networkIndex++) {
            network = ModuleWifi::discoveredNetworks[networkIndex];
            if (network.rssi > NETWORK_RSSI_INVALID) {
#ifdef USE___SERIAL
                Serial.printf("network: %s, %d\n", String(network.key), network.rssi);
#endif
                scanKey = String(network.key);
                for (int i = 0; i < configuredNetworkCount; i++) {
                    confKey = String(configuredNetworks[i].key);
                    if (confKey == scanKey) {  // found a configured network that matched one of the scanned networks
#ifdef USE___SERIAL
                        Serial.printf("connecting ...\n");
#endif
                        if (ModuleWifi::connectToNetwork(config, configuredNetworks[i])) {
#ifdef USE___SERIAL
                            Serial.printf("connected\n");
#endif
                            config.wifi.networkConnIndexLast = i;
                            powerupSuccess = true;
                            break;  // break the configured network loop
                        }
                    }
                }
                if (powerupSuccess) {
                    break;  // break the discovered network loop
                }
            }
        }
    }

    // no connection through any of the configured networks, start in ap mode
    if (!powerupSuccess && allowApMode) {
        ModuleWifi::depower(config);  // sets valPower to WIFI____VAL_P__CUR_N
        delay(500);
        powerupSuccess = ModuleWifi::enableSoftAP(config);
    }

    if (powerupSuccess) {
        ModuleHttp::begin();
        ModuleWifi::access();
    } else {
        ModuleWifi::depower(config);  // if no connection could be established, set valPower to WIFI____VAL_P__CUR_N
        ModuleWifi::expire();
    }

    return powerupSuccess;
}

int ModuleWifi::cmpfunc(const void* a, const void* b) {
    return (((network_t*)b)->rssi - ((network_t*)a)->rssi);
}

bool ModuleWifi::isPowered() {
    wifi_mode_t wifiMode = WiFi.getMode();
    return wifiMode == WIFI_AP || WiFi.isConnected();  // WiFi.isConnected() includes both WIFI_MODE_STA and WIFI_MODE_APSTA
}

bool ModuleWifi::connectToNetwork(config_t& config, network_t& network) {

    WiFi.mode(WIFI_STA);
    WiFi.begin(network.key, network.pwd);
    for (int i = 0; i < 10; i++) {
        delay(200);
        if (ModuleWifi::isPowered()) {
            char networkNameBuf[64];
            sprintf(networkNameBuf, "%s%s%s", "WIFI:T:WPA;S:", network.key, ";;;");
            ModuleWifi::networkName = String(networkNameBuf);
            config.wifi.wifiValPower = WIFI____VAL_P__CUR_Y;  // set flag to current on
            return true;
        }
    }

    ModuleWifi::depower(config);  // be sure the correct mode is set
    return false;
}

bool ModuleWifi::enableSoftAP(config_t& config) {

    WiFi.mode(WIFI_AP);

    uint64_t _chipmacid = 0LL;
    esp_efuse_mac_get_default((uint8_t*)(&_chipmacid));

    // build a unique name for the ap, TODO :: is this correct
    char networkKeyBuf[32];
    sprintf(networkKeyBuf, "mothbox_%s", String(_chipmacid, HEX));

    char networkNameBuf[64];
    sprintf(networkNameBuf, "WIFI:T:WPA;S:mothbox_%s;P:CO2@420PPM;;", String(_chipmacid, HEX));
    ModuleWifi::networkName = String(networkNameBuf);

    WiFi.softAP(networkKeyBuf, WIFI_AP_PWD.c_str());

    for (int i = 0; i < 10; i++) {
        delay(200);
        if (ModuleWifi::isPowered()) {
            config.wifi.wifiValPower = WIFI____VAL_P__CUR_Y;  // set flag to current on
            return true;
        }
    }

    ModuleWifi::depower(config);  // be sure the correct mode is set
    return false;
}

void ModuleWifi::depower(config_t& config) {
    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);
    for (int i = 0; i < 10; i++) {
        delay(200);
        if (!ModuleWifi::isPowered()) {
            break;
        }
    }
    WiFi.mode(WIFI_OFF);
    config.wifi.wifiValPower = WIFI____VAL_P__CUR_N;  // set flag to currently off
}

/**
 * awkward code for incrementing a single variable
 * there were problems, when the main code would not see the correct expiry time while the async web requests were updating
 */
void ModuleWifi::access() {
    ModuleWifi::secondstimeExpiry = SensorTime::getSecondstime() + ModuleWifi::expiryMinutes * SECONDS_PER___________MINUTE;
}

void ModuleWifi::expire() {
    ModuleWifi::secondstimeExpiry = SensorTime::getSecondstime() + 1;
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
        return String(wifiMode);
    }
}

String ModuleWifi::getRootUrl() {
    wifi_mode_t wifiMode = WiFi.getMode();
    if (wifiMode == WIFI_AP) {
        return "http://" + WiFi.softAPIP().toString() + "/server/root.html";
    } else if (wifiMode == WIFI_STA) {
        return "http://" + WiFi.localIP().toString() + "/server/root.html";
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