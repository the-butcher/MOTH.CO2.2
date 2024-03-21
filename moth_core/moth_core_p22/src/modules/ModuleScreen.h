#ifndef ModuleScreen_h
#define ModuleScreen_h

#include <Adafruit_EPD.h>
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <Fonts/smb06pt_b.h>
#include <Fonts/smb06pt_d.h>
#include <Fonts/smb06pt_l.h>
#include <Fonts/smb08pt_b.h>
#include <Fonts/smb08pt_d.h>
#include <Fonts/smb08pt_l.h>
#include <Fonts/smb18pt_b.h>
#include <Fonts/smb18pt_d.h>
#include <Fonts/smb18pt_l.h>
#include <Fonts/smb36pt_b.h>
#include <Fonts/smb36pt_d.h>
#include <Fonts/smb36pt_l.h>

#include "ModuleScreenBase.h"
#include "ModuleTicker.h"
#include "buttons/ButtonAction.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"
#include "types/Config.h"
#include "types/Values.h"

typedef struct {
    uint16_t xmin;
    uint8_t ymin;
    uint16_t xmax;
    uint8_t ymax;
} rectangle_t;

class ModuleScreen {
   private:
    static ModuleScreenBase baseDisplay;
    static void renderHeader();
    static void renderButton(button_action_t buttonAction, uint16_t x);
    static void renderFooter(values_all_t *measurement);
    static void drawAntialiasedText06(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color);
    static void drawAntialiasedText08(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color);
    static void drawAntialiasedText18(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color);
    static void drawAntialiasedText36(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color);
    static void drawAntialiasedText(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color, const GFXfont *fontL, const GFXfont *fontD, const GFXfont *fontB);
    static void clearBuffer(config_t *config);
    static void flushBuffer();
    static void drawOuterBorders(uint16_t color);
    static void drawInnerBorders(uint16_t color);
    static void fillRectangle(rectangle_t rectangle, uint8_t color);
    static bool isWarn(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi);
    static bool isRisk(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi);
    static uint8_t getTextColor(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi);
    static uint8_t getFillColor(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi);
    static uint8_t getVertColor(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi);
    static String formatString(String value, char const *format);
    static float celsiusToFahrenheit(float celsius);

   public:
    static void begin();
    static void renderTable(values_all_t *measurement, config_t *config);
    static void renderChart(values_all_t history[60], config_t *config);
    static void hibernate();
};

#endif