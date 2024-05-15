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

    // ModuleSignal::beep(523);
    // delay(100);
    // ModuleSignal::beep(659);
    // delay(100);

    File32 mqttFileJson;
    bool jsonSuccess = mqttFileJson.open(MQTT_CONFIG_JSON.c_str(), O_RDONLY);
    if (jsonSuccess) {
        StaticJsonBuffer<512> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(mqttFileJson);
        if (root.success()) {

            config.mqtt.configStatus = CONFIG_STAT__APPLIED;

            File32 mqttFileDat;
            bool datSuccess = mqttFileDat.open(MQTT_CONFIG__DAT.c_str(), O_RDWR | O_CREAT | O_AT_END);  // the file has been checked to not exist
            if (datSuccess) {

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

#ifdef USE___SERIAL
    Serial.printf("before mqtt publish, heap: %u\n", ESP.getFreeHeap());
#endif

    ModuleCard::begin();

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
                char* certFileData = NULL;
                if (mqtt.crt != "" && ModuleCard::existsPath(mqtt.crt)) {
                    wifiClient = new WiFiClientSecure();
                    File32 certFile;
                    certFile.open(mqtt.crt, O_RDONLY);
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

                    // max publishable features
                    values_all_t datValue;
                    uint16_t year = 2024;
                    File32 yeaFold;
                    File32 monFold;
                    File32 datFile;
                    char yeaFoldNameBuffer[16];
                    char monFoldNameBuffer[16];
                    char datFileNameBuffer[16];
                    String datFilePath;

                    bool success = true;

                    while (year > 0) {
                        String folderYearName = String(year);
                        if (ModuleCard::existsPath(folderYearName)) {
                            if (yeaFold) {
                                yeaFold.close();
                            }
                            yeaFold.open(folderYearName.c_str(), O_RDONLY);
                            while (monFold.openNext(&yeaFold, O_RDONLY)) {
                                if (monFold.isDirectory()) {
                                    while (datFile.openNext(&monFold, O_RDONLY)) {

                                        yeaFold.getName(yeaFoldNameBuffer, 16);  // i.e. 2024
                                        monFold.getName(monFoldNameBuffer, 16);  // i.e. 05
                                        datFile.getName(datFileNameBuffer, 16);  // i.e. 20240429.dar

                                        char datPathBuffer[48];
                                        sprintf(datPathBuffer, "/%s/%s/%s", String(yeaFoldNameBuffer), String(monFoldNameBuffer), String(datFileNameBuffer));
                                        datFilePath = String(datPathBuffer);  // either dap or dar

                                        if (datFilePath.endsWith(FILE_FORMAT_DATA_PUBLISHABLE)) {

                                            // close and reopen with read and write
                                            datFile.close();
                                            datFile.open(datFilePath.c_str(), O_RDWR);

                                            uint32_t filePos = 0;
                                            while (datFile.available() > 1) {
                                                datFile.read((byte*)&datValue, sizeof(datValue));  // read one measurement from the file
                                                // filePos += sizeof(datFile);
                                                filePos = datFile.position();
                                                if (datValue.publishable = true) {  // must still check for publishable (could have been partially published while measuring)
#ifdef USE___SERIAL
                                                    Serial.printf("datValue.co2: %d\n", datValue.valuesCo2.co2Raw);
#endif
                                                    success = success && ModuleMqtt::publishMeasurement(config, &datValue, mqtt.cli, mqttClient);
                                                    datFile.seekSet(filePos - 2);  // the assumed position where the publishable flag is stored
                                                    datFile.write(false);          // set to publishable false
                                                    datFile.seekSet(filePos);      // reset to original file position
                                                    if (!success) {
                                                        break;  // stop reading from this file
                                                    }
                                                    mqttClient->loop();
                                                }
                                            }
                                            if (success) {  // when all measurements from one file were published, mark this file as archive
                                                if (!SensorTime::isPersistPath(datFilePath)) {
                                                    // this leads to the file havin persispath being published over and over
                                                    String darFilePath = String(datFilePath);
                                                    darFilePath.replace(FILE_FORMAT_DATA_PUBLISHABLE, FILE_FORMAT_DATA____ARCHIVED);
                                                    ModuleCard::renameFile32(datFilePath, darFilePath);
                                                }
                                            }
                                        }
                                        datFile.close();
                                        if (!success) {
                                            break;  // dont open another file in case of failure
                                        }
                                    }
                                }
                                monFold.close();
                            }
                            year++;  // continue with next year
                            break;
                        } else {
                            year = 0;  // no further year search
                        }
                    }
                    if (datFile) {
                        datFile.close();
                    }
                    if (monFold) {
                        monFold.close();
                    }
                    if (yeaFold) {
                        yeaFold.close();
                    }

                    // if publishing from file was successful so far
                    if (success) {
                        uint32_t lineLimit = min((uint32_t)Values::values->nextMeasureIndex, (uint32_t)MEASUREMENT_BUFFER_SIZE);
                        uint32_t dataIndex;
                        for (uint32_t lineIndex = 0; lineIndex < lineLimit; lineIndex++) {  // similar code in ValcsvResponse
                            dataIndex = lineIndex + Values::values->nextMeasureIndex - lineLimit;
                            datValue = Values::values->measurements[dataIndex % MEASUREMENT_BUFFER_SIZE];
                            if (datValue.publishable) {
                                success = success / ModuleMqtt::publishMeasurement(config, &datValue, mqtt.cli, mqttClient);
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
                    }

                    // be sure any incoming data is cleaned out
                    int loopExecutionCount = 0;
                    mqttClient->loop();
                    while (wifiClient->available() && loopExecutionCount++ < 5) {
                        mqttClient->loop();
                    }

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
                config.mqtt.mqttFailureCount = 5;  // treat as non-recoverable
            }

        } else {  // no data available in dat
            config.mqtt.mqttStatus = MQTT_NO__________DAT;
            config.mqtt.mqttFailureCount = 5;  // treat as non-recoverable
        }

        mqttFileDat.close();  // close, regardless of data available or not

    } else {  // could not open dat
        config.mqtt.mqttStatus = MQTT_FAIL________DAT;
        config.mqtt.mqttFailureCount = 5;  // treat as non-recoverable
    }

    // TODO :: appropriate config and values for status
    if (config.mqtt.mqttStatus != MQTT______________OK) {

        config.mqtt.mqttFailureCount++;
        if (config.mqtt.mqttFailureCount > 3) {
#ifdef USE___SERIAL
            Serial.printf("returning from mqtt failure (non-recoverable), stat: %d, heap: %u\n", config.mqtt.mqttStatus, ESP.getFreeHeap());
#endif
            config.mqtt.mqttPublishMinutes = MQTT_PUBLISH___NEVER;  // no update
        } else {
#ifdef USE___SERIAL
            Serial.printf("returning from mqtt failure (recoverable), stat: %d, heap: %u\n", config.mqtt.mqttStatus, ESP.getFreeHeap());
#endif
        }
    } else {
        // do nothing the correct interval must have been set when the status was set to OK, failure count reset there as well
#ifdef USE___SERIAL
        Serial.printf("returning from mqtt success, heap: %u\n", ESP.getFreeHeap());
#endif
    }
}

bool ModuleMqtt::publishMeasurement(config_t& config, values_all_t* value, char* client, PubSubClient* mqttClient) {

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root[FIELD_NAME____TIME] = SensorTime::getDateTimeSecondsString(value->secondstime);
    root[FIELD_NAME_CO2_LPF] = (uint16_t)round(value->valuesCo2.co2Lpf / VALUE_SCALE_CO2LPF);
    root[FIELD_NAME_CO2_RAW] = value->valuesCo2.co2Raw;
    root[FIELD_NAME_____DEG] = round(SensorScd041::toFloatDeg(value->valuesCo2.deg) * 10) / 10.0;
    root[FIELD_NAME_____HUM] = round(SensorScd041::toFloatHum(value->valuesCo2.hum) * 10) / 10.0;
    root[FIELD_NAME_____HPA] = value->valuesBme.pressure;
    root[FIELD_NAME_____BAT] = round(SensorEnergy::toFloatPercent(value->valuesNrg.percent) * 10) / 10.0;

    // print json to string
    String outputStr;
    root.printTo(outputStr);
    int outputLen = outputStr.length() + 1;

    char outputBuf[outputLen];
    outputStr.toCharArray(outputBuf, outputLen);

    return mqttClient->publish(client, (uint8_t const*)outputBuf, outputLen);
}
