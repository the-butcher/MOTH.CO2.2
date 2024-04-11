#include "ModuleMqtt.h"

#include <ArduinoJson.h>
#include <SdFat.h>
#include <WiFiClientSecure.h>

#include "modules/ModuleCard.h"
#include "types/Define.h"

void ModuleMqtt::configure(config_t& config) {
    ModuleCard::begin();
    // be sure the dat file is recreated from most current config
    ModuleCard::removeFile32(MQTT_CONFIG__DAT);
    ModuleMqtt::createDat(config);
}

void ModuleMqtt::createDat(config_t& config) {

    File32 mqttFileJson;
    bool jsonSuccess = mqttFileJson.open(MQTT_CONFIG_JSON.c_str(), O_RDONLY);
    if (jsonSuccess) {
        StaticJsonBuffer<512> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(mqttFileJson);
        if (root.success()) {

            config.mqtt.configStatus = CONFIG_STAT__APPLIED;

            File32 mqttFileDat;
            mqttFileDat.open(MQTT_CONFIG__DAT.c_str(), O_RDWR | O_CREAT | O_AT_END);  // the file has been checked to not exist

            String srv = root[JSON_KEY____SERVER] | "";
            uint16_t prt = root[JSON_KEY______PORT] | 0;
            String usr = root[JSON_KEY______USER] | "";
            String pwd = root[JSON_KEY_______PWD] | "";
            String cli = root[JSON_KEY____CLIENT] | "";
            String crt = root[JSON_KEY______CERT] | "";
            uint8_t min = root[JSON_KEY___MINUTES] | 15;

            mqtt____t mqtt;
            mqtt.prt = prt;
            mqtt.min = min;
            srv.toCharArray(mqtt.srv, 64);
            usr.toCharArray(mqtt.usr, 64);
            pwd.toCharArray(mqtt.pwd, 64);
            cli.toCharArray(mqtt.cli, 16);
            crt.toCharArray(mqtt.crt, 16);

            mqttFileDat.write((byte*)&mqtt, sizeof(mqtt));
            mqttFileDat.close();

        } else {
            // TODO :: handle this condition (+ handle when dat file could not be opened)
        }
    } else {
        // TODO :: handle this condition
    }
    if (mqttFileJson) {
        mqttFileJson.close();
    }
}

mqtt____stat__e ModuleMqtt::checkDatStat(mqtt____t& mqtt) {

    if (mqtt.srv == "") {
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

    if (!ModuleCard::existsPath(MQTT_CONFIG__DAT)) {
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
                if (mqtt.crt != "" && ModuleCard::existsPath(mqtt.crt)) {
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
                    config.mqtt.mqttPublishMinutes = mqtt.min;  // set to configured interval

#ifdef USE___SERIAL
                    Serial.printf("mqtt connected, mqtt.min: %d\n", mqtt.min);
#endif
                    // max publishable features
                    uint32_t lineLimit = min((uint32_t)Values::values->nextMeasureIndex, (uint32_t)MEASUREMENT_BUFFER_SIZE);
                    values_all_t datValue;
                    uint32_t dataIndex;
                    for (uint32_t lineIndex = 0; lineIndex < lineLimit; lineIndex++) {  // similar code in ValuesResponse
                        dataIndex = lineIndex + Values::values->nextMeasureIndex - lineLimit;
                        datValue = Values::values->measurements[dataIndex % MEASUREMENT_BUFFER_SIZE];
                        if (datValue.publishable) {

                            // char mqttClidCO2[mqttClid.length() + 5];
                            // sprintf(mqttClidCO2, "%s/%s", mqttClid, "CO2");

                            DynamicJsonBuffer jsonBuffer;
                            JsonObject& root = jsonBuffer.createObject();
                            root[FIELD_NAME____TIME] = SensorTime::getDateTimeSecondsString(datValue.secondstime);
                            root[FIELD_NAME_CO2_LPF] = datValue.valuesCo2.co2Lpf;
                            root[FIELD_NAME_CO2_RAW] = datValue.valuesCo2.co2Raw;
                            root[FIELD_NAME_____DEG] = round(SensorScd041::toFloatDeg(datValue.valuesCo2.deg) * 10) / 10.0;
                            root[FIELD_NAME_____HUM] = round(SensorScd041::toFloatHum(datValue.valuesCo2.hum) * 10) / 10.0;
                            root[FIELD_NAME_____HPA] = datValue.valuesBme.pressure;
                            root[FIELD_NAME_____BAT] = SensorEnergy::toFloatPercent(datValue.valuesNrg.percent);

                            // print json to string
                            String outputStr;
                            root.printTo(outputStr);
                            int outputLen = outputStr.length() + 1;

                            char outputBuf[outputLen];
                            outputStr.toCharArray(outputBuf, outputLen);

                            bool success = mqttClient->publish(mqtt.cli, (uint8_t const*)outputBuf, outputLen);
                            if (success) {
                                // replace with non-publishable version
                                Values::values->measurements[dataIndex % MEASUREMENT_BUFFER_SIZE] = {
                                    datValue.secondstime,  // secondstime
                                    datValue.valuesCo2,    // co2-sensor values
                                    datValue.valuesBme,    // bme-sensor values
                                    datValue.valuesNrg,    // bat-sensor values
                                    false                  // publishable
                                };
                            } else {
                                config.mqtt.mqttStatus = MQTT_FAIL____PUBLISH;
                                break;
                            }
                        }
                    }

                    // TODO :: find a way to know then the last publication time was, open and read all files from there and publish
                    // finally publish anything currently in measurement buffer
                    // publishable cant be older than mqtt config time (boot time?)
                    // if publishing measurements already stored in file, those files would have to be rewritten with publishable = false flags
                    // how could the file be tagged as completely published

                    mqttClient->disconnect();

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

    // TODO :: appropriate config and values for status
    if (config.mqtt.mqttStatus != MQTT______________OK) {
#ifdef USE___SERIAL
        Serial.printf("before returning infinite publish interval, stat: %d\n", config.mqtt.mqttStatus);
#endif
        config.mqtt.mqttPublishMinutes = MQTT_PUBLISH___NEVER;  // no update
    } else {
        // do nothing the correct interval must have been set when the status was set to OK
#ifdef USE___SERIAL
        Serial.println("before returning from mqtt success");
#endif
    }
}
