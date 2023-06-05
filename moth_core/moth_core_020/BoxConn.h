#ifndef BoxConn_h
#define BoxConn_h

#include <Arduino.h>
#include "ESPAsyncWebServer.h"
#include "Network.h"
#include "WiFi.h"
#include "BoxStatus.h"

class BoxConn {

private:
  static void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

public:
  static String VNUM;
  static String CONFIG_PATH;
  static config_status_t configStatus;
  static void handleStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
  static int requestedCalibrationReference;
  static bool isHibernationRequired;
  static bool isCo2CalibrationReset;
  static bool isRenderStateRequired;
  static wifi_mode_t getMode();
  static void begin();
  static void on();
  static void off();
  static String getRootUrl();
  static String getAddress();
  static String getNetworkName();
  static String getNetworkPass();
  static int wifiButtonCount;
  static bool isExpireable();
  static bool checkNumeric(String value);
  static void updateConfiguration();
  static String formatConfigStatus(config_status_t configStatus);

};

#endif