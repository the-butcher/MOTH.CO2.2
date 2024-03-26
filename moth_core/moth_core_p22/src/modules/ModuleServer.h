#ifndef ModuleServer_h
#define ModuleServer_h

#include <ESPAsyncWebServer.h>

class ModuleServer {
   private:
    static AsyncWebServer server;
    static bool hasBegun;

   public:
    static uint16_t requestedCalibrationReference;
    static void begin();
    static void handleApiStatus(AsyncWebServerRequest *request);
    static void handleCalibrate(AsyncWebServerRequest *request);
    static void serveStatic(AsyncWebServerRequest *request);
};

#endif