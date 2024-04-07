#include "ModuleMqtt.h"

#include <ArduinoJson.h>
#include <SdFat.h>

#include "modules/ModuleSdcard.h"

void ModuleMqtt::configure(config_t& config) {
    ModuleSdcard::begin();
    File32 mqttFileDat;
    if (!ModuleSdcard::existsPath(FILE_MQTT_CONFIG__DAT)) {
        File32 mqttFileJson;
        bool jsonSuccess = mqttFileJson.open(FILE_MQTT_CONFIG_JSON.c_str(), O_RDONLY);
        if (jsonSuccess) {
            StaticJsonBuffer<512> jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(mqttFileJson);
            if (root.success()) {

                mqttFileDat.open(FILE_MQTT_CONFIG__DAT.c_str(), O_RDWR | O_CREAT | O_AT_END);  // the file has been checked to not exist

                String srv = root[JSON_KEY____SERVER] | "";
                uint8_t prt = root[JSON_KEY______PORT] | 0;
                String usr = root[JSON_KEY______USER] | "";
                String pwd = root[JSON_KEY_______PWD] | "";
                String cli = root[JSON_KEY____CLIENT] | "";
                String crt = root[JSON_KEY______CERT] | "";
                String top = root[JSON_KEY_____TOPIC] | "";
                uint8_t min = root[JSON_KEY___MINUTES] | 15;

                mqtt____t mqtt;
                srv.toCharArray(mqtt.srv, 64);
                mqtt.prt = prt;
                usr.toCharArray(mqtt.usr, 64);
                pwd.toCharArray(mqtt.pwd, 64);
                cli.toCharArray(mqtt.pwd, 16);
                crt.toCharArray(mqtt.crt, 16);
                top.toCharArray(mqtt.top, 16);
                mqtt.min = min;

                mqttFileDat.write((byte*)&mqtt, sizeof(mqtt));
                mqttFileDat.close();

                // TODO :: update according to configuration
                config.mqtt.mqttPublishMinutes = MQTT_PUBLISH___NEVER;
            } else {
                // TODO :: handle this condition (+ handle when dat file could not be opened)
            }
        } else {
            // TODO :: handle this condition
        }
    }
}

void ModuleMqtt::publish(config_t& config) {

    File32 mqttFileDat;
    bool datSuccess = mqttFileDat.open(FILE_MQTT_CONFIG__DAT.c_str(), O_RDONLY);
    if (datSuccess) {
        mqtt____t mqtt;
        if (mqttFileDat.available()) {
            mqttFileDat.read((byte*)&mqtt, sizeof(mqtt));

        } else {
            // TODO :: handle, apply appropriate status
        }
        // connect, publish
        // set publish minutes to configured value
    } else {
        // write error status to config
        // set publish minute to never
    }

    config.mqtt.mqttPublishMinutes = MQTT_PUBLISH___NEVER;  // no update
}