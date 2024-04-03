#ifndef ModuleServer_h
#define ModuleServer_h

#include <ESPAsyncWebServer.h>

#include "types/Values.h"

const String CSV_HEAD = "time;co2_lpf;co2_raw;temperature;humidity;pressure;battery\r\n";
const String CSV_FRMT = "%04d-%02d-%02d %02d:%02d:%02d;%4d;%4d;%5.1f;%4.1f;%7.2f;%6.2f\r\n";
const uint8_t CSV_LINE_LENGTH = 57;

class ModuleServer {
   private:
    static AsyncWebServer server;
    static bool hasBegun;

   public:
    static uint16_t requestedCo2Ref;
    static bool requestedCo2Rst;
    static void begin();
    static void handleApiLatest(AsyncWebServerRequest *request);
    static void handleApiStatus(AsyncWebServerRequest *request);
    static void handleApiFolder(AsyncWebServerRequest *request);
    static void handleApiDatCsv(AsyncWebServerRequest *request);
    static void handleApiValCsv(AsyncWebServerRequest *request);
    static void handleApiCo2Cal(AsyncWebServerRequest *request);
    static void handleApiCo2Rst(AsyncWebServerRequest *request);
    static void serveStatic(AsyncWebServerRequest *request);
    static void fillBufferWithCsv(values_all_t *value, uint8_t *data, uint16_t offset);
};

#endif