#include "ModuleServer.h"

#include <ArduinoJson.h>
#include <Update.h>

#include "DatCsvResponse.h"
#include "File32Response.h"
#include "ValuesResponse.h"
#include "modules/ModuleMqtt.h"
#include "modules/ModuleSdcard.h"
#include "modules/ModuleWifi.h"
#include "types/Define.h"
#include "types/Device.h"
#include "types/Values.h"

AsyncWebServer ModuleServer::server(80);
bool ModuleServer::hasBegun = false;
// uint16_t ModuleServer::requestedCo2Ref = 0;
// bool ModuleServer::requestedCo2Rst = false;
std::function<void(config_t &config, values_t &values)> ModuleServer::requestedReconfiguration = nullptr;
int ModuleServer::updateCode = -1;
int ModuleServer::uploadCode = -1;

void ModuleServer::begin() {
    if (!ModuleServer::hasBegun) {
        server.on("/api/latest", HTTP_GET, handleApiLatest);
        server.on("/api/valcsv", HTTP_GET, handleApiValCsv);  // get csv from data measured in the last hour
        server.on("/api/datcsv", HTTP_GET, handleApiDatCsv);  // get csv from data stored in dat files
        server.on("/api/status", HTTP_GET, handleApiStatus);
        server.on("/api/dspset", HTTP_GET, handleApiDspSet);
        server.on("/api/dirout", HTTP_GET, handleApiDirOut);
        server.on("/api/datout", HTTP_GET, handleApiDatOut);
        server.on("/api/upload", HTTP_POST, handleApiUpload, ModuleServer::handleUpload);
        server.on("/api/dirdel", HTTP_GET, handleApiDirDel);
        server.on("/api/datdel", HTTP_GET, handleApiDatDel);
        server.on("/api/netout", HTTP_GET, handleApiNetOut);
        server.on("/api/netoff", HTTP_GET, handleApiNetOff);
        server.on("/api/co2cal", HTTP_GET, handleApiCo2Cal);
        server.on("/api/co2rst", HTTP_GET, handleApiCo2Rst);
        server.on("/api/esprst", HTTP_GET, handleApiEspRst);
        server.on("/api/update", HTTP_POST, handleApiUpdate, ModuleServer::handleUpdate);
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

void ModuleServer::handleApiValCsv(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    ValuesResponse *response = new ValuesResponse();  // cache headers in ValuesResponse
    request->send(response);
}

void ModuleServer::handleApiDatCsv(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("file")) {
        String datFileName = "/" + request->getParam("file")->value();
        String csvLine;
        if (ModuleSdcard::existsPath(datFileName)) {
            DatCsvResponse *response = new DatCsvResponse(datFileName);  // cache headers in ValuesResponse
            if (request->hasHeader("If-Modified-Since")) {
                String ifModifiedSince = request->getHeader("If-Modified-Since")->value();
                if (!response->wasModifiedSince(ifModifiedSince)) {
                    response->~DatCsvResponse();
                    request->send(304);  // not-modified
                    return;
                }
            }
            request->send(response);
        } else {
            serve404Json(request, datFileName);
        }
    } else {
        serve400Json(request, "file required");
    }
}

void ModuleServer::handleApiStatus(AsyncWebServerRequest *request) {

    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;
    root["vnum"] = VNUM;
    root["boot"] = SensorTime::getDateTimeSecondsString(Device::secondstimeBoot);
    root["heap"] = ESP.getFreeHeap();
    root["sram"] = ESP.getPsramSize();
    root["freq"] = ESP.getCpuFreqMHz();

    JsonObject &scd041Jo = root.createNestedObject("scd041");
    // scd041Jo["co2r"] = SensorScd041::getCo2Reference(); // TODO :: decide whether and how to expose config to ModuleServer
    // scd041Jo["toff"] = SensorScd041::getTemperatureOffset();
    // scd041Jo["iasc"] = SensorScd041::isAutomaticSelfCalibration();
    // scd041Jo["calt"] = SensorScd041::getCompensationAltitude();

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

void ModuleServer::handleApiDspSet(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("p") && request->hasParam("v")) {
        String pRaw = request->getParam("p")->value();
        String vRaw = request->getParam("v")->value();
        if (ModuleServer::isNumeric(pRaw) && ModuleServer::isNumeric(vRaw)) {
            uint8_t p = pRaw.toInt();
            if (p >= 0 && p <= 5) {
                uint8_t v = vRaw.toInt();
                if (p == 0) {
                    if (v >= DISPLAY_VAL_M__TABLE && v <= DISPLAY_VAL_M__CHART) {
                        ModuleServer::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                            config.disp.displayValModus = (display_val_m_e)v;
                        };
                        serve200Json(request, p, v);
                        return;
                    } else {
                        serve400Json(request, "v must be between 0 and 1 for modus (p=0)");
                        return;
                    }
                } else if (p == 1) {
                    if (v >= DISPLAY_THM____LIGHT && v <= DISPLAY_THM_____DARK) {
                        ModuleServer::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                            config.disp.displayValTheme = (display_val_e_e)v;
                        };
                        serve200Json(request, p, v);
                        return;
                    } else {
                        serve400Json(request, "v must be between 0 and 1 for theme (p=1)");
                        return;
                    }
                } else if (p == 2) {
                    if (v >= DISPLAY_VAL_T____CO2 && v <= DISPLAY_VAL_T____ALT) {
                        ModuleServer::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                            config.disp.displayValTable = (display_val_t_e)v;
                        };
                        serve200Json(request, p, v);
                        return;
                    } else {
                        serve400Json(request, "v must be between 0 and 2 for table value (p=2)");
                        return;
                    }
                } else if (p == 3) {
                    if (v >= DISPLAY_VAL_C____CO2 && v <= DISPLAY_VAL_C____NRG) {
                        ModuleServer::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                            config.disp.displayValChart = (display_val_c_e)v;
                        };
                        serve200Json(request, p, v);
                        return;
                    } else {
                        serve400Json(request, "v must be between 0 and 5 for chart value (p=3)");
                        return;
                    }
                } else if (p == 4) {
                    if (v == DISPLAY_HRS_C_____01 || v == DISPLAY_HRS_C_____03 || v == DISPLAY_HRS_C_____06 || v == DISPLAY_HRS_C_____12 || v == DISPLAY_HRS_C_____24) {
                        ModuleServer::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                            config.disp.displayHrsChart = (display_val_h_e)v;
                        };
                        serve200Json(request, p, v);
                        return;
                    } else {
                        serve400Json(request, "v must be one of 1,3,6,12,24 for chart hours (p=4)");
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

bool ModuleServer::isNumeric(String value) {
    for (uint8_t i = 0; i < value.length(); i++) {
        if (!isDigit(value.charAt(i))) {
            return false;
        }
    }
    return true;
}

void ModuleServer::handleApiDirOut(AsyncWebServerRequest *request) {

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
            file.getModifyDateTime(&pdate, &ptime);

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

void ModuleServer::handleApiDatOut(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("file")) {
        ModuleServer::serveFile32(request, request->getParam("file")->value());
    } else {
        ModuleServer::serve400Json(request, "file required");
    }
}

void ModuleServer::handleApiUpload(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = ModuleServer::uploadCode;
    ModuleServer::uploadCode = -1;  // reset for next usage
    root.printTo(*response);
    request->send(response);
}

void ModuleServer::handleApiEspRst(AsyncWebServerRequest *request) {
    ESP.restart();
}

void ModuleServer::handleApiUpdate(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = ModuleServer::updateCode;
    ModuleServer::updateCode = -1;  // reset for next usage
    root.printTo(*response);
    request->send(response);
}

void ModuleServer::handleApiDirDel(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("folder")) {
        String path = "/" + request->getParam("folder")->value();
        if (ModuleSdcard::existsPath(path)) {

            bool success = ModuleSdcard::removeFolder(path);

            AsyncResponseStream *response = request->beginResponseStream("application/json");
            DynamicJsonBuffer jsonBuffer;
            JsonObject &root = jsonBuffer.createObject();
            root["code"] = success ? 200 : 500;
            root["folder"] = path;
            root.printTo(*response);
            request->send(response);

        } else {
            ModuleServer::serve404Json(request, path);
        }
    } else {
        ModuleServer::serve400Json(request, "folder required");
    }
}

void ModuleServer::handleApiDatDel(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("file")) {
        String path = "/" + request->getParam("file")->value();
        if (ModuleSdcard::existsPath(path)) {

            bool success = ModuleSdcard::removeFile32(path);

            AsyncResponseStream *response = request->beginResponseStream("application/json");
            DynamicJsonBuffer jsonBuffer;
            JsonObject &root = jsonBuffer.createObject();
            root["code"] = success ? 200 : 500;
            root["file"] = path;
            root.printTo(*response);
            request->send(response);

        } else {
            ModuleServer::serve404Json(request, path);
        }
    } else {
        ModuleServer::serve400Json(request, "file required");
    }
}

void ModuleServer::handleApiNetOut(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;

    JsonArray &networksJa = root.createNestedArray("networks");
    network_t network;
    for (int networkIndex = 0; networkIndex < 10; networkIndex++) {
        network = ModuleWifi::discoveredNetworks[networkIndex];
        if (network.rssi != 0) {
            JsonObject &networkJo = networksJa.createNestedObject();
            networkJo["ssid"] = network.key;
            networkJo["rssi"] = network.rssi;
        }
    }

    root.printTo(*response);
    request->send(response);
}

void ModuleServer::handleApiNetOff(AsyncWebServerRequest *request) {
    ModuleWifi::expire();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=10");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;
    root.printTo(*response);
    request->send(response);
}

void ModuleServer::handleApiCo2Cal(AsyncWebServerRequest *request) {

    ModuleWifi::access();
    if (request->hasParam("ref")) {
        String refRaw = request->getParam("ref")->value();
        if (ModuleServer::isNumeric(refRaw)) {
            int ref = refRaw.toInt();
            if (ref >= 400) {

                ModuleServer::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                    config.sco2.requestedCo2Ref = ref;
                };

                AsyncResponseStream *response = request->beginResponseStream("application/json");
                DynamicJsonBuffer jsonBuffer;
                JsonObject &root = jsonBuffer.createObject();
                root["code"] = 200;
                root["ref"] = ref;
                response->addHeader("Cache-Control", "max-age=180");

                root.printTo(*response);
                request->send(response);

            } else {
                serve400Json(request, "ref must be >= 400");
                return;
            }
        } else {
            serve400Json(request, "ref must be numeric");
            return;
        }

    } else {
        serve400Json(request, "ref must be specified");
        return;
    }
}

void ModuleServer::handleApiCo2Rst(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=180");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;

    ModuleServer::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
        config.sco2.requestedCo2Rst = true;
    };

    root.printTo(*response);
    request->send(response);
}

void ModuleServer::serveStatic(AsyncWebServerRequest *request) {
    serveFile32(request, request->url());
}

void ModuleServer::serveFile32(AsyncWebServerRequest *request, String file) {

    ModuleWifi::access();
    if (ModuleSdcard::existsPath(file)) {

        String fileType = file.substring(file.indexOf("."));
        if (fileType == ".html") {
            fileType = "text/html";
        } else if (fileType == ".js") {
            fileType = "application/javascript";
        } else if (fileType == ".css") {
            fileType = "text/css";
        } else if (fileType == ".ttf") {
            fileType = "font/ttf";
        }
        File32Response *response = new File32Response(file, fileType);
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
            serve404Json(request, file);
        }
    }
}

void ModuleServer::serve200Json(AsyncWebServerRequest *request, uint8_t p, uint8_t v) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;
    root["p"] = p;
    root["v"] = v;
    root.printTo(*response);
    request->send(response);
}

void ModuleServer::serve400Json(AsyncWebServerRequest *request, String description) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 400;
    root["desc"] = description;
    root.printTo(*response);
    request->send(response);
}

void ModuleServer::serve404Json(AsyncWebServerRequest *request, String file) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 404;
    root["file"] = file;
    root["desc"] = "file not found";
    root.printTo(*response);
    request->send(response);
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

void ModuleServer::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (request->hasParam("file", true)) {
        String dataFileName = "/" + request->getParam("file", true)->value();

        // delete file if exists
        if (index == 0) {
            if (ModuleSdcard::existsPath(dataFileName) && index == 0) {
                ModuleSdcard::removeFile32(dataFileName);
            } else {
                int lastIndexOfSlash = dataFileName.lastIndexOf("/");
                if (lastIndexOfSlash > 0) {
                    String dataFilePath = dataFileName.substring(0, lastIndexOfSlash);
                    ModuleSdcard::buildFolders(dataFilePath);
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

        // if MQTT json was uploaded, the dat file is deleted to force rebuild
        if (dataFileName == MQTT_CONFIG_JSON) {
            ModuleSdcard::removeFile32(MQTT_CONFIG__DAT);
            ModuleServer::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                values.nextAutoPubIndex = 0;  // force republish attempt
            };
        }

        // if (dataFileName == BoxEncr::CONFIG_PATH) {
        //     BoxEncr::updateConfiguration();
        //     BoxConn::updateConfiguration();  // may have changed encryption
        //     BoxMqtt::updateConfiguration();  // may have changed encryption
        // } else if (dataFileName == BoxDisplay::CONFIG_PATH) {
        //     BoxDisplay::updateConfiguration();
        // } else if (dataFileName == BoxConn::CONFIG_PATH) {
        //     BoxConn::updateConfiguration();
        // } else if (dataFileName == BoxMqtt::CONFIG_PATH) {
        //     BoxMqtt::updateConfiguration();
        // }

        if (final) {
            ModuleServer::uploadCode = 200;
        }

    } else {
        ModuleServer::uploadCode = 400;
    }
}

void ModuleServer::handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    // delete file if exists
    if (index == 0) {
        Update.begin();
    }

    Update.write(data, len);

    if (final) {
        if (Update.end(true)) {  // true to set the size to the current progress
            if (Update.isFinished()) {
                ModuleServer::updateCode = 200;  // success
            } else {
                ModuleServer::updateCode = 206;  // partial content
            }
        } else {
            ModuleServer::updateCode = 205;  // reset content
        }
    }
}
