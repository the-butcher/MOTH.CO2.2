#ifndef ModuleDisp_h
#define ModuleDisp_h

#include <Adafruit_EPD.h>
#include <Adafruit_GFX.h>
#include <Arduino.h>

#include "ModuleDispBase.h"
#include "fonts/smb06pt_b.h"
#include "fonts/smb06pt_d.h"
#include "fonts/smb06pt_l.h"
#include "fonts/smb08pt_b.h"
#include "fonts/smb08pt_d.h"
#include "fonts/smb08pt_l.h"
#include "fonts/smb18pt_b.h"
#include "fonts/smb18pt_d.h"
#include "fonts/smb18pt_l.h"
#include "fonts/smb36pt_b.h"
#include "fonts/smb36pt_d.h"
#include "fonts/smb36pt_l.h"
#include "modules/ModuleWifi.h"
#include "sensors/SensorScd041.h"
#include "types/Config.h"
#include "types/Define.h"
#include "types/Device.h"
#include "types/Values.h"

typedef struct {
    uint16_t xmin;
    uint8_t ymin;
    uint16_t xmax;
    uint8_t ymax;
} rectangle_t;

const String DISP_CONFIG_JSON = "/config/disp.json";

const String SYMBOL__WIFI = "¥";
const String SYMBOL_THEME = "¤";
const String SYMBOL_TABLE = "£";
const String SYMBOL_CHART = "¢";
const String SYMBOL_YBEEP = "©";
const String SYMBOL_NBEEP = "ª";

class ModuleDisp {
   private:
    static ModuleDispBase baseDisplay;
    static bool interrupted;
    static void handleInterrupt();
    static void renderHeader();
    static void renderButton(button_action_t buttonAction, uint16_t x);
    static void renderFooter(config_t& config);
    static void drawAntialiasedText06(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color);
    static void drawAntialiasedText08(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color);
    static void drawAntialiasedText18(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color);
    static void drawAntialiasedText36(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color);
    static void drawAntialiasedText(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color, const GFXfont* fontL, const GFXfont* fontD, const GFXfont* fontB);
    static void clearBuffer(config_t& config);
    static void flushBuffer();
    static void drawOuterBorders(uint16_t color);
    static void drawInnerBorders(uint16_t color);
    static void fillRectangle(rectangle_t rectangle, uint8_t color);
    static bool isWarn(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi);
    static bool isRisk(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi);
    static uint8_t getTextColor(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi);
    static uint8_t getFillColor(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi);
    static uint8_t getVertColor(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi);
    static String formatString(String value, char const* format);

   public:
    static void configure(config_t& config);  // loads json configuration
    static void begin();
    static void prepareSleep(wakeup_action_e wakeupType);
    static void attachWakeup(wakeup_action_e wakeupType);
    static void detachWakeup(wakeup_action_e wakeupType);
    static void renderTable(values_all_t& measurement, config_t& config);
    static void renderChart(values_all_t history[60], config_t& config);
    static void renderCo2Cal(co2cal______t co2cal, config_t& config);
    static void renderQRCodes(config_t& config);
    static void renderEntry(config_t& config);
    // static void renderCo2(config_t& config, calibration_t calibration);
    static void depower();
    static bool isInterrupted();
};

#endif