#include "ModuleServer.h"

#include <ArduinoJson.h>

#include "File32Response.h"
#include "modules/ModuleSdcard.h"
#include "modules/ModuleWifi.h"

AsyncWebServer ModuleServer::server(80);
bool ModuleServer::hasBegun = false;
uint16_t ModuleServer::requestedCalibrationReference = 0;

void ModuleServer::begin() {
    if (!ModuleServer::hasBegun) {
        server.on("/api/calibrate", HTTP_GET, handleCalibrate);
        server.on("/api/status", HTTP_GET, handleApiStatus);
        server.onNotFound(serveStatic);
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
        server.begin();
        ModuleServer::hasBegun = true;
    }
}

void ModuleServer::handleCalibrate(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;

    if (request->hasParam("ref")) {
        String refRaw = request->getParam("ref")->value();
        int ref = refRaw.toInt();
        if (ref >= 400) {
            ModuleServer::requestedCalibrationReference = ref;  // TODO :: there needs to be some callback
            root["ref"] = ref;
            response->addHeader("Cache-Control", "max-age=180");
        } else {
            root["code"] = 400;
            root["desc"] = "ref must be >= 400";
        }
    } else {
        root["code"] = 400;
        root["desc"] = "ref must be specified";
    }

    root.printTo(*response);
    request->send(response);
}

void ModuleServer::handleApiStatus(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;
    root["vnum"] = VNUM;
    root["heap"] = ESP.getFreeHeap();
    root["sram"] = ESP.getPsramSize();
    root["freq"] = ESP.getCpuFreqMHz();

    JsonObject &scd041Jo = root.createNestedObject("scd041");
    // scd041Jo["co2r"] = SensorScd041::getCo2Reference();
    scd041Jo["toff"] = SensorScd041::getTemperatureOffset();
    scd041Jo["iasc"] = SensorScd041::isAutomaticSelfCalibration();

    // JsonObject &bme280Jo = root.createNestedObject("bme280");
    // bme280Jo["toff"] = SensorBme280::getTemperatureOffset();

    // JsonObject &encrJo = root.createNestedObject("encr");
    // encrJo["config"] = BoxConn::formatConfigStatus(BoxEncr::configStatus);

    // JsonObject &wifiJo = root.createNestedObject("wifi");
    // wifiJo["config"] = BoxConn::formatConfigStatus(BoxConn::configStatus);

    // JsonObject &dispJo = root.createNestedObject("disp");
    // dispJo["config"] = BoxConn::formatConfigStatus(BoxDisplay::configStatus);

    // JsonObject &mqttJo = root.createNestedObject("mqtt");
    // mqttJo["config"] = BoxConn::formatConfigStatus(BoxMqtt::configStatus);
    // mqttJo["status"] = BoxMqtt::status;
    // mqttJo["active"] = BoxMqtt::isConfiguredToBeActive();
    // mqttJo["pcount"] = Measurements::getPublishableCount();

    root.printTo(*response);
    request->send(response);
}

void ModuleServer::serveStatic(AsyncWebServerRequest *request) {
    String url = request->url();
    if (ModuleSdcard::existsPath(url)) {
        ModuleWifi::access();
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
        response->addHeader("Last-Modified", "Mon, 22 May 2023 00:00:00 GMT");
        request->send(response);
    } else {
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            request->send(404);
        }
    }
}
