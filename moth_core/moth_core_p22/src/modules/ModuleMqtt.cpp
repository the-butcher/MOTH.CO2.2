#include "ModuleMqtt.h"

#include <ArduinoJson.h>
#include <SdFat.h>
#include <WiFiClientSecure.h>

#include "modules/ModuleSdcard.h"
#include "types/Define.h"

void ModuleMqtt::configure(config_t& config) {
    ModuleSdcard::begin();
    if (!ModuleSdcard::existsPath(MQTT_CONFIG__DAT)) {
        ModuleMqtt::createDat(config);
    }
}

void ModuleMqtt::createDat(config_t& config) {
#ifdef USE___SERIAL
    Serial.println("createDat ...");
#endif
    File32 mqttFileJson;
    bool jsonSuccess = mqttFileJson.open(MQTT_CONFIG_JSON.c_str(), O_RDONLY);
    if (jsonSuccess) {
        StaticJsonBuffer<512> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(mqttFileJson);
        if (root.success()) {

#ifdef USE___SERIAL
            Serial.println("got json config ...");
#endif

            File32 mqttFileDat;
            mqttFileDat.open(MQTT_CONFIG__DAT.c_str(), O_RDWR | O_CREAT | O_AT_END);  // the file has been checked to not exist

            String srv = root[JSON_KEY____SERVER] | "";
            uint16_t prt = root[JSON_KEY______PORT] | 0;
            String usr = root[JSON_KEY______USER] | "";
            String pwd = root[JSON_KEY_______PWD] | "";
            String cli = root[JSON_KEY____CLIENT] | "";
            String crt = root[JSON_KEY______CERT] | "";
            String top = root[JSON_KEY_____TOPIC] | "";
            uint8_t min = root[JSON_KEY___MINUTES] | 15;

            mqtt____t mqtt;
            mqtt.prt = prt;
            mqtt.min = min;
            srv.toCharArray(mqtt.srv, 64);
            usr.toCharArray(mqtt.usr, 64);
            pwd.toCharArray(mqtt.pwd, 64);
            cli.toCharArray(mqtt.cli, 16);
            crt.toCharArray(mqtt.crt, 16);
            top.toCharArray(mqtt.top, 16);

            mqttFileDat.write((byte*)&mqtt, sizeof(mqtt));
            mqttFileDat.close();

        } else {
            // TODO :: handle this condition (+ handle when dat file could not be opened)
        }
    } else {
        // TODO :: handle this condition
    }
}

mqtt____stat__e ModuleMqtt::checkDatStat(mqtt____t& mqtt) {

    if (mqtt.srv == "") {
        return MQTT_CNF_________SRV;
    } else if (mqtt.cli == "") {
        return MQTT_CNF_________CLI;
    } else if (mqtt.prt == 0) {
#ifdef USE___SERIAL
        Serial.printf("mqtt, prt: %d\n", mqtt.prt);
#endif
        return MQTT_CNF_________PRT;
    } else {
        return MQTT______________OK;
    }
}

mqtt____stat__e ModuleMqtt::checkCliStat(PubSubClient* mqttClient) {
    int state = mqttClient->state();
    if (state == MQTT_CONNECTION_TIMEOUT) {  // -4
        return MQTT_TIMEOUT____CONN;
    } else if (state == MQTT_CONNECTION_LOST) {  // -3
        return MQTT_LOST_______CONN;
    } else if (state == MQTT_CONNECT_FAILED) {  // -2
        return MQTT_FAIL_______CONN;
    } else if (state == MQTT_DISCONNECTED) {  // -1
        return MQTT_LOST_______CONN;
    } else if (state == MQTT_CONNECTED) {  // 0 :: OK
        return MQTT______________OK;
    } else if (state == MQTT_CONNECT_BAD_PROTOCOL) {  // 1
        return MQTT_BAD____PROTOCOL;
    } else if (state == MQTT_CONNECT_BAD_CLIENT_ID) {  // 2
        return MQTT_BAD_________CLI;
    } else if (state == MQTT_CONNECT_UNAVAILABLE) {  // 3
        return MQTT_UNAVAIL____CONN;
    } else if (state == MQTT_CONNECT_BAD_CREDENTIALS) {  // 4
        return MQTT_BAD_CREDENTIALS;
    } else if (state == MQTT_CONNECT_UNAUTHORIZED) {  // 5
        return MQTT_NO_________AUTH;
    } else {
        return MQTT_________UNKNOWN;
    }
}

void ModuleMqtt::publish(config_t& config) {

    if (!ModuleSdcard::existsPath(MQTT_CONFIG__DAT)) {
        ModuleMqtt::createDat(config);
    }

    File32 mqttFileDat;
    bool datSuccess = mqttFileDat.open(MQTT_CONFIG__DAT.c_str(), O_RDONLY);
    if (datSuccess) {
        mqtt____t mqtt;
        if (mqttFileDat.available()) {
            mqttFileDat.read((byte*)&mqtt, sizeof(mqtt));
            mqtt____stat__e datStat = ModuleMqtt::checkDatStat(mqtt);
            if (datStat == MQTT______________OK) {  // a set of config worth trying

                WiFiClient* wifiClient;
                if (mqtt.crt != "" && ModuleSdcard::existsPath(mqtt.crt)) {
#ifdef USE___SERIAL
                    Serial.println("creating secure wifi client ...");
#endif
                    wifiClient = new WiFiClientSecure();
                    File32 certFile;
                    certFile.open(mqtt.crt, O_RDONLY);
                    ((WiFiClientSecure*)wifiClient)->loadCACert(certFile, certFile.size());
                    certFile.close();
                } else {
#ifdef USE___SERIAL
                    Serial.println("creating nonsecure wifi client ...");
#endif
                    wifiClient = new WiFiClient();
                }
                PubSubClient* mqttClient;
                mqttClient = new PubSubClient(*wifiClient);
                mqttClient->setServer(mqtt.srv, mqtt.prt);
                if (mqtt.usr != "" && mqtt.pwd != "") {
                    mqttClient->connect(mqtt.cli, mqtt.usr, mqtt.pwd, 0, 0, 0, 0, 0);
                } else {
                    mqttClient->connect(mqtt.cli);  // connect without credentials
                }
                if (mqttClient->connected()) {
                    config.mqtt.mqttStatus = MQTT______________OK;
#ifdef USE___SERIAL
                    Serial.println("connected!");
#endif
                    // TODO :: find a way to know then the lass publication time was, open and read all files from there and publish
                    // finally publish anything currently in measurement buffer
                    // publishable cant be older than mqtt config time (boot time?)

                } else {
                    config.mqtt.mqttStatus = ModuleMqtt::checkCliStat(mqttClient);
                }

            } else {  // datStat other than MQTT______________OK
                config.mqtt.mqttStatus = datStat;
            }

        } else {  // no data available in dat
            config.mqtt.mqttStatus = MQTT_NO__________DAT;
        }

    } else {  // could not open dat
        config.mqtt.mqttStatus = MQTT_FAIL________DAT;
    }

#ifdef USE___SERIAL
    Serial.printf("before returning infinite publish interval, stat: %d\n", config.mqtt.mqttStatus);
#endif
    config.mqtt.mqttPublishMinutes = MQTT_PUBLISH___NEVER;  // no update
}