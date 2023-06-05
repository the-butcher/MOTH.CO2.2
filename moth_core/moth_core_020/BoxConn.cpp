#include "esp_wifi_types.h"
#include "WiFiType.h"
#include "esp32-hal.h"

#include "BoxConn.h"
#include "BoxClock.h"
#include "BoxPack.h"
#include "BoxEncr.h"
#include "BoxDisplay.h"
#include "Measurements.h"
#include "Measurement.h"
#include "BoxMqtt.h"
#include "Network.h"
#include "SensorScd041.h"
#include "SensorBme280.h"
#include "BoxFiles.h"
#include <ArduinoJson.h>
#include "File32Response.h"
#include "DataResponse.h"
#include "AESLib.h"
#include "StringPrint.h"
#include <SdFat.h>
#include "ValuesCo2.h"

/**
 * ################################################
 * ## constants
 * ################################################
 */
const char *CODE = "code";
const String JSON_KEY___MINUTES = "min";
const String JSON_KEY__NETWORKS = "ntw";
const String JSON_KEY_______KEY = "key";
const String JSON_KEY_______PWD = "pwd";


/**
 * ################################################
 * ## mutable variables
 * ################################################
 */
AsyncWebServer server(80);

int64_t wifiTimeoutMillis;
int64_t wifiExpiryMillis = 0;
wifi_mode_t mode = WIFI_OFF;

Network configuredNetworks[10];
Network discoveredNetworks[10];

String apNetworkName = "mothbox";
String apNetworkConn = "";
String apNetworkPass = "CO2@420PPM";

String stNetworkName = "";

/**
 * ################################################
 * ## static class variabales                     
 * ################################################
 */
String BoxConn::CONFIG_PATH = "/config/wifi.json";
config_status_t BoxConn::configStatus = CONFIG_STATUS_PENDING;
int BoxConn::requestedCalibrationReference = -1;
bool BoxConn::isHibernationRequired = false;
bool BoxConn::isCo2CalibrationReset = false;
bool BoxConn::isRenderStateRequired = false;
String BoxConn::VNUM = "1.0.004";

void BoxConn::updateConfiguration() {

  BoxConn::configStatus = CONFIG_STATUS_PENDING;

  int _wifiTimeoutMinutes = wifiTimeoutMillis / 60000;

  if (BoxFiles::existsPath(BoxConn::CONFIG_PATH)) {

    BoxConn::configStatus = CONFIG_STATUS_PRESENT;

    File32 wifiFile;
    bool fileSuccess = wifiFile.open(BoxConn::CONFIG_PATH.c_str(), O_RDONLY);
    if (fileSuccess) {

      BoxConn::configStatus = CONFIG_STATUS__LOADED;

      StaticJsonBuffer<512> jsonBuffer;
      JsonObject &root = jsonBuffer.parseObject(wifiFile);
      if (root.success()) {

        // timeout minutes
        _wifiTimeoutMinutes = root[JSON_KEY___MINUTES] | _wifiTimeoutMinutes;

        int configuredNetworkIndex = 0;
        int networkCount = root[JSON_KEY__NETWORKS].as<JsonArray>().size();
        for (int i = 0; i < networkCount; i++) {
          String key = root[JSON_KEY__NETWORKS][i][JSON_KEY_______KEY] | "";
          String pwd = root[JSON_KEY__NETWORKS][i][JSON_KEY_______PWD] | "";
          if (pwd != "") {
            pwd = BoxEncr::decrypt(pwd);
          }
          configuredNetworks[configuredNetworkIndex] = {
            key,
            pwd,
            0,
            WIFI_AUTH_OPEN
          };
          configuredNetworkIndex++;
        }
        BoxConn::configStatus = CONFIG_STATUS__PARSED;

      }
      
      wifiFile.close();

    }
    
  } else {
    BoxConn::configStatus = CONFIG_STATUS_MISSING;
  }

  wifiTimeoutMillis = 60000 * _wifiTimeoutMinutes;
}

bool BoxConn::isExpireable() {
  if (BoxPack::values.powered) {
    wifiExpiryMillis = millis() + wifiTimeoutMillis;
  }
  return mode != WIFI_OFF && millis() > wifiExpiryMillis;  // not off AND expirable
}

String BoxConn::formatConfigStatus(config_status_t configStatus) {
  if (configStatus == CONFIG_STATUS_PENDING) {
    return "PENDING";
  } else if (configStatus == CONFIG_STATUS_PRESENT) {
    return "PRESENT";
  } else if (configStatus == CONFIG_STATUS_MISSING) {
    return "MISSING";
  } else if (configStatus == CONFIG_STATUS__LOADED) {
    return "LOADED";
  } else if (configStatus == CONFIG_STATUS__PARSED) {
    return "PARSED";
  } else {
    return "UNKNOWN";
  }
}

void BoxConn::begin() {

  BoxConn::updateConfiguration();

  WiFi.mode(WIFI_STA);
  mode = WIFI_STA;

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {

    wifiExpiryMillis = millis() + wifiTimeoutMillis;

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60"); // assume status does not change too quick

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root[CODE] = 200;

    root["vnum"] = BoxConn::VNUM;
    root["heap"] = ESP.getFreeHeap();
    root["sram"] = ESP.getPsramSize();
    root["freq"] = ESP.getCpuFreqMHz();
    root["boff"] = SensorBme280::getTemperatureOffset();

    JsonObject &encrJo = root.createNestedObject("encr");
    encrJo["config"] = BoxConn::formatConfigStatus(BoxEncr::configStatus);    

    JsonObject &wifiJo = root.createNestedObject("wifi");
    wifiJo["config"] = BoxConn::formatConfigStatus(BoxConn::configStatus);      

    JsonObject &dispJo = root.createNestedObject("disp");
    dispJo["config"] = BoxConn::formatConfigStatus(BoxDisplay::configStatus);

    JsonObject &mqttJo = root.createNestedObject("mqtt");
    mqttJo["config"] = BoxConn::formatConfigStatus(BoxMqtt::configStatus);
    mqttJo["status"] = BoxMqtt::status;
    mqttJo["active"] = BoxMqtt::isConfiguredToBeActive();
    mqttJo["pcount"] = Measurements::getPublishableCount();

    root.printTo(*response);
    request->send(response);

  });  
  server.on("/api/reset", HTTP_GET, [](AsyncWebServerRequest *request) {

    ESP.restart();

  });
  server.on("/api/latest", HTTP_GET, [](AsyncWebServerRequest *request) {

    wifiExpiryMillis = millis() + wifiTimeoutMillis;

    AsyncResponseStream *response = request->beginResponseStream("application/json");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    Measurement latestMeasurement = Measurements::getLatestMeasurement();
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + latestMeasurement.secondstime);

    ValuesCo2 valuesCo2 = BoxDisplay::getDisplayValues();

    root["time"] = BoxClock::getDateTimeString(date);
    root["co2"] = valuesCo2.co2;
    root["temperature"] = round(valuesCo2.temperature * 10) / 10.0;
    root["humidity"] = round(valuesCo2.humidity * 10) / 10.0;
    root["pressure"] = latestMeasurement.valuesBme.pressure;
    root["percent"] = latestMeasurement.valuesBat.percent;

    int maxAge = 60 - (BoxClock::getDate().secondstime() - latestMeasurement.secondstime); // time until next measurement
    char maxAgeBuf[16];
    sprintf(maxAgeBuf, "max-age=%s", String(maxAge));
    response->addHeader("Cache-Control", maxAgeBuf);

    root.printTo(*response);
    request->send(response);

  });
  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request) {

    wifiExpiryMillis = millis() + wifiTimeoutMillis;

    DataResponse *response = new DataResponse();

    int maxAge = 60 - (BoxClock::getDate().secondstime() - Measurements::getLatestMeasurement().secondstime); // time until next measurement
    char maxAgeBuf[16];
    sprintf(maxAgeBuf, "max-age=%s", String(maxAge));
    response->addHeader("Cache-Control", maxAgeBuf);

    request->send(response);

  });
  server.on("/api/folder", HTTP_GET, [](AsyncWebServerRequest *request) {

    wifiExpiryMillis = millis() + wifiTimeoutMillis;

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=10"); // assume no frequent changes in folder content

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root[CODE] = 200;

    JsonArray &foldersJa = root.createNestedArray("folders");
    JsonArray &filesJa = root.createNestedArray("files");

    String folderName = "/";
    if (request->hasParam("folder")) {
      folderName = request->getParam("folder")->value();
    }
    File32 folder;
    folder.open(folderName.c_str(), O_RDONLY);
    File32 file;

    while (file.openNext(&folder, O_RDONLY)) {
      if (!file.isHidden()) {

        char nameBuf[16];
        file.getName(nameBuf, 16);
        String name = String(nameBuf);

        if (!BoxEncr::CONFIG_PATH.endsWith(name)) {

          String output;
          StringPrint stringPrint((String &)output);
          file.printModifyDateTime(&stringPrint);

          if (file.isDirectory()) {
            JsonObject &itemJo = foldersJa.createNestedObject();
            itemJo["folder"] = name;
            itemJo["last"] = output;
          } else {
            JsonObject &itemJo = filesJa.createNestedObject();
            itemJo["size"] = file.size();
            itemJo["file"] = name;
            itemJo["last"] = output;
          }

        }

      }
      file.close();
    }
    folder.close();

    root.printTo(*response);
    request->send(response);

  });
  server.on("/api/file", HTTP_GET, [](AsyncWebServerRequest *request) {

    wifiExpiryMillis = millis() + wifiTimeoutMillis;

    bool appendUnsaved;
    String dataFileName;
    if (request->hasParam("file")) {

      dataFileName = "/" + request->getParam("file")->value();
      if (dataFileName != BoxEncr::CONFIG_PATH && BoxFiles::existsPath(dataFileName)) { // hide encr from api

        File32Response *response = new File32Response(dataFileName, "text/csv");
        
        int maxAge = 86400; // 24 hours
        if (dataFileName == Measurements::dataFileNameCurr) {
          maxAge = (Measurements::csvBufferSize - Measurements::getCsvBufferIndex()) * 60; // time until next file update
        }
        char maxAgeBuf[16];
        sprintf(maxAgeBuf, "max-age=%s", String(maxAge));
        response->addHeader("Cache-Control", maxAgeBuf);

        // TODO :: refect actual content type
        request->send(response);

      } else {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->addHeader("Cache-Control", "max-age=60");
        DynamicJsonBuffer jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        root[CODE] = 404;  // file not found
        root["file"] = dataFileName;
        root["desc"] = "file not found";
        root.printTo(*response);
        request->send(response);
      }

    } else {
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      response->addHeader("Cache-Control", "max-age=60");
      DynamicJsonBuffer jsonBuffer;
      JsonObject &root = jsonBuffer.createObject();
      root[CODE] = 400;  // bad request
      root["file"] = dataFileName;
      root["desc"] = "file required";
      root.printTo(*response);
      request->send(response);
    }

  });
  server.on("/api/encrypt", HTTP_GET, [](AsyncWebServerRequest *request) {

    wifiExpiryMillis = millis() + wifiTimeoutMillis;

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=10"); // tell the browser to not call too often

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    if (request->hasParam("value")) {

      String encrypted = BoxEncr::encrypt(request->getParam("value")->value());
      root[CODE] = 200;
      root["encr"] = encrypted;

    } else {
      root[CODE] = 400;  // bad request
      root["desc"] = "value required";
    }

    root.printTo(*response);
    request->send(response);

  });  
  server.on("/api/upload", HTTP_POST, [](AsyncWebServerRequest *request) {

      wifiExpiryMillis = millis() + wifiTimeoutMillis;
      request->send(200);
   
  }, BoxConn::handleUpload);
  server.on("/api/delete", HTTP_GET, [](AsyncWebServerRequest *request) {

    wifiExpiryMillis = millis() + wifiTimeoutMillis;

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    if (request->hasParam("file")) {
      String dataFileName = "/" + request->getParam("file")->value();
      root["file"] = dataFileName;
      if (BoxFiles::existsPath(dataFileName)) {
        bool success = BoxFiles::removeFile32(dataFileName);
        root[CODE] = success ? 200 : 500;
      } else {
        root[CODE] = 404;  // file not found
      }
    } else if (request->hasParam("folder")) {
      String folderName = "/" + request->getParam("folder")->value();
      root["folder"] = folderName;
      if (BoxFiles::existsPath(folderName)) {
        bool success = BoxFiles::removeFolder(folderName);
        root[CODE] = success ? 200 : 500;
      } else {
        root[CODE] = 404;  // folder not found
      }
    } else {
      root[CODE] = 400;
      root["desc"] = "file or folder required";
    }

    root.printTo(*response);
    request->send(response);

  });
  server.on("/api/networks", HTTP_GET, [](AsyncWebServerRequest *request) {

    wifiExpiryMillis = millis() + wifiTimeoutMillis;

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=10"); // networks are only evaluated when turning wifi off, then on again, ne frequent changes

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root[CODE] = 200;

    JsonArray &networksJa = root.createNestedArray("networks");
    Network network;
    for (int networkIndex = 0; networkIndex < 10; networkIndex++) {

      network = discoveredNetworks[networkIndex];
      if (network.rssi != 0) {
        JsonObject &networkJo = networksJa.createNestedObject();
        networkJo["ssid"] = network.ssid;
        networkJo["pass"] = network.pass != "" ? "***" : "";
        networkJo["rssi"] = network.rssi;
        networkJo["encr"] = network.encr;
      }
    }

    root.printTo(*response);
    request->send(response);

  });
  server.on("/api/display", HTTP_GET, [](AsyncWebServerRequest *request) {
    
    wifiExpiryMillis = millis() + wifiTimeoutMillis;

    AsyncResponseStream *response = request->beginResponseStream("application/json");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root[CODE] = 200;

    if (request->hasParam("display")) {
      String requestedDisplay = request->getParam("display")->value();
      BoxConn::isRenderStateRequired = true;
      root["display"] = requestedDisplay;      
      if (requestedDisplay == "state_table") {
        BoxDisplay::setState(DISPLAY_STATE_TABLE);
      } else if (requestedDisplay == "state_chart") {
        BoxDisplay::setState(DISPLAY_STATE_CHART);
      } else if (requestedDisplay == "theme_light") {
        BoxDisplay::setTheme(DISPLAY_THEME_LIGHT);
      } else if (requestedDisplay == "theme_dark") {
        BoxDisplay::setTheme(DISPLAY_THEME__DARK);
      } else if (requestedDisplay == "value_co2") {
        BoxDisplay::setValue(DISPLAY_VALUE___CO2);
      } else if (requestedDisplay == "value_deg") {
        BoxDisplay::setValue(DISPLAY_VALUE___DEG);
      } else if (requestedDisplay == "value_hum") {
        BoxDisplay::setValue(DISPLAY_VALUE___HUM);
      } else if (requestedDisplay == "value_hpa") {
        BoxDisplay::setValue(DISPLAY_VALUE___HPA);
      } else {
        BoxConn::isRenderStateRequired = false;
        root[CODE] = 400;
        root["desc"] = "invalid display value";
      }
    } else {
      root[CODE] = 400;
      root["desc"] = "display required";
    }
    
    root.printTo(*response);
    request->send(response);

  });
  server.on("/api/disconnect", HTTP_GET, [](AsyncWebServerRequest *request) {

    wifiExpiryMillis = millis() + 1000;  // have a short wifi expiry

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=10");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    root[CODE] = 200;

    root.printTo(*response);
    request->send(response);
  });
  server.on("/api/calibrate", HTTP_GET, [](AsyncWebServerRequest *request) {

    wifiExpiryMillis = millis() + wifiTimeoutMillis;

    AsyncResponseStream *response = request->beginResponseStream("application/json");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root[CODE] = 200;

    if (request->hasParam("ref")) {
      String refRaw = request->getParam("ref")->value();
      if (BoxConn::checkNumeric(refRaw)) {
        int ref = refRaw.toInt();
        if (ref >= 400) {
          BoxConn::requestedCalibrationReference = ref;
          root["ref"] = ref;
          response->addHeader("Cache-Control", "max-age=180"); // no sense in calibrating quicker than 3 minutes
        } else {
          root[CODE] = 400;
          root["desc"] = "ref must be >= 400";
        }
      } else {
        root[CODE] = 400;
        root["desc"] = "ref must be numeric";
      }
    } else {
      root[CODE] = 400;
      root["desc"] = "ref must be specified";
    }

    root.printTo(*response);
    request->send(response);
  });
  server.on("/api/hibernate", HTTP_GET, [](AsyncWebServerRequest *request) {

    wifiExpiryMillis = millis() + wifiTimeoutMillis;

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root[CODE] = 200;

    // store the hibernation indicator flag
    BoxConn::isHibernationRequired = true;

    root.printTo(*response);
    request->send(response);
  });
  server.on("/api/co2_reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    
    wifiExpiryMillis = millis() + wifiTimeoutMillis;

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=180");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root[CODE] = 200;

    // store the co2_reset indicator flag
    BoxConn::isCo2CalibrationReset = true;

    root.printTo(*response);
    request->send(response);
  });
  server.onNotFound([](AsyncWebServerRequest *request) {

    String url = request->url();
    if (BoxFiles::existsPath(url)) {

      wifiExpiryMillis = millis() + wifiTimeoutMillis;

      String fileType = url.substring(url.indexOf("."));

      if (fileType == ".html") {
        fileType = "text/html";
      } else if (fileType == ".js") {
        fileType = "application/javascript";
      } else if (fileType == ".css") {
        fileType = "text/css";
      } else if (fileType == ".ttf") {
        fileType = "font/ttf";
      }
      
      File32Response *response = new File32Response(url, fileType);
      response->addHeader("Last-Modified", "Mon, 22 May 2023 00:00:00 GMT"); // TODO this should change with updates to the server files
      request->send(response);

    } else {

      if (request->method() == HTTP_OPTIONS) {
        request->send(200);
      } else {
        request->send(404);
      }

    }



  });

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();

  WiFi.mode(WIFI_OFF);
  mode = WIFI_OFF;

  uint64_t _chipmacid = 0LL;
  esp_efuse_mac_get_default((uint8_t *)(&_chipmacid));

  char networkNameBuf[20];
  sprintf(networkNameBuf, "mothbox%s", String(_chipmacid, HEX));
  apNetworkName = String(networkNameBuf);

  char networkConnBuf[48];
  sprintf(networkConnBuf, "WIFI:T:WPA;S:mothbox%s;P:CO2@420PPM;;", String(_chipmacid, HEX));
  apNetworkConn = String(networkConnBuf);

  WiFi.onEvent(BoxConn::handleStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);

}

void BoxConn::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

  if (request->hasParam("file", true)) {

    String dataFileName = "/" + request->getParam("file", true)->value();

    // delete file if exists
    if (index == 0) {
      if (BoxFiles::existsPath(dataFileName) && index == 0) {
        BoxFiles::removeFile32(dataFileName);
      } else {
        int lastIndexOfSlash = dataFileName.lastIndexOf("/");
        if (lastIndexOfSlash > 0) {
          String dataFilePath = dataFileName.substring(0, lastIndexOfSlash);
          BoxFiles::buildFolders(dataFilePath);
        }
      }
    }

    File32 targetFile;
    targetFile.open(dataFileName.c_str(), O_RDWR | O_CREAT | O_AT_END);
    for (size_t i = 0; i < len; i++) {
      targetFile.write(data[i]);
    }
    targetFile.sync();
    targetFile.close();

    if (dataFileName == BoxEncr::CONFIG_PATH) {
      BoxEncr::updateConfiguration();
      BoxConn::updateConfiguration(); // may have changed encryption
      BoxMqtt::updateConfiguration(); // may have changed encryption
    } else if (dataFileName == BoxDisplay::CONFIG_PATH) {
      BoxDisplay::updateConfiguration();
    } else if (dataFileName == BoxConn::CONFIG_PATH) {
      BoxConn::updateConfiguration();
    } else if (dataFileName == BoxMqtt::CONFIG_PATH) {
      BoxMqtt::updateConfiguration();
    }

  }

}

void BoxConn::handleStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  BoxClock::updateFromNtp(); // give the clock an opportunity to update
}

bool BoxConn::checkNumeric(String value) {
  uint16_t numberCount = 0;
  for (uint16_t i = 0; i < value.length(); i++) {
    if (
      value[i] == '0' || value[i] == '1' || value[i] == '2' || value[i] == '3' || value[i] == '4' || value[i] == '5' || value[i] == '6' || value[i] == '7' || value[i] == '8' || value[i] == '9') {
      numberCount++;
    }
  }
  return numberCount == value.length() ? true : false;
}

void BoxConn::on() {

  stNetworkName = ""; // reset network, we dont know where we will end up being connected to

  // find the first network that has a configured password
  int ssidCount = WiFi.scanNetworks();
  String ssidCandidate;
  String passCandidate;
  String passDefault = "";

  for (int ssidIndex = 0; ssidIndex < ssidCount; ssidIndex++) {

    ssidCandidate = WiFi.SSID(ssidIndex);
    passCandidate = "";

    // see if anything is configured
    for (int i = 0; i < 10; i++) {
      if (configuredNetworks[i].ssid == ssidCandidate) {
        passCandidate = configuredNetworks[i].pass;
      }
    }

    if (passCandidate != passDefault) {

      WiFi.mode(WIFI_STA);
      mode = WIFI_STA;
      WiFi.begin(ssidCandidate.c_str(), passCandidate.c_str());

      // allow some time for a connection to be established
      for (int i = 0; i < 10; i++) {
        if (WiFi.status() == WL_CONNECTED) {
          stNetworkName = ssidCandidate;
          break;
        }
        delay(250);
      }
    }

    // prevent crash that would occur if an index outside the array size were adressed
    if (ssidIndex < 10) {
      discoveredNetworks[ssidIndex] = {
        ssidCandidate,
        passCandidate,
        WiFi.RSSI(ssidIndex),
        WiFi.encryptionType(ssidIndex)
      };
    }
  }

  // add empty networks until the list is filled
  for (int ssidIndex = ssidCount; ssidIndex < 10; ssidIndex++) {
    discoveredNetworks[ssidIndex] = {
      "",
      "",
      0,
      WIFI_AUTH_OPEN
    };
  }

  // either no network found to connect to or no connection possible -> go into ap mode
  if (WiFi.status() != WL_CONNECTED) {

    BoxConn::off();
    delay(500);

    WiFi.mode(WIFI_AP);
    mode = WIFI_AP;

    char apBuf[apNetworkName.length() + 1];
    apNetworkName.toCharArray(apBuf, apNetworkName.length() + 1);

    char psBuf[apNetworkPass.length() + 1];
    apNetworkPass.toCharArray(psBuf, apNetworkPass.length() + 1);

    WiFi.softAP(apBuf, psBuf);
    mode = WIFI_AP;
  }

  wifiExpiryMillis = millis() + wifiTimeoutMillis;

}

void BoxConn::off() {

  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  mode = WIFI_OFF;
}

String BoxConn::getRootUrl() {
  if (mode == WIFI_AP) {
    return "http://" + WiFi.softAPIP().toString() + "/server/client.html";
  } else if (mode == WIFI_STA) {
    return "http://" + WiFi.localIP().toString() + "/server/client.html";
  } else {
    return "";
  }
}

String BoxConn::getNetworkName() {
  if (mode == WIFI_AP) {
    return apNetworkConn;
  } else if (stNetworkName != "") {
    char networkBuf[stNetworkName.length() + 17];
    sprintf(networkBuf, "%s%s%s", "WIFI:T:WPA;S:", stNetworkName, ";;;");
    return networkBuf;
  } else {
    return "";
  }
}

String BoxConn::getAddress() {
  if (mode == WIFI_STA) {
    return WiFi.localIP().toString();
  } else if (mode == WIFI_AP) {
    return WiFi.softAPIP().toString();
  } else if (mode == WIFI_OFF) {
    return "wifi off";
  } else {
    return "wifi unknown";
  }
}

String BoxConn::getNetworkPass() {
  if (mode == WIFI_AP) {
    return apNetworkPass;
  } else {
    return "";
  }
}

wifi_mode_t BoxConn::getMode() {
  return mode;
}
