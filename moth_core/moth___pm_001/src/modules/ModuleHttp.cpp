#include "ModuleHttp.h"

#include <ArduinoJson.h>
#include <Update.h>

#include "File32Response.h"
#include "ValcsvResponse.h"
#include "ValoutResponse.h"
#include "modules/ModuleCard.h"
#include "modules/ModuleDisp.h"
#include "modules/ModuleMqtt.h"
#include "modules/ModuleWifi.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorPms003.h"
#include "types/Define.h"
#include "types/Device.h"
#include "types/Values.h"

AsyncWebServer ModuleHttp::server(80);
bool ModuleHttp::hasBegun = false;
std::function<void(config_t &config, values_t &values)> ModuleHttp::requestedReconfiguration = nullptr;
int ModuleHttp::updateCode = -1;
int ModuleHttp::uploadCode = -1;

void ModuleHttp::begin() {
    if (!ModuleHttp::hasBegun) {
        server.on("/api/latest", HTTP_GET, handleApiLatest);
        server.on("/api/valcsv", HTTP_GET, handleApiValCsv);
        server.on("/api/valout", HTTP_GET, handleApiValOut);
        server.on("/api/datout", HTTP_GET, handleApiDatOut);
        server.on("/api/dirout", HTTP_GET, handleApiDirOut);
        server.on("/api/upload", HTTP_POST, handleApiUpload, ModuleHttp::handleUpload);
        server.on("/api/datdel", HTTP_GET, handleApiDatDel);
        server.on("/api/dirdel", HTTP_GET, handleApiDirDel);
        server.on("/api/dspset", HTTP_GET, handleApiDspSet);
        server.on("/api/status", HTTP_GET, handleApiStatus);
        server.on("/api/netout", HTTP_GET, handleApiNetOut);
        server.on("/api/netoff", HTTP_GET, handleApiNetOff);
        server.on("/api/esprst", HTTP_GET, handleApiEspRst);
        server.on("/api/update", HTTP_POST, handleApiUpdate, ModuleHttp::handleUpdate);
        server.onNotFound(serveStatic);
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
        server.begin();
        ModuleHttp::hasBegun = true;
    }
}

/**
 * get the latest measurement as JSON
 */
void ModuleHttp::handleApiLatest(AsyncWebServerRequest *request) {

    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");

    JsonDocument jsonDocument;
    jsonDocument["code"] = 200;

    values_all_t latestValue = Values::latest();

    jsonDocument[FIELD_NAME____TIME] = SensorTime::getDateTimeSecondsString(latestValue.secondstime);
    jsonDocument[FIELD_NAME___PM010] = latestValue.valuesPms.pm010;
    jsonDocument[FIELD_NAME___PM025] = latestValue.valuesPms.pm025;
    jsonDocument[FIELD_NAME___PM100] = latestValue.valuesPms.pm100;
    jsonDocument[FIELD_NAME_____DEG] = round(latestValue.valuesBme.deg * 10) / 10.0;
    jsonDocument[FIELD_NAME_____HUM] = round(latestValue.valuesBme.hum * 10) / 10.0;
    jsonDocument[FIELD_NAME_____HPA] = latestValue.valuesBme.pressure;

    int maxAge = SECONDS_PER___________MINUTE + 10 - SensorTime::getSecondstime() % SECONDS_PER___________MINUTE;  // 10 seconds extra to account for measure time being some seconds behind the full minute
    char maxAgeBuf[16];
    sprintf(maxAgeBuf, "max-age=%s", String(maxAge));
    response->addHeader("Cache-Control", maxAgeBuf);

    serializeJson(jsonDocument, *response);
    request->send(response);
}

/**
 * get the last hour of measurements as CSV
 */
void ModuleHttp::handleApiValCsv(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    ValcsvResponse *response = new ValcsvResponse();  // cache headers in ValcsvResponse
    request->send(response);
}

/**
 * get the last hour of measurements as binary data
 */
void ModuleHttp::handleApiValOut(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    ValoutResponse *response = new ValoutResponse();  // cache headers in ValoutResponse
    request->send(response);
}

/**
 * get the contents of a file as binary data
 */
void ModuleHttp::handleApiDatOut(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("file")) {
        String path = request->getParam("file")->value();
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        ModuleHttp::serveFile32(request, path);
        return;
    } else {
        ModuleHttp::serve400Json(request, "file required");
        return;
    }
}

/**
 * list the contents of a folder
 */
void ModuleHttp::handleApiDirOut(AsyncWebServerRequest *request) {

    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=10");

    JsonDocument jsonDocument;
    jsonDocument["code"] = 200;

    JsonArray foldersJa = jsonDocument["folders"].to<JsonArray>();
    JsonArray filesJa = jsonDocument["files"].to<JsonArray>();

    String path = "/";
    if (request->hasParam("folder")) {
        path = request->getParam("folder")->value();
    }
    if (!path.startsWith("/")) {
        path = "/" + path;
    }
    File folder = LittleFS.open(path.c_str(), FILE_READ);
    File file;

    Serial.printf("listing folder: %s\n", path.c_str());
    while ((file = folder.openNextFile(FILE_READ))) {

        Serial.println("got file");

        String name = String(file.name());

        // uint16_t pdate;
        // uint16_t ptime;
        // file.getModifyDateTime(&pdate, &ptime);

        // char lastModifiedBuffer[32];
        // sprintf(lastModifiedBuffer, "%d-%02d-%02d %02d:%02d:%02d", FS_YEAR(pdate), FS_MONTH(pdate), FS_DAY(pdate), FS_HOUR(ptime), FS_MINUTE(ptime), FS_SECOND(ptime));
        String lastModified = "";  // String(lastModifiedBuffer);

        if (file.isDirectory()) {
            JsonObject itemJo = foldersJa.add<JsonObject>();
            itemJo["folder"] = name;
            itemJo["last"] = lastModified;
        } else {
            JsonObject itemJo = filesJa.add<JsonObject>();
            itemJo["size"] = file.size();
            itemJo["file"] = name;
            itemJo["last"] = lastModified;
        }

        file.close();
    }
    folder.close();

    serializeJson(jsonDocument, *response);
    request->send(response);
}

/**
 * upload files to the device
 */
void ModuleHttp::handleApiUpload(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument jsonDocument;
    jsonDocument["code"] = ModuleHttp::uploadCode;
    ModuleHttp::uploadCode = -1;  // reset for next usage
    serializeJson(jsonDocument, *response);
    request->send(response);
}

/**
 * delete files from the device
 */
void ModuleHttp::handleApiDatDel(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("file")) {
        String path = "/" + request->getParam("file")->value();
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        if (ModuleCard::existsPath(path)) {

            bool success = ModuleCard::removeFile(path);

            AsyncResponseStream *response = request->beginResponseStream("application/json");
            JsonDocument jsonDocument;
            jsonDocument["code"] = success ? 200 : 500;
            jsonDocument["file"] = path;
            serializeJson(jsonDocument, *response);
            request->send(response);

        } else {
            ModuleHttp::serve404Json(request, path);
        }
    } else {
        ModuleHttp::serve400Json(request, "file required");
    }
}

/**
 * delete folders from the device
 */
void ModuleHttp::handleApiDirDel(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("folder")) {
        String path = "/" + request->getParam("folder")->value();
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        if (ModuleCard::existsPath(path)) {

            bool success = ModuleCard::removeFolder(path);

            AsyncResponseStream *response = request->beginResponseStream("application/json");
            JsonDocument jsonDocument;
            jsonDocument["code"] = success ? 200 : 500;
            jsonDocument["folder"] = path;
            serializeJson(jsonDocument, *response);
            request->send(response);

        } else {
            ModuleHttp::serve404Json(request, path);
        }
    } else {
        ModuleHttp::serve400Json(request, "folder required");
    }
}

/**
 * change device display
 */
void ModuleHttp::handleApiDspSet(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("p") && request->hasParam("v")) {
        String pRaw = request->getParam("p")->value();
        String vRaw = request->getParam("v")->value();
        if (ModuleHttp::isNumeric(pRaw) && ModuleHttp::isNumeric(vRaw)) {
            uint8_t p = pRaw.toInt();
            if (p >= 0 && p <= 5) {
                uint8_t v = vRaw.toInt();
                if (p == 0) {
                    if (v >= DISPLAY_VAL_T____010 && v <= DISPLAY_VAL_T____HPA) {
                        ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                            config.disp.displayValTable = (display_val_t_e)v;
                        };
                        serve200Json(request, p, v);
                        return;
                    } else {
                        serve400Json(request, "v must be between 0 and 2 for table value (p=2)");
                        return;
                    }
                } else {
                    serve400Json(request, "unhandled p value");
                    return;
                }
            } else {
                serve400Json(request, "p must be between 0 and 5");
                return;
            }
        } else {
            serve400Json(request, "p and v must be numeric");
            return;
        }
    } else {
        serve400Json(request, "p and v required");
        return;
    }
}

/**
 * get details about device status
 */
void ModuleHttp::handleApiStatus(AsyncWebServerRequest *request) {

    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");  // CHECK_MEASURE_INTERVAL
    JsonDocument jsonDocument;
    jsonDocument["code"] = 200;
    jsonDocument["vnum"] = VNUM;
    jsonDocument["boot"] = SensorTime::getDateTimeSecondsString(Device::secondstimeBoot);
    jsonDocument["heap"] = ESP.getFreeHeap();
    jsonDocument["sram"] = ESP.getPsramSize();
    jsonDocument["freq"] = ESP.getCpuFreqMHz();

#ifdef USE____DELAY
    jsonDocument["ddel"] = true;
#else
    jsonDocument["ddel"] = false;
#endif
#ifdef USE___SERIAL
    jsonDocument["dser"] = true;
#else
    jsonDocument["dser"] = false;
#endif
#ifdef USE_NEOPIXEL
    jsonDocument["dneo"] = true;
#else
    jsonDocument["dneo"] = false;
#endif
#ifdef USE_PERIODIC
    jsonDocument["dper"] = true;
#else
    jsonDocument["dper"] = false;
#endif

    JsonObject values = jsonDocument["values"].to<JsonObject>();
    values["nmsr"] = Values::values->nextMeasureIndex;
    values["mmsr"] = Values::values->nextMeasureIndex % MEASUREMENT_BUFFER_SIZE;
    values["ndsp"] = Values::values->nextDisplayIndex;
    values["nntp"] = Values::values->nextAutoNtpIndex;
    values["npub"] = Values::values->nextAutoPubIndex;

    JsonObject bme280Jo = jsonDocument["bme280"].to<JsonObject>();

    JsonObject mqttJo = jsonDocument["mqtt"].to<JsonObject>();
    mqttJo["stat"] = (int)Config::getMqttStatus();

    JsonObject timeJo = jsonDocument["time"].to<JsonObject>();
    timeJo["dutc"] = Config::getUtcOffsetSeconds();

    serializeJson(jsonDocument, *response);
    request->send(response);
}

/**
 * get a list of networks visible to the device
 */
void ModuleHttp::handleApiNetOut(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument jsonDocument;
    jsonDocument["code"] = 200;

    JsonArray networksJa = jsonDocument["networks"].to<JsonArray>();
    network_t network;
    for (int networkIndex = 0; networkIndex < NETWORKS_BUFFER_SIZE; networkIndex++) {
        network = ModuleWifi::discoveredNetworks[networkIndex];
        if (network.rssi > NETWORK_RSSI_INVALID) {
            JsonObject networkJo = networksJa.add<JsonObject>();
            networkJo["ssid"] = network.key;
            networkJo["rssi"] = network.rssi;
        }
    }
    serializeJson(jsonDocument, *response);
    request->send(response);
}

/**
 * disconnect the device
 */
void ModuleHttp::handleApiNetOff(AsyncWebServerRequest *request) {
    ModuleWifi::expire();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=10");
    JsonDocument jsonDocument;
    jsonDocument["code"] = 200;
    serializeJson(jsonDocument, *response);
    request->send(response);
}

/**
 * reset the device
 */
void ModuleHttp::handleApiEspRst(AsyncWebServerRequest *request) {

    ModuleWifi::expire();

    ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
        // this happens before wifi becomes a chance to disconnect
        // check for wifi to become disconnected, or ESP.restart() may not work
        ModuleWifi::depower(config);
        for (uint8_t i = 0; i < 5; i++) {
            delay(1000);
            if (!ModuleWifi::isPowered()) {
                ESP.restart();
            }
        }
    };

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=10");
    JsonDocument jsonDocument;
    jsonDocument["code"] = 200;
    serializeJson(jsonDocument, *response);
    request->send(response);
}

void ModuleHttp::handleApiUpdate(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    JsonDocument jsonDocument;
    jsonDocument["code"] = ModuleHttp::updateCode;
    ModuleHttp::updateCode = -1;  // reset for next usage
    serializeJson(jsonDocument, *response);
    request->send(response);
}

bool ModuleHttp::isNumeric(String value) {
    for (uint8_t i = 0; i < value.length(); i++) {
        if (!isDigit(value.charAt(i))) {
            return false;
        }
    }
    return true;
}

void ModuleHttp::serveStatic(AsyncWebServerRequest *request) {
    serveFile32(request, request->url());
}

void ModuleHttp::serveFile32(AsyncWebServerRequest *request, String path) {

    ModuleWifi::access();
    if (ModuleCard::existsPath(path)) {

        String mimeType = "text/plain";
        if (path.endsWith("html")) {
            mimeType = "text/html";
        } else if (path.endsWith("js")) {
            mimeType = "application/javascript";
        } else if (path.endsWith("css")) {
            mimeType = "text/css";
        } else if (path.endsWith("ttf")) {
            mimeType = "font/ttf";
        }
        File32Response *response = new File32Response(path, mimeType);
        if (request->hasHeader("If-Modified-Since")) {
            String ifModifiedSince = request->getHeader("If-Modified-Since")->value();
            if (!response->wasModifiedSince(ifModifiedSince)) {
                response->~File32Response();
                request->send(304);  // not-modified
                return;
            }
        }
        request->send(response);

    } else {
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            serve404Json(request, path);
        }
    }
}

void ModuleHttp::serve200Json(AsyncWebServerRequest *request, uint8_t p, uint8_t v) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");  // CHECK_MEASURE_INTERVAL
    JsonDocument jsonDocument;
    jsonDocument["code"] = 200;
    jsonDocument["p"] = p;
    jsonDocument["v"] = v;
    serializeJson(jsonDocument, *response);
    request->send(response);
}

void ModuleHttp::serve400Json(AsyncWebServerRequest *request, String description) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");  // CHECK_MEASURE_INTERVAL
    JsonDocument jsonDocument;
    jsonDocument["code"] = 400;
    jsonDocument["desc"] = description;
    serializeJson(jsonDocument, *response);
    request->send(response);
}

void ModuleHttp::serve404Json(AsyncWebServerRequest *request, String file) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");  // CHECK_MEASURE_INTERVAL
    JsonDocument jsonDocument;
    jsonDocument["code"] = 404;
    jsonDocument["file"] = file;
    jsonDocument["desc"] = "file not found";
    serializeJson(jsonDocument, *response);
    request->send(response);
}

void ModuleHttp::fillBufferWithCsv(values_all_t *value, uint8_t *data, uint16_t offset) {
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + value->secondstime);
    uint16_t pm010 = value->valuesPms.pm010;
    uint16_t pm025 = value->valuesPms.pm025;
    uint16_t pm100 = value->valuesPms.pm100;
    float deg = value->valuesBme.deg;
    float hum = value->valuesBme.hum;
    char csvBuffer[CSV_LINE_LENGTH + 1];
    sprintf(csvBuffer, CSV_FRMT.c_str(), date.year(), date.month(), date.day(), date.hour(), date.minute(), date.second(), min(MAX_4DIGIT_VALUE, pm010), min(MAX_4DIGIT_VALUE, pm025), min(MAX_4DIGIT_VALUE, pm100), deg, hum, value->valuesBme.pressure);
    for (uint8_t charIndex = 0; charIndex < CSV_LINE_LENGTH; charIndex++) {
        if (csvBuffer[charIndex] == '.') {
            data[offset + charIndex] = ',';
        } else {
            data[offset + charIndex] = csvBuffer[charIndex];
        }
    }
}

void ModuleHttp::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

    if (request->hasParam("file", true)) {
        String path = request->getParam("file", true)->value();
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        // delete file if exists
        if (index == 0) {
            if (ModuleCard::existsPath(path) && index == 0) {
                ModuleCard::removeFile(path);
            } else {
                int lastIndexOfSlash = path.lastIndexOf("/");
                if (lastIndexOfSlash > 0) {
                    String dataFilePath = path.substring(0, lastIndexOfSlash);
                    ModuleCard::buildFolders(dataFilePath);
                }
            }
        }

        Serial.println("opening file for append");

        File targetFile = LittleFS.open(path.c_str(), FILE_APPEND);
        for (size_t i = 0; i < len; i++) {
            targetFile.write(data[i]);
        }
        targetFile.flush();
        targetFile.close();

        Serial.println("done writing");

        if (path == MQTT_CONFIG_JSON || path == MQTT_CONFIG__CRT) {
            ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                ModuleMqtt::configure(config);  // deletes and rebuilds the mqtt dat
                values.nextAutoPubIndex = 0;    // resetting this value will trigger a republish attempt (and this rebuilds the mqtt dat)
            };
        } else if (path == WIFI_CONFIG_JSON) {
            ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                ModuleWifi::configure(config);  // deletes and rebuilds the wifi dat
                // changes become effective upon next reconnect
            };
        } else if (path == DISP_CONFIG_JSON) {
            ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                ModuleDisp::configure(config);  // reloads json and applies settings to config
                values.nextAutoNtpIndex = 0;    // trigger an ntp update (timezone may have changed)
            };
        }

        if (final) {
            ModuleHttp::uploadCode = 200;
        }

    } else {
        ModuleHttp::uploadCode = 400;
    }
}

void ModuleHttp::handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    // delete file if exists
    if (index == 0) {
        Update.begin();
    }

    Update.write(data, len);

    if (final) {
        if (Update.end(true)) {  // true to set the size to the current progress
            if (Update.isFinished()) {
                ModuleHttp::updateCode = 200;  // success
            } else {
                ModuleHttp::updateCode = 206;  // partial content
            }
        } else {
            ModuleHttp::updateCode = 205;  // reset content
        }
    }
}
