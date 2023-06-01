#ifndef BoxDisplay_h
#define BoxDisplay_h

#include "Adafruit_EPD.h"
#include "ThinkInk_290_Grayscale4_T5_Clone.h"
#include <Arduino.h>
#include "qrlib.h"
#include <Adafruit_GFX.h>

#include <Fonts/smb08pt_b.h>
#include <Fonts/smb08pt_d.h>
#include <Fonts/smb08pt_l.h>
#include <Fonts/smb20pt_b.h>
#include <Fonts/smb20pt_d.h>
#include <Fonts/smb20pt_l.h>
#include <Fonts/smb36pt_b.h>
#include <Fonts/smb36pt_d.h>
#include <Fonts/smb36pt_l.h>

#include "Thresholds.h"
#include "Measurements.h"
#include "Measurement.h"
#include "Rectangle.h"
#include "BoxStatus.h"

typedef enum {
  DISPLAY_STATE_TABLE,
  DISPLAY_STATE_CHART
} display_state_t;

typedef enum {
  DISPLAY_THEME_LIGHT,
  DISPLAY_THEME__DARK
} display_theme_t;

typedef enum {
  DISPLAY_VALUE___CO2,
  DISPLAY_VALUE___DEG,
  DISPLAY_VALUE___HUM,
  DISPLAY_VALUE___HPA
} display_value_t;

class BoxDisplay { 
  
  private:
    static ThinkInk_290_Grayscale4_T5_Clone baseDisplay;  
    static void renderHeader();
    static void renderFooter();
    static String formatString(String value, char const* format);
    static String firstDigitOfFraction(float value);
    static int getTextColor(float value, Thresholds thresholds);
    static int getFillColor(float value, Thresholds thresholds);
    static int getVertColor(float value, Thresholds thresholds);
    static void fillRectangle(Rectangle rectangle, uint16_t color);
    static void drawOuterBorders(uint16_t color);
    static void drawInnerBorders(uint16_t color);
    static void drawAntialiasedText08(String text, Rectangle rectangle, int xRel, int yRel, uint16_t color);
    static void drawAntialiasedText20(String text, Rectangle rectangle, int xRel, int yRel, uint16_t color);
    static void drawAntialiasedText36(String text, Rectangle rectangle, int xRel, int yRel, uint16_t color);
    static void drawAntialiasedText(String text, Rectangle rectangle, int xRel, int yRel, uint16_t color, const GFXfont *fontL, const GFXfont *fontD, const GFXfont *fontB);
    static bool isWarn(float value, Thresholds thresholds);
    static bool isRisk(float value, Thresholds thresholds);
    static void renderChart();
    static void renderTable();
    static float celsiusToFahrenheit(float celsius);
    
  public:
    static String CONFIG_PATH;
    static config_status_t configStatus;
    static int64_t renderStateSeconds;
    static Thresholds thresholdsCo2;
    static void begin();
    static void clearBuffer();
    static void flushBuffer();
    static void updateConfiguration();
    static void renderCalibration(String messageA, String messageB);
    static void renderState();
    static void renderMothInfo(String info);
    static void renderQRCode();
    static void toggleTheme();
    static void toggleState();
    static void toggleValue();
    static display_state_t getState();
    static ValuesCo2 getDisplayValues();

};

#endif