#include "ModuleMqtt.h"

#include <WiFiClientSecure.h>

#include "modules/ModuleCard.h"
#include "modules/ModuleSignal.h"
#include "types/Define.h"

void ModuleMqtt::configure(config_t& config) {
    ModuleCard::begin();
    // be sure the dat file is recreated from most current config
    ModuleCard::removeFile(MQTT_CONFIG__DAT);
    ModuleMqtt::createDat(config);
}

void ModuleMqtt::createDat(config_t& config) {

    File mqttFileJson = LittleFS.open(MQTT_CONFIG_JSON.c_str(), FILE_READ);
    if (mqttFileJson) {
        JsonDocument jsonDocument;
        DeserializationError error = deserializeJson(jsonDocument, mqttFileJson);
        if (!error) {

            config.mqtt.configStatus = CONFIG_STAT__APPLIED;

            File mqttFileDat = LittleFS.open(MQTT_CONFIG__DAT.c_str(), FILE_WRITE);  // the file has been checked to not exist
            if (mqttFileDat) {

                bool use = jsonDocument[JSON_KEY_______USE] | false;
                String srv = jsonDocument[JSON_KEY____SERVER] | "";
                uint16_t prt = jsonDocument[JSON_KEY______PORT] | 0;
                String usr = jsonDocument[JSON_KEY______USER] | "";
                String pwd = jsonDocument[JSON_KEY_______PWD] | "";
                String cli = jsonDocument[JSON_KEY____CLIENT] | "";
                uint8_t min = jsonDocument[JSON_KEY___MINUTES] | 15;

                mqtt____t mqtt;
                mqtt.use = use;
                mqtt.prt = prt;
                mqtt.min = min;
                srv.toCharArray(mqtt.srv, 64);
                usr.toCharArray(mqtt.usr, 64);
                pwd.toCharArray(mqtt.pwd, 64);
                cli.toCharArray(mqtt.cli, 16);

                mqttFileDat.write((byte*)&mqtt, sizeof(mqtt));
                mqttFileDat.close();
            }

        } else {
            // TODO :: handle this condition (+ handle when dat file could not be opened)
        }
        mqttFileJson.close();
    } else {
        // TODO :: handle this condition
    }
}

mqtt____stat__e ModuleMqtt::checkDatStat(mqtt____t& mqtt) {

    if (!mqtt.use) {
        return MQTT_CNF____NOT_USED;
    } else if (mqtt.srv == "") {
        return MQTT_CNF_________SRV;
    } else if (mqtt.cli == "") {
        return MQTT_CNF_________CLI;
    } else if (mqtt.prt == 0) {
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

    // #ifdef USE___SERIAL
    //     Serial.printf("before mqtt publish, heap: %u\n", ESP.getFreeHeap());
    // #endif

    ModuleCard::begin();

    if (!ModuleCard::existsPath(MQTT_CONFIG__DAT)) {
        ModuleMqtt::createDat(config);
    }

    // TODO :: reenable
    File mqttFileDat = LittleFS.open(MQTT_CONFIG__DAT.c_str(), FILE_READ);
    if (mqttFileDat) {
        mqtt____t mqtt;
        if (mqttFileDat.available()) {
            mqttFileDat.read((byte*)&mqtt, sizeof(mqtt));

            mqtt____stat__e datStat = ModuleMqtt::checkDatStat(mqtt);
            if (datStat == MQTT______________OK) {  // a set of config worth trying

                WiFiClient* wifiClient;
                char* certFileData = NULL;
                if (ModuleCard::existsPath(MQTT_CONFIG__CRT)) {
                    wifiClient = new WiFiClientSecure();
                    File certFile = LittleFS.open(MQTT_CONFIG__CRT.c_str(), O_RDONLY);
                    certFileData = (char*)malloc(certFile.size() + 1);
                    certFile.readBytes(certFileData, certFile.size());
                    ((WiFiClientSecure*)wifiClient)->setCACert(certFileData);
                    certFile.close();
                } else {
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
                    config.mqtt.mqttPublishMinutes = mqtt.min;  // set to configured interval
                    config.mqtt.mqttFailureCount = 0;

                    uint8_t pubCount = 0;
                    values_all_t datValue;

                    if (pubCount < MAX_PUB_COUNT) {
                        uint32_t lineLimit = min((uint32_t)Values::values->nextMeasureIndex, (uint32_t)MEASUREMENT_BUFFER_SIZE);
                        uint32_t dataIndex;
                        for (uint32_t lineIndex = 0; lineIndex < lineLimit && pubCount < MAX_PUB_COUNT; lineIndex++) {  // similar code in ValcsvResponse
                            dataIndex = lineIndex + Values::values->nextMeasureIndex - lineLimit;
                            datValue = Values::values->measurements[dataIndex % MEASUREMENT_BUFFER_SIZE];
                            if (datValue.publishable) {
                                if (ModuleMqtt::publishMeasurement(config, &datValue, mqtt.cli, mqttClient)) {
                                    Values::values->measurements[dataIndex % MEASUREMENT_BUFFER_SIZE] = {
                                        datValue.secondstime,  // secondstime
                                        datValue.valuesPms,    // co2-sensor values
                                        datValue.valuesBme,    // bme-sensor values
                                        false                  // publishable
                                    };
                                    pubCount++;
                                } else {
                                    config.mqtt.mqttStatus = MQTT_FAIL____PUBLISH;
                                    pubCount = ERR_PUB_COUNT;
                                    break;  // no further line
                                }
                            }
                        }
                    }

                    // be sure any incoming data is cleaned out
                    int loopExecutionCount = 0;
                    mqttClient->loop();
                    while (wifiClient->available() && loopExecutionCount++ < 5) {
                        mqttClient->loop();
                    }

                    // #ifdef USE___SERIAL
                    //                     Serial.printf("disconnecting mqtt\n");
                    // #endif
                    mqttClient->disconnect();  // calls stop() on wificlient

                } else {
                    config.mqtt.mqttStatus = ModuleMqtt::checkCliStat(mqttClient);
                }

                wifiClient->stop();  // explicit stop to be sure it happened (before resetting certFileData)
                if (certFileData != NULL) {
                    free(const_cast<char*>(certFileData));  // there seemed to be a memory issue with _CA_cert not being released when closing/destroying the WifiClient
                    certFileData = NULL;
                }

                delete mqttClient;  // releases some memory buffer
                delete wifiClient;  // calls stop (again) and deletes an internal sslclient instance
                mqttClient = NULL;
                wifiClient = NULL;

            } else {  // datStat other than MQTT______________OK
                config.mqtt.mqttStatus = datStat;
            }

        } else {  // no data available in dat
            config.mqtt.mqttStatus = MQTT_NO__________DAT;
        }

        mqttFileDat.close();  // close, regardless of data available or not

    } else {  // could not open dat
        config.mqtt.mqttStatus = MQTT_FAIL________DAT;
    }

    // TODO :: appropriate config and values for status
    if (config.mqtt.mqttStatus != MQTT______________OK) {

        if (config.mqtt.mqttStatus >= 40) {                         // non-recoverable
                                                                    // #ifdef USE___SERIAL
                                                                    //             Serial.printf("returning from mqtt failure (non----recoverable), stat: %d, heap: %u\n", config.mqtt.mqttStatus, ESP.getFreeHeap());
                                                                    // #endif
            config.mqtt.mqttPublishMinutes = MQTT_PUBLISH___NEVER;  // new-config needs to be uploaded for retry
        } else {                                                    // recoverable
            config.mqtt.mqttFailureCount++;
            if (config.mqtt.mqttFailureCount > 3) {
                // #ifdef USE___SERIAL
                //                 Serial.printf("returning from mqtt failure (medium-recoverable), stat: %d, heap: %u\n", config.mqtt.mqttStatus, ESP.getFreeHeap());
                // #endif
                config.mqtt.mqttPublishMinutes = MQTT_PUBLISH_RECOVER;  // try to recover after one hour
            } else {
                // #ifdef USE___SERIAL
                //                 Serial.printf("returning from mqtt failure (short--recoverable), stat: %d, heap: %u\n", config.mqtt.mqttStatus, ESP.getFreeHeap());
                // #endif
            }
        }

    } else {
        // do nothing the correct interval must have been set when the status was set to OK, failure count reset there as well
        // #ifdef USE___SERIAL
        //         Serial.printf("returning from mqtt success, heap: %u\n", ESP.getFreeHeap());
        // #endif
    }
}

bool ModuleMqtt::publishMeasurement(config_t& config, values_all_t* value, char* client, PubSubClient* mqttClient) {

    JsonDocument jsonDocument;
    jsonDocument[FIELD_NAME____TIME] = value->secondstime + SECONDS_FROM_1970_TO_2000 - Config::getUtcOffsetSeconds();
    jsonDocument[FIELD_NAME___PM010] = value->valuesPms.pm010;
    jsonDocument[FIELD_NAME___PM025] = value->valuesPms.pm025;
    jsonDocument[FIELD_NAME___PM100] = value->valuesPms.pm100;
    jsonDocument[FIELD_NAME_____DEG] = round(value->valuesBme.deg * 10) / 10.0;
    jsonDocument[FIELD_NAME_____HUM] = round(value->valuesBme.hum * 10) / 10.0;
    jsonDocument[FIELD_NAME_____HPA] = value->valuesBme.pressure;

    // https://arduinojson.org/v7/how-to/use-arduinojson-with-pubsubclient/
    char outputBuf[128];
    serializeJson(jsonDocument, outputBuf);
    return mqttClient->publish(client, outputBuf, true);
}
