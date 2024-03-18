#ifndef BoxDisplay_h
#define BoxDisplay_h

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

#include "BoxDisplayBase.h"
#include "BoxTime.h"
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

class BoxDisplay {
   private:
    BoxDisplayBase baseDisplay;
    void renderHeader();
    void renderFooter(values_all_t *measurement);
    void drawAntialiasedText06(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color);
    void drawAntialiasedText08(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color);
    void drawAntialiasedText18(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color);
    void drawAntialiasedText36(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color);
    void drawAntialiasedText(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color, const GFXfont *fontL, const GFXfont *fontD, const GFXfont *fontB);
    void clearBuffer();
    void flushBuffer();
    void drawOuterBorders(uint16_t color);
    void drawInnerBorders(uint16_t color);
    void fillRectangle(rectangle_t rectangle, uint8_t color);
    static bool isWarn(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi);
    static bool isRisk(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi);
    static uint8_t getTextColor(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi);
    static uint8_t getFillColor(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi);
    static uint8_t getVertColor(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi);
    static String formatString(String value, char const *format);
    static float celsiusToFahrenheit(float celsius);

   public:
    void begin();
    void renderMeasurement(values_all_t *measurement, config_t *config);
    void hibernate();
};

#endif