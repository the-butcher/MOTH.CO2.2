#include "ModuleMqtt.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <SdFat.h>
#include <WiFiClientSecure.h>

#include "modules/ModuleSdcard.h"
#include "types/Define.h"

void ModuleMqtt::configure(config_t& config) {
    ModuleSdcard::begin();
    File32 mqttFileDat;
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

void ModuleMqtt::publish(config_t& config) {

    if (!ModuleSdcard::existsPath(MQTT_CONFIG__DAT)) {
#ifdef USE___SERIAL
        Serial.println("creating mqtt dat ...");
#endif
        ModuleMqtt::createDat(config);
    }

    File32 mqttFileDat;
#ifdef USE___SERIAL
    Serial.println("opening mqtt dat ...");
#endif
    bool datSuccess = mqttFileDat.open(MQTT_CONFIG__DAT.c_str(), O_RDONLY);
    if (datSuccess) {
#ifdef USE___SERIAL
        Serial.println("reading mqtt dat ...");
#endif
        mqtt____t mqtt;
        if (mqttFileDat.available()) {
            mqttFileDat.read((byte*)&mqtt, sizeof(mqtt));
#ifdef USE___SERIAL
            Serial.println("done reading mqtt dat ...");
#endif
            if (mqtt.srv != "" && mqtt.prt > 0) {

                WiFiClient* wifiClient;
                if (mqtt.crt != "" && ModuleSdcard::existsPath(mqtt.crt)) {
#ifdef USE___SERIAL
                    Serial.println("srv and prt configured, creating secure wifi client ...");
#endif
                    wifiClient = new WiFiClientSecure();
                    File32 certFile;
                    certFile.open(mqtt.crt, O_RDONLY);
                    ((WiFiClientSecure*)wifiClient)->loadCACert(certFile, certFile.size());
                    certFile.close();
                } else {
#ifdef USE___SERIAL
                    Serial.println("srv and prt configured, creating nonsecure wifi client ...");
#endif
                    wifiClient = new WiFiClient();
                }
#ifdef USE___SERIAL
                Serial.printf("creating pubsub client, srv: %s, prt: %d ...\n", mqtt.srv, mqtt.prt);
#endif
                PubSubClient* mqttClient;
                mqttClient = new PubSubClient(*wifiClient);
                mqttClient->setServer(mqtt.srv, mqtt.prt);

#ifdef USE___SERIAL
                Serial.println("connecting pubsub client ...");
#endif
                if (mqtt.usr != "" && mqtt.pwd != "") {
                    // mqttClient->connect(mqttClid.c_str(), mqttUser.c_str(), mqttPass.c_str()); // connect with credentials
                    mqttClient->connect(mqtt.cli, mqtt.usr, mqtt.pwd, 0, 0, 0, 0, 0);
                } else {
                    mqttClient->connect(mqtt.cli);  // connect without credentials
                }
#ifdef USE___SERIAL
                Serial.println("checking pubsub client ...");
#endif

                // mqttClient->setCallback(BoxMqtt::callback);
                if (mqttClient->connected()) {  // must be connected to subscribe
                    // TODO :: subscribe to anything that may be needed, think of a pattern that may substitute for other wifi operation on a low power level
                } else {
#ifdef USE___SERIAL
                    Serial.println("failed connecting pubsub client, apply infinite publish time ...");
#endif
                    config.mqtt.mqttPublishMinutes = MQTT_PUBLISH___NEVER;
                }

            } else {
#ifdef USE___SERIAL
                Serial.println("srv and prt not configured, apply infinite publish time ...");
#endif
                config.mqtt.mqttPublishMinutes = MQTT_PUBLISH___NEVER;
            }

        } else {
            // TODO :: handle, apply appropriate status
        }
        // connect, publish
        // set publish minutes to configured value
    } else {
        // write error status to config
        // set publish minute to never
    }
#ifdef USE___SERIAL
    Serial.println("before returning infinite publish interval ...");
#endif
    config.mqtt.mqttPublishMinutes = MQTT_PUBLISH___NEVER;  // no update
}