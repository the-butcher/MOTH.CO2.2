#include "BoxMqtt.h"
#include <WiFiClientSecure.h>
#include "Measurements.h"
#include "Measurement.h"
#include "BoxConn.h"
#include "BoxEncr.h"
#include "BoxFiles.h"
#include "RTClib.h"
#include <SdFat.h>
#include "AESLib.h"

/**
 * ################################################
 * ## constants
 * ################################################
 */
const String JSON_KEY___ADDR = "srv";
const String JSON_KEY___PORT = "prt";
const String JSON_KEY___USER = "usr";
const String JSON_KEY___PASS = "pwd";
const String JSON_KEY_CLIENT = "cli";
const String JSON_KEY___CERT = "crt";

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

WiFiClient* wifiClient;
PubSubClient* mqttClient;

/**
 * ################################################
 * ## static class variabales                     
 * ################################################
 */
int BoxMqtt::failureCount = 0;
int BoxMqtt::status;
String BoxMqtt::CONFIG_PATH = "/config/mqtt.json";
config_status_t BoxMqtt::configStatus = CONFIG_STATUS_PENDING;

void BoxMqtt::begin() {
  BoxMqtt::updateConfiguration();
}

void BoxMqtt::loop() {
  // loop mqttClient to maintain the connection
  if (mqttClient != NULL && mqttClient->connected()) {
    mqttClient->loop();
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
    mqttFile.open(BoxMqtt::CONFIG_PATH.c_str(), O_RDONLY);

    BoxMqtt::configStatus = CONFIG_STATUS__LOADED;

    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(mqttFile);    

    mqttAddr = root[JSON_KEY___ADDR] | mqttAddr;
    mqttPort = root[JSON_KEY___PORT] | mqttPort;
    mqttUser = root[JSON_KEY___USER] | mqttUser;
    String _mqttPass = root[JSON_KEY___PASS] | mqttPass;
    if (_mqttPass != "") {
      mqttPass = BoxEncr::decrypt(_mqttPass);
    }
    mqttClid = root[JSON_KEY_CLIENT] | mqttClid;
    mqttCert = root[JSON_KEY___CERT] | mqttCert;

    mqttFile.close();

    BoxMqtt::configStatus = CONFIG_STATUS__PARSED;    

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
      ((WiFiClientSecure*)wifiClient)->loadCACert(certFile, certFile.size());
      certFile.close();
    } else {
      wifiClient = new WiFiClient();
    }
    mqttClient = new PubSubClient(*wifiClient);
    mqttClient->setServer(mqttAddr.c_str(), mqttPort);
  }  
}

bool BoxMqtt::checkConnect() {
  if (!mqttClient->connected()) {
    if (mqttUser != "" && mqttPass != "") {
      return mqttClient->connect(mqttClid.c_str(), mqttUser.c_str(), mqttPass.c_str()); // connect with credentials
    } else {
      return mqttClient->connect(mqttClid.c_str()); // connect without credentials
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

void BoxMqtt::publish() {

  BoxMqtt::status = MQTT_STATUS___________________OK;
  if (mqttAddr != "" &&  mqttPort > 0 && BoxConn::getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED) {

    // be sure there are valid wifi and mqtt clients
    BoxMqtt::checkClients();
    bool connected = BoxMqtt::checkConnect();

    if (connected) {

      char mqttClidCO2[mqttClid.length() + 5];
      sprintf(mqttClidCO2, "%s/%s", mqttClid, "CO2");

      int firstPublishableIndex;
      int numPublished = 0;
      while ((firstPublishableIndex = Measurements::getFirstPublishableIndex()) >= 0) {

        Measurement measurement = Measurements::getMeasurement(firstPublishableIndex);
        String time = DateTime(SECONDS_FROM_1970_TO_2000 + measurement.secondstime).timestamp();

        DynamicJsonBuffer jsonBuffer;

        JsonObject &rootCo2 = jsonBuffer.createObject();
        rootCo2["time"] = time;
        rootCo2["co2"] = measurement.valuesCo2.co2;
        rootCo2["temperature"] = measurement.valuesCo2.temperature;
        rootCo2["humidity"] = measurement.valuesCo2.humidity;
        rootCo2["pressure"] = measurement.valuesBme.pressure;
        bool successCo2 = publishJson(mqttClient, rootCo2, mqttClidCO2);

        if (successCo2) {
          Measurements::setPublished(firstPublishableIndex);
        } else {
          BoxMqtt::status = MQTT_STATUS_FAILURE______PUBLISH;
          break;
        }

      }

    } else {

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

    // disconnect will call flush and stop on the wifi client
    // mqttClient->disconnect();

    // delete mqttClient;
    // delete wifiClient;

    // mqttClient = NULL;
    // wifiClient = NULL;

  } else if (mqttAddr == "") { // any config or wifi problem
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

}

bool BoxMqtt::publishJson(PubSubClient* mqttClient, JsonObject &root, char* mqid) {

  String outputStr;
  root.printTo(outputStr);
  int outputLen = outputStr.length() + 1;

  char outputBuf[outputLen];
  outputStr.toCharArray(outputBuf, outputLen);

  return mqttClient->publish(mqid, (uint8_t const *)outputBuf, outputLen);

}