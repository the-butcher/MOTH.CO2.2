#include "BoxMqtt.h"

#include <SdFat.h>
#include <WiFiClientSecure.h>

#include "AESLib.h"
#include "BoxConn.h"
#include "BoxEncr.h"
#include "BoxFiles.h"
#include "Measurement.h"
#include "Measurements.h"
#include "RTClib.h"
#include "SensorPmsa003i.h"

/**
 * ################################################
 * ## constants
 * ################################################
 */
const String JSON_KEY______ADDR = "srv";
const String JSON_KEY______PORT = "prt";
const String JSON_KEY______USER = "usr";
const String JSON_KEY______PASS = "pwd";
const String JSON_KEY____CLIENT = "cli";
const String JSON_KEY______CERT = "crt";
const String JSON_KEY_____TOPIC = "top";
const String JSON_KEY___MINUTES = "min";

/**
 * ################################################
 * ## mutable variables
 * ################################################
 */
String mqttAddr = "";
int mqttPort = -1;
String mqttUser = "";
String mqttPass = "";
String mqttClid = "";
String mqttCert = "";

bool isPublishCo2;
bool isPublishBme;
bool isPublishPms;
bool isPublishBat;

int minPublishableMemBufferIndex = 0;
String lastPayload = "";

WiFiClient *wifiClient;
PubSubClient *mqttClient;

/**
 * ################################################
 * ## static class variabales
 * ################################################
 */
int BoxMqtt::failureCount = 0;
int BoxMqtt::status;
String BoxMqtt::CONFIG_PATH = "/config/mqtt.json";
config_status_t BoxMqtt::configStatus = CONFIG_STATUS_PENDING;
bool BoxMqtt::isWifiConnectionRequested = false;
int BoxMqtt::publishIntervalMinutes = 15;  // default :: 15 minutes, will be overridden by config

void BoxMqtt::begin() {
    BoxMqtt::updateConfiguration();
}

bool BoxMqtt::loop() {
    if (mqttClient != NULL && mqttClient->connected()) {
        return mqttClient->loop();
    } else {
        return false;
    }
}

void BoxMqtt::updateConfiguration() {

    BoxMqtt::configStatus = CONFIG_STATUS_PENDING;

    mqttAddr = "";
    mqttPort = -1;
    mqttUser = "";
    mqttPass = "";
    mqttCert = "";
    if (BoxFiles::existsPath(BoxMqtt::CONFIG_PATH)) {

        BoxMqtt::configStatus = CONFIG_STATUS_PRESENT;

        File32 mqttFile;
        bool fileSuccess = mqttFile.open(BoxMqtt::CONFIG_PATH.c_str(), O_RDONLY);
        if (fileSuccess) {

            BoxMqtt::configStatus = CONFIG_STATUS__LOADED;

            StaticJsonBuffer<512> jsonBuffer;
            JsonObject &root = jsonBuffer.parseObject(mqttFile);
            if (root.success()) {

                BoxMqtt::publishIntervalMinutes = max(1, root[JSON_KEY___MINUTES] | BoxMqtt::publishIntervalMinutes);
                minPublishableMemBufferIndex = 1;  // Measurements::memBufferIndx + BoxMqtt::publishIntervalMinutes;

                mqttAddr = root[JSON_KEY______ADDR] | mqttAddr;
                mqttPort = root[JSON_KEY______PORT] | mqttPort;
                mqttUser = root[JSON_KEY______USER] | mqttUser;
                String _mqttPass = root[JSON_KEY______PASS] | mqttPass;
                if (_mqttPass != "") {
                    mqttPass = BoxEncr::decrypt(_mqttPass);
                }
                mqttClid = root[JSON_KEY____CLIENT] | mqttClid;
                mqttCert = root[JSON_KEY______CERT] | mqttCert;

                String mqttTopics = root[JSON_KEY_____TOPIC] | "CO2";
                isPublishCo2 = mqttTopics.indexOf("CO2" >= 0);
                isPublishBme = mqttTopics.indexOf("BME" >= 0);
                isPublishPms = mqttTopics.indexOf("PMS" >= 0) && SensorPmsa003i::ACTIVE;
                isPublishBat = mqttTopics.indexOf("BAT" >= 0);

                BoxMqtt::configStatus = CONFIG_STATUS__PARSED;
            }

            mqttFile.close();
        }
    } else {
        BoxMqtt::configStatus = CONFIG_STATUS_MISSING;
    }

    BoxMqtt::failureCount = 0;

    if (mqttClient != NULL) {
        mqttClient->disconnect();
    }
    delete mqttClient;
    delete wifiClient;
    mqttClient = NULL;
    wifiClient = NULL;
}

void BoxMqtt::checkClients() {

    if (wifiClient == NULL || mqttClient == NULL) {
        if (mqttCert != "" && BoxFiles::existsPath(mqttCert)) {
            wifiClient = new WiFiClientSecure();
            File32 certFile;
            certFile.open(mqttCert.c_str(), O_RDONLY);
            ((WiFiClientSecure *)wifiClient)->loadCACert(certFile, certFile.size());
            certFile.close();
        } else {
            wifiClient = new WiFiClient();
        }
        mqttClient = new PubSubClient(*wifiClient);
        mqttClient->setServer(mqttAddr.c_str(), mqttPort);
        mqttClient->setCallback(BoxMqtt::callback);
    }
}

void BoxMqtt::callback(char *topic, byte *payload, unsigned int length) {

    String currPayload = String((char *)payload).substring(0, length);
    if (currPayload != lastPayload) {

        BoxMqtt::isWifiConnectionRequested = true;  // can there be overlap with the wifi still being on (?)

        char mqttClidACK[mqttClid.length() + 5];
        sprintf(mqttClidACK, "%s/%s", mqttClid, "ACK");

        char payloadBuf[currPayload.length() + 1];
        currPayload.toCharArray(payloadBuf, currPayload.length() + 1);

        mqttClient->publish(mqttClidACK, (uint8_t const *)payloadBuf, currPayload.length() + 1, true);  // send an empty retained message to clear the existing message
    }
    lastPayload = currPayload;
}

bool BoxMqtt::checkConnect() {
    if (!mqttClient->connected()) {

        if (mqttUser != "" && mqttPass != "") {
            // mqttClient->connect(mqttClid.c_str(), mqttUser.c_str(), mqttPass.c_str()); // connect with credentials
            mqttClient->connect(mqttClid.c_str(), mqttUser.c_str(), mqttPass.c_str(), 0, 0, 0, 0, 0);
        } else {
            mqttClient->connect(mqttClid.c_str());  // connect without credentials
        }

        if (mqttClient->connected()) {  // must be connected to subscribe

            char mqttClidCON[mqttClid.length() + 5];
            sprintf(mqttClidCON, "%s/%s", mqttClid, "CON");
            mqttClient->subscribe(mqttClidCON);

            return true;
        } else {
            return false;
        }
    }
    return true;
}

bool BoxMqtt::isConfiguredToBeActive() {
    return mqttAddr != "";
}

bool BoxMqtt::hasFailure() {
    return failureCount >= 5;
}

bool BoxMqtt::isPublishable() {
    return BoxMqtt::isConfiguredToBeActive() && Measurements::memBufferIndx >= minPublishableMemBufferIndex;
}

void BoxMqtt::publish() {

    BoxMqtt::status = MQTT_STATUS___________________OK;
    if (mqttAddr != "" && mqttPort > 0 && BoxConn::getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {

        // be sure there are valid wifi and mqtt clients
        BoxMqtt::checkClients();
        bool connected = BoxMqtt::checkConnect();
        if (connected) {

            int firstPublishableIndex;
            int numPublished = 0;
            while ((firstPublishableIndex = Measurements::getFirstPublishableIndex()) >= 0) {

                Measurement measurement = Measurements::getMeasurement(firstPublishableIndex);
                String time = DateTime(SECONDS_FROM_1970_TO_2000 + measurement.secondstime).timestamp();

                DynamicJsonBuffer jsonBuffer;

                bool successCo2 = false;
                if (isPublishCo2) {

                    char mqttClidCO2[mqttClid.length() + 5];
                    sprintf(mqttClidCO2, "%s/%s", mqttClid, "CO2");

                    JsonObject &rootCo2 = jsonBuffer.createObject();
                    rootCo2["time"] = time;
                    rootCo2["client"] = mqttClid;
                    rootCo2["co2"] = measurement.valuesCo2.co2;
                    rootCo2["temperature"] = measurement.valuesCo2.temperature;
                    rootCo2["humidity"] = measurement.valuesCo2.humidity;
                    successCo2 = publishJson(rootCo2, mqttClidCO2);
                }

                bool successBme = false;
                if (isPublishBme) {

                    char mqttClidBME[mqttClid.length() + 5];
                    sprintf(mqttClidBME, "%s/%s", mqttClid, "BME");

                    JsonObject &rootBme = jsonBuffer.createObject();
                    rootBme["time"] = time;
                    rootBme["client"] = mqttClid;
                    rootBme["temperature"] = measurement.valuesBme.temperature;
                    rootBme["humidity"] = measurement.valuesBme.humidity;
                    rootBme["pressure"] = measurement.valuesBme.pressure;
                    successBme = publishJson(rootBme, mqttClidBME);
                }

                bool successPms = false;
                if (isPublishPms) {

                    char mqttClidPMS[mqttClid.length() + 5];
                    sprintf(mqttClidPMS, "%s/%s", mqttClid, "PMS");

                    JsonObject &rootPms = jsonBuffer.createObject();
                    rootPms["time"] = time;
                    rootPms["client"] = mqttClid;
                    rootPms["pm010"] = (int)round(measurement.valuesPms.pm010);
                    rootPms["pm025"] = (int)round(measurement.valuesPms.pm025);
                    rootPms["pm100"] = (int)round(measurement.valuesPms.pm100);
                    rootPms["pc003"] = measurement.valuesPms.pc003;
                    rootPms["pc005"] = measurement.valuesPms.pc005;
                    rootPms["pc010"] = measurement.valuesPms.pc010;
                    rootPms["pc025"] = measurement.valuesPms.pc025;
                    rootPms["pc050"] = measurement.valuesPms.pc050;
                    rootPms["pc100"] = measurement.valuesPms.pc100;
                    successPms = publishJson(rootPms, mqttClidPMS);
                }

                bool successBat = false;
                if (isPublishBat) {

                    char mqttClidBAT[mqttClid.length() + 5];
                    sprintf(mqttClidBAT, "%s/%s", mqttClid, "BAT");

                    JsonObject &rootBat = jsonBuffer.createObject();
                    rootBat["time"] = time;
                    rootBat["client"] = mqttClid;
                    rootBat["percent"] = measurement.valuesBat.percent;
                    rootBat["voltage"] = measurement.valuesBat.voltage;
                    successBat = publishJson(rootBat, mqttClidBAT);
                }

                if (successCo2 || successBme || successBat) {
                    Measurements::setPublished(firstPublishableIndex);
                } else {
                    BoxMqtt::status = MQTT_STATUS_FAILURE______PUBLISH;
                    break;
                }
            }

            // loop must be called multiple times to get down to incoming messages
            int loopExecutionCount = 0;
            BoxMqtt::loop();
            while (wifiClient->available() && loopExecutionCount++ < 5) {
                BoxMqtt::loop();
            }

            // disconnect will call flush and stop on the wifi client
            // mqttClient->disconnect();

        } else {  // not connected

            int state = mqttClient->state();
            if (state == MQTT_CONNECTION_TIMEOUT) {
                BoxMqtt::status = MQTT_STATUS_TIMEOUT___CONNECTION;
            } else if (state == MQTT_CONNECTION_LOST) {
                BoxMqtt::status = MQTT_STATUS_LOST______CONNECTION;
            } else if (state == MQTT_CONNECT_FAILED) {
                BoxMqtt::status = MQTT_STATUS_FAILURE___CONNECTION;
            } else if (state == MQTT_DISCONNECTED) {
                BoxMqtt::status = MQTT_STATUS_LOST______CONNECTION;
            } else if (state == MQTT_CONNECT_BAD_PROTOCOL) {
                BoxMqtt::status = MQTT_STATUS_INVALID_____PROTOCOL;
            } else if (state == MQTT_CONNECT_BAD_CLIENT_ID) {
                BoxMqtt::status = MQTT_STATUS_INVALID_____CLIENTID;
            } else if (state == MQTT_CONNECT_UNAVAILABLE) {
                BoxMqtt::status = MQTT_STATUS_UNAVAIL___CONNECTION;
            } else if (state == MQTT_CONNECT_BAD_CREDENTIALS) {
                BoxMqtt::status = MQTT_STATUS_INVALID__CREDENTIALS;
            } else if (state == MQTT_CONNECT_UNAUTHORIZED) {
                BoxMqtt::status = MQTT_STATUS_INVALID_UNAUTHORIZED;
            } else {
                BoxMqtt::status = MQTT_STATUS______________UNKNOWN;
            }
        }
    } else if (mqttAddr == "") {  // any config or wifi problem
        BoxMqtt::status = MQTT_STATUS_INVALID_________ADDR;
    } else if (mqttPort <= 0) {
        BoxMqtt::status = MQTT_STATUS_INVALID_________PORT;
    } else if (mqttClid == "") {
        BoxMqtt::status = MQTT_STATUS_INVALID_____CLIENTID;
    } else if (BoxConn::getMode() != WIFI_STA) {
        BoxMqtt::status = MQTT_STATUS_INVALID____WIFI_MODE;
    } else if (WiFi.status() != WL_CONNECTED) {
        BoxMqtt::status = MQTT_STATUS_INVALID__WIFI_STATUS;
    } else {
        BoxMqtt::status = MQTT_STATUS______________UNKNOWN;
    }

    if (BoxMqtt::status != MQTT_STATUS___________________OK) {
        failureCount++;
    }

    minPublishableMemBufferIndex += BoxMqtt::publishIntervalMinutes;
}

bool BoxMqtt::publishJson(JsonObject &root, char *mqid) {

    String outputStr;
    root.printTo(outputStr);
    int outputLen = outputStr.length() + 1;

    char outputBuf[outputLen];
    outputStr.toCharArray(outputBuf, outputLen);

    return mqttClient->publish(mqid, (uint8_t const *)outputBuf, outputLen);
}