#ifndef ModuleHttp_h
#define ModuleHttp_h

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "types/Config.h"
#include "types/Values.h"

const String SEMICOLON = ";";
const String CSV_HEAD = FIELD_NAME____TIME + SEMICOLON + FIELD_NAME_CO2_LPF + SEMICOLON + FIELD_NAME_CO2_RAW + SEMICOLON + FIELD_NAME_____DEG + SEMICOLON + FIELD_NAME_____HUM + SEMICOLON + FIELD_NAME_____HPA + SEMICOLON + FIELD_NAME_____BAT + "\r\n";
const String CSV_FRMT = "%04d-%02d-%02d %02d:%02d:%02d;%4d;%4d;%5.1f;%4.1f;%7.2f;%6.2f\r\n";
const uint8_t CSV_LINE_LENGTH = 57;
const uint16_t MAX_4DIGIT_VALUE = 0x1FFF;  // 8191

class ModuleHttp {
   private:
    static AsyncWebServer server;
    static bool hasBegun;
    static bool isNumeric(String value);
    static void serveFile32(AsyncWebServerRequest *request, String file);
    static void serve200Json(AsyncWebServerRequest *request, uint8_t p, uint8_t v);
    static void serve400Json(AsyncWebServerRequest *request, String file);
    static void serve404Json(AsyncWebServerRequest *request, String file);
    static void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    static void handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

    static int updateCode;
    static int uploadCode;

   public:
    static std::function<void(config_t &config, values_t &values)> requestedReconfiguration;
    static void begin();
    static void handleApiLatest(AsyncWebServerRequest *request);
    static void handleApiValCsv(AsyncWebServerRequest *request);
    static void handleApiDatCsv(AsyncWebServerRequest *request);
    static void handleApiStatus(AsyncWebServerRequest *request);
    static void handleApiDspSet(AsyncWebServerRequest *request);
    static void handleApiDirOut(AsyncWebServerRequest *request);
    static void handleApiDatOut(AsyncWebServerRequest *request);
    static void handleApiUpload(AsyncWebServerRequest *request);
    static void handleApiDirDel(AsyncWebServerRequest *request);
    static void handleApiDatDel(AsyncWebServerRequest *request);
    static void handleApiNetOut(AsyncWebServerRequest *request);
    static void handleApiNetOff(AsyncWebServerRequest *request);
    static void handleApiCo2Cal(AsyncWebServerRequest *request);
    static void handleApiCo2Rst(AsyncWebServerRequest *request);
    static void handleApiCo2Tst(AsyncWebServerRequest *request);
    static void handleApiUpdate(AsyncWebServerRequest *request);
    static void handleApiEspRst(AsyncWebServerRequest *request);
    static void serveStatic(AsyncWebServerRequest *request);
    static void fillBufferWithCsv(values_all_t *value, uint8_t *data, uint16_t offset);
};

#endif