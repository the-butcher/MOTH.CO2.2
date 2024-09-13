#ifndef ModuleDisp_h
#define ModuleDisp_h

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>

#include "fonts/smb36pt.h"
#include "modules/ModuleWifi.h"
#include "types/Action.h"
#include "types/Config.h"
#include "types/Values.h"

const String DISP_CONFIG_JSON = "/config/disp.json";

const String SYMBOL__WIFI = "W";
const String SYMBOL_YBEEP = "Y";
const String SYMBOL_NBEEP = "N";

class ModuleDisp {
   private:
    static Adafruit_SH1107 display;
    static void renderHeader();
    static void renderFooter(config_t& config);
    static void renderButton(button_action_t buttonAction, uint16_t y);
    static String formatString(String value, char const* format);

   public:
    static void configure(config_t& config);  // loads json configuration
    static void begin();
    static void renderEntry(config_t& config);
    static void renderTable(values_all_t& measurement, config_t& config);
};

#endif