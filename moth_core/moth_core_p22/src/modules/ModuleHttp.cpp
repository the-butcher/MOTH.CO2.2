#include "ModuleHttp.h"

#include <ArduinoJson.h>
#include <Update.h>

#include "DatCsvResponse.h"
#include "File32Response.h"
#include "ValuesResponse.h"
#include "modules/ModuleCard.h"
#include "modules/ModuleDisp.h"
#include "modules/ModuleMqtt.h"
#include "modules/ModuleWifi.h"
#include "types/Define.h"
#include "types/Device.h"
#include "types/Values.h"

AsyncWebServer ModuleHttp::server(80);
bool ModuleHttp::hasBegun = false;
// uint16_t ModuleHttp::requestedCo2Ref = 0;
// bool ModuleHttp::requestedCo2Rst = false;
std::function<void(config_t &config, values_t &values)> ModuleHttp::requestedReconfiguration = nullptr;
int ModuleHttp::updateCode = -1;
int ModuleHttp::uploadCode = -1;

void ModuleHttp::begin() {
    if (!ModuleHttp::hasBegun) {
        server.on("/api/latest", HTTP_GET, handleApiLatest);
        server.on("/api/valcsv", HTTP_GET, handleApiValCsv);  // get csv from data measured in the last hour
        server.on("/api/datcsv", HTTP_GET, handleApiDatCsv);  // get csv from data stored in dat files
        server.on("/api/status", HTTP_GET, handleApiStatus);
        server.on("/api/dspset", HTTP_GET, handleApiDspSet);
        server.on("/api/dirout", HTTP_GET, handleApiDirOut);
        server.on("/api/datout", HTTP_GET, handleApiDatOut);
        server.on("/api/upload", HTTP_POST, handleApiUpload, ModuleHttp::handleUpload);
        server.on("/api/dirdel", HTTP_GET, handleApiDirDel);
        server.on("/api/datdel", HTTP_GET, handleApiDatDel);
        server.on("/api/netout", HTTP_GET, handleApiNetOut);
        server.on("/api/netoff", HTTP_GET, handleApiNetOff);
        server.on("/api/co2cal", HTTP_GET, handleApiCo2Cal);
        server.on("/api/co2rst", HTTP_GET, handleApiCo2Rst);
        server.on("/api/co2tst", HTTP_GET, handleApiCo2Tst);
        server.on("/api/esprst", HTTP_GET, handleApiEspRst);
        server.on("/api/update", HTTP_POST, handleApiUpdate, ModuleHttp::handleUpdate);
        server.onNotFound(serveStatic);
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
        server.begin();
        ModuleHttp::hasBegun = true;
    }
}

void ModuleHttp::handleApiLatest(AsyncWebServerRequest *request) {

    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;

    values_all_t latestValue = Values::values->measurements[(Values::values->nextMeasureIndex - 1) % MEASUREMENT_BUFFER_SIZE];

    root[FIELD_NAME____TIME] = SensorTime::getDateTimeSecondsString(latestValue.secondstime);
    root[FIELD_NAME_CO2_LPF] = (uint16_t)round(latestValue.valuesCo2.co2Lpf / VALUE_SCALE_CO2LPF);
    root[FIELD_NAME_CO2_RAW] = latestValue.valuesCo2.co2Raw;
    root[FIELD_NAME_____DEG] = round(SensorScd041::toFloatDeg(latestValue.valuesCo2.deg) * 10) / 10.0;
    root[FIELD_NAME_____HUM] = round(SensorScd041::toFloatHum(latestValue.valuesCo2.hum) * 10) / 10.0;
    root[FIELD_NAME_____HPA] = latestValue.valuesBme.pressure;
    root[FIELD_NAME_____BAT] = SensorEnergy::toFloatPercent(latestValue.valuesNrg.percent);

    int maxAge = SECONDS_PER___________MINUTE + 10 - SensorTime::getSecondstime() % SECONDS_PER___________MINUTE;  // 10 seconds extra to account for measure time being some seconds behind the full minute
    char maxAgeBuf[16];
    sprintf(maxAgeBuf, "max-age=%s", String(maxAge));
    response->addHeader("Cache-Control", maxAgeBuf);

    root.printTo(*response);
    request->send(response);
}

void ModuleHttp::handleApiValCsv(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    ValuesResponse *response = new ValuesResponse();  // cache headers in ValuesResponse
    request->send(response);
}

void ModuleHttp::handleApiDatCsv(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("file")) {
        String datFileName = "/" + request->getParam("file")->value();
        String csvLine;
        if (ModuleCard::existsPath(datFileName)) {
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

void ModuleHttp::handleApiStatus(AsyncWebServerRequest *request) {

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
    scd041Jo["toff"] = SensorScd041::getTemperatureOffset();
    scd041Jo["iasc"] = SensorScd041::isAutomaticSelfCalibration();
    scd041Jo["calt"] = SensorScd041::getCompensationAltitude();

    // mqtt attaches status to config. either config or just the status should be available here

    // JsonObject &wifiJo = root.createNestedObject("wifi");
    // wifiJo["config"] = BoxConn::formatConfigStatus(BoxConn::configStatus);

    // JsonObject &dispJo = root.createNestedObject("disp");
    // dispJo["config"] = BoxConn::formatConfigStatus(BoxDisplay::configStatus);

    JsonObject &mqttJo = root.createNestedObject("mqtt");
    mqttJo["stat"] = (int)Config::getMqttStatus();
    // mqttJo["config"] = BoxConn::formatConfigStatus(BoxMqtt::configStatus);
    // mqttJo["active"] = BoxMqtt::isConfiguredToBeActive();
    // mqttJo["pcount"] = Measurements::getPublishableCount();

    root.printTo(*response);
    request->send(response);
}

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
                    if (v >= DISPLAY_VAL_M__TABLE && v <= DISPLAY_VAL_M__CHART) {
                        ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
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
                        ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
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
                        ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
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
                        ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
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
                        ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
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

bool ModuleHttp::isNumeric(String value) {
    for (uint8_t i = 0; i < value.length(); i++) {
        if (!isDigit(value.charAt(i))) {
            return false;
        }
    }
    return true;
}

void ModuleHttp::handleApiDirOut(AsyncWebServerRequest *request) {

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

void ModuleHttp::handleApiDatOut(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("file")) {
        ModuleHttp::serveFile32(request, request->getParam("file")->value());
    } else {
        ModuleHttp::serve400Json(request, "file required");
    }
}

void ModuleHttp::handleApiUpload(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = ModuleHttp::uploadCode;
    ModuleHttp::uploadCode = -1;  // reset for next usage
    root.printTo(*response);
    request->send(response);
}

void ModuleHttp::handleApiEspRst(AsyncWebServerRequest *request) {
    ESP.restart();
}

void ModuleHttp::handleApiUpdate(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = ModuleHttp::updateCode;
    ModuleHttp::updateCode = -1;  // reset for next usage
    root.printTo(*response);
    request->send(response);
}

void ModuleHttp::handleApiDirDel(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("folder")) {
        String path = "/" + request->getParam("folder")->value();
        if (ModuleCard::existsPath(path)) {

            bool success = ModuleCard::removeFolder(path);

            AsyncResponseStream *response = request->beginResponseStream("application/json");
            DynamicJsonBuffer jsonBuffer;
            JsonObject &root = jsonBuffer.createObject();
            root["code"] = success ? 200 : 500;
            root["folder"] = path;
            root.printTo(*response);
            request->send(response);

        } else {
            ModuleHttp::serve404Json(request, path);
        }
    } else {
        ModuleHttp::serve400Json(request, "folder required");
    }
}

void ModuleHttp::handleApiDatDel(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    if (request->hasParam("file")) {
        String path = "/" + request->getParam("file")->value();
        if (ModuleCard::existsPath(path)) {

            bool success = ModuleCard::removeFile32(path);

            AsyncResponseStream *response = request->beginResponseStream("application/json");
            DynamicJsonBuffer jsonBuffer;
            JsonObject &root = jsonBuffer.createObject();
            root["code"] = success ? 200 : 500;
            root["file"] = path;
            root.printTo(*response);
            request->send(response);

        } else {
            ModuleHttp::serve404Json(request, path);
        }
    } else {
        ModuleHttp::serve400Json(request, "file required");
    }
}

void ModuleHttp::handleApiNetOut(AsyncWebServerRequest *request) {
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

void ModuleHttp::handleApiNetOff(AsyncWebServerRequest *request) {
    ModuleWifi::expire();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=10");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;
    root.printTo(*response);
    request->send(response);
}

void ModuleHttp::handleApiCo2Cal(AsyncWebServerRequest *request) {

    ModuleWifi::access();
    if (request->hasParam("ref")) {
        String refRaw = request->getParam("ref")->value();
        if (ModuleHttp::isNumeric(refRaw)) {
            int ref = refRaw.toInt();
            if (ref >= 400) {

                ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
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

void ModuleHttp::handleApiCo2Rst(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=180");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;

    ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
        config.sco2.requestedCo2Rst = true;
    };

    root.printTo(*response);
    request->send(response);
}

void ModuleHttp::handleApiCo2Tst(AsyncWebServerRequest *request) {
    ModuleWifi::access();
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=180");

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 200;

    ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
        config.sco2.requestedCo2Tst = true;
    };

    root.printTo(*response);
    request->send(response);
}

void ModuleHttp::serveStatic(AsyncWebServerRequest *request) {
    serveFile32(request, request->url());
}

void ModuleHttp::serveFile32(AsyncWebServerRequest *request, String file) {

    ModuleWifi::access();
    if (ModuleCard::existsPath(file)) {

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

void ModuleHttp::serve200Json(AsyncWebServerRequest *request, uint8_t p, uint8_t v) {
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

void ModuleHttp::serve400Json(AsyncWebServerRequest *request, String description) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "max-age=60");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["code"] = 400;
    root["desc"] = description;
    root.printTo(*response);
    request->send(response);
}

void ModuleHttp::serve404Json(AsyncWebServerRequest *request, String file) {
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

void ModuleHttp::fillBufferWithCsv(values_all_t *value, uint8_t *data, uint16_t offset) {
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + value->secondstime);
    uint16_t co2 = (uint16_t)round(value->valuesCo2.co2Lpf / VALUE_SCALE_CO2LPF);
    float deg = SensorScd041::toFloatDeg(value->valuesCo2.deg);
    float hum = SensorScd041::toFloatHum(value->valuesCo2.hum);
    float nrg = SensorEnergy::toFloatPercent(value->valuesNrg.percent);
    char csvBuffer[CSV_LINE_LENGTH + 1];
    sprintf(csvBuffer, CSV_FRMT.c_str(), date.year(), date.month(), date.day(), date.hour(), date.minute(), date.second(), min(MAX_4DIGIT_VALUE, co2), min(MAX_4DIGIT_VALUE, value->valuesCo2.co2Raw), deg, hum, value->valuesBme.pressure, nrg);
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
        String dataFileName = "/" + request->getParam("file", true)->value();

        // delete file if exists
        if (index == 0) {
            if (ModuleCard::existsPath(dataFileName) && index == 0) {
                ModuleCard::removeFile32(dataFileName);
            } else {
                int lastIndexOfSlash = dataFileName.lastIndexOf("/");
                if (lastIndexOfSlash > 0) {
                    String dataFilePath = dataFileName.substring(0, lastIndexOfSlash);
                    ModuleCard::buildFolders(dataFilePath);
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

        if (dataFileName == MQTT_CONFIG_JSON) {
            ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                ModuleMqtt::configure(config);  // deletes and rebuilds the mqtt dat
                values.nextAutoPubIndex = 0;    // resetting this value will trigger a republish attempt (and this rebuilds the mqtt dat)
            };
        } else if (dataFileName == WIFI_CONFIG_JSON) {
            ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                ModuleWifi::configure(config);  // deletes and rebuilds the wifi dat
                // changes become effective upon next reconnect
            };
        } else if (dataFileName == DISP_CONFIG_JSON) {
            ModuleHttp::requestedReconfiguration = [=](config_t &config, values_t &values) -> void {
                ModuleDisp::configure(config);    // reloads json and applies settings to config
                SensorScd041::configure(config);  // will apply temperature offset (if different from current value)
                values.nextAutoNtpIndex = 0;      // trigger an ntp update (timezone may have changed)
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
