#include "ModuleServer.h"

#include <ArduinoJson.h>

#include "File32Response.h"
#include "modules/ModuleSdcard.h"
#include "modules/ModuleWifi.h"

AsyncWebServer ModuleServer::server(80);
bool ModuleServer::hasBegun = false;

void ModuleServer::begin() {
    if (!ModuleServer::hasBegun) {
        server.on("/api/status", HTTP_GET, handleApiStatus);
        server.onNotFound(serveStatic);
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
        server.begin();
        ModuleServer::hasBegun = true;
    }
}

void ModuleServer::handleApiStatus(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;
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
