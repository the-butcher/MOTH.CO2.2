#include "ModuleServer.h"

#include <ArduinoJson.h>

#include "DatCsvResponse.h"
#include "File32Response.h"
#include "ValuesResponse.h"
#include "modules/ModuleSdcard.h"
#include "modules/ModuleWifi.h"
#include "types/Define.h"
#include "types/Values.h"

AsyncWebServer ModuleServer::server(80);
bool ModuleServer::hasBegun = false;
uint16_t ModuleServer::requestedCalibrationReference = 0;

void ModuleServer::begin() {
    if (!ModuleServer::hasBegun) {
        server.on("/api/latest", HTTP_GET, handleApiLatest);
        server.on("/api/status", HTTP_GET, handleApiStatus);
        server.on("/api/folder", HTTP_GET, handleApiFolder);
        server.on("/api/datcsv", HTTP_GET, handleApiDatCsv);  // get csv from data stored in dat files
        server.on("/api/valcsv", HTTP_GET, handleApiValCsv);  // get csv from data measured in the last hour
        server.on("/api/co2cal", HTTP_GET, handleApiCo2Cal);
        server.onNotFound(serveStatic);
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
        server.begin();
        ModuleServer::hasBegun = true;
    }
}

void ModuleServer::handleApiLatest(AsyncWebServerRequest *request) {

    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;

    values_all_t latestValue = Values::values->measurements[(Values::values->nextMeasureIndex - 1) % MEASUREMENT_BUFFER_SIZE];

    root["time"] = SensorTime::getDateTimeSecondsString(latestValue.secondstime);
    root["co2_lpf"] = latestValue.valuesCo2.co2Lpf;
    root["co2_raw"] = latestValue.valuesCo2.co2Raw;
    root["temperature"] = round(SensorScd041::toFloatDeg(latestValue.valuesCo2.deg) * 10) / 10.0;
    root["humidity"] = round(SensorScd041::toFloatHum(latestValue.valuesCo2.hum) * 10) / 10.0;
    root["pressure"] = latestValue.valuesBme.pressure;
    root["battery"] = SensorEnergy::toFloatPercent(latestValue.valuesNrg.percent);

    int maxAge = SECONDS_PER___________MINUTE + 10 - SensorTime::getSecondstime() % SECONDS_PER___________MINUTE;  // 10 seconds extra to account for measure time being some seconds behind the full minute
    char maxAgeBuf[16];
    sprintf(maxAgeBuf, "max-age=%s", String(maxAge));
    response->addHeader("Cache-Control", maxAgeBuf);

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

void ModuleServer::handleApiFolder(AsyncWebServerRequest *request) {

    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=10");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;

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

            uint16_t pdate;
            uint16_t ptime;
            file.getAccessDateTime(&pdate, &ptime);

#ifdef USE___SERIAL
            Serial.printf("listing file or folder, name %s, pdate: %d, ptime: %d\n", name, pdate, ptime);
#endif

            char lastModifiedBuffer[32];
            sprintf(lastModifiedBuffer, "%d-%02d-%02d %02d:%02d:%02d", FS_YEAR(pdate), FS_MONTH(pdate), FS_DAY(pdate), FS_HOUR(ptime), FS_MINUTE(ptime), FS_SECOND(ptime));
            String lastModified = String(lastModifiedBuffer);

            if (file.isDirectory()) {
                JsonObject &itemJo = foldersJa.createNestedObject();
                itemJo["folder"] = name;
                itemJo["last"] = lastModified;
            } else {
                JsonObject &itemJo = filesJa.createNestedObject();
                itemJo["size"] = file.size();
                itemJo["file"] = name;
                itemJo["last"] = lastModified;
            }
        }
        file.close();
    }
    folder.close();

    root.printTo(*response);
    request->send(response);
}

void ModuleServer::handleApiDatCsv(AsyncWebServerRequest *request) {
#ifdef USE___SERIAL
    Serial.println("entering handleApiDatCsv");
#endif
    ModuleWifi::access();
    if (request->hasParam("file")) {
        String datFileName = "/" + request->getParam("file")->value();
        String csvLine;
        if (ModuleSdcard::existsPath(datFileName)) {
            DatCsvResponse *response = new DatCsvResponse(datFileName);
            request->send(response);
        } else {
            AsyncResponseStream *response = request->beginResponseStream("application/json");
            response->addHeader("Cache-Control", "max-age=60");
            DynamicJsonBuffer jsonBuffer;
            JsonObject &root = jsonBuffer.createObject();
            root["code"] = 404;
            root["file"] = datFileName;
            root["desc"] = "file not found";
            root.printTo(*response);
            request->send(response);
        }
    } else {
        AsyncResponseStream *response = request->beginResponseStream("application/json");
        response->addHeader("Cache-Control", "max-age=60");
        DynamicJsonBuffer jsonBuffer;
        JsonObject &root = jsonBuffer.createObject();
        root["code"] = 400;
        root["desc"] = "file required";
        root.printTo(*response);
        request->send(response);
    }
}

void ModuleServer::handleApiValCsv(AsyncWebServerRequest *request) {
#ifdef USE___SERIAL
    Serial.println("entering handleApiValCsv");
#endif
    ModuleWifi::access();
    ValuesResponse *response = new ValuesResponse();
    request->send(response);
}

void ModuleServer::handleApiCo2Cal(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;

    if (request->hasParam("ref")) {
        String refRaw = request->getParam("ref")->value();
        int ref = refRaw.toInt();
        if (ref >= 400) {
            ModuleServer::requestedCalibrationReference = ref;
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

void ModuleServer::fillBufferWithCsv(values_all_t *value, uint8_t *data, uint16_t offset) {
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + value->secondstime);
    float deg = SensorScd041::toFloatDeg(value->valuesCo2.deg);
    float hum = SensorScd041::toFloatHum(value->valuesCo2.hum);
    float nrg = SensorEnergy::toFloatPercent(value->valuesNrg.percent);
    char csvBuffer[CSV_LINE_LENGTH + 1];
    sprintf(csvBuffer, CSV_FRMT.c_str(), date.year(), date.month(), date.day(), date.hour(), date.minute(), date.second(), value->valuesCo2.co2Lpf, value->valuesCo2.co2Raw, deg, hum, value->valuesBme.pressure, nrg);
    for (uint8_t charIndex = 0; charIndex < CSV_LINE_LENGTH; charIndex++) {
        if (csvBuffer[charIndex] == '.') {
            data[offset + charIndex] = ',';
        } else {
            data[offset + charIndex] = csvBuffer[charIndex];
        }
    }
}
