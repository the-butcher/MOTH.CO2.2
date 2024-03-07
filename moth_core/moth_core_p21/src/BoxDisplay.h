#ifndef BoxDisplay_h
#define BoxDisplay_h

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
#include <qrcode.h>

#include "Adafruit_EPD.h"
#include "BoxClock.h"
#include "BoxStatus.h"
#include "Measurement.h"
#include "Measurements.h"
#include "Rectangle.h"
#include "ThinkInk_290_Grayscale4_T5_Clone.h"
#include "Thresholds.h"

typedef enum { DISPLAY_STATE_TABLE, DISPLAY_STATE_CHART } display_state_t;

typedef enum { DISPLAY_THEME_LIGHT, DISPLAY_THEME__DARK } display_theme_t;

/**
 * the value to be shown in chart
 */
typedef enum {
    DISPLAY_VAL_C___CO2,
    DISPLAY_VAL_C___DEG,
    DISPLAY_VAL_C___HUM,
    DISPLAY_VAL_C___HPA,
    DISPLAY_VAL_C___ALT,
    DISPLAY_VAL_C__P010,
    DISPLAY_VAL_C__P025,
    DISPLAY_VAL_C__P100
} display_val_c_v;

/**
 * the value to be shown in table
 */
typedef enum {
    DISPLAY_VAL_T___CO2,
    DISPLAY_VAL_T___HPA,
    DISPLAY_VAL_T___ALT,
    DISPLAY_VAL_T__P010,
    DISPLAY_VAL_T__P025,
    DISPLAY_VAL_T__P100
} display_val_t_v;

class BoxDisplay {
   private:
    static ThinkInk_290_Grayscale4_T5_Clone baseDisplay;
    static void renderHeader();
    static void renderFooter();
    static String formatString(String value, char const *format);
    static String firstDigitOfFraction(float value);
    static int getTextColor(float value, Thresholds thresholds);
    static int getFillColor(float value, Thresholds thresholds);
    static int getVertColor(float value, Thresholds thresholds);
    static void fillRectangle(Rectangle rectangle, uint16_t color);
    static void drawOuterBorders(uint16_t color);
    static void drawInnerBorders(uint16_t color);
    static void drawAntialiasedText06(String text, Rectangle rectangle,
                                      int xRel, int yRel, uint16_t color);
    static void drawAntialiasedText08(String text, Rectangle rectangle,
                                      int xRel, int yRel, uint16_t color);
    static void drawAntialiasedText18(String text, Rectangle rectangle,
                                      int xRel, int yRel, uint16_t color);
    static void drawAntialiasedText36(String text, Rectangle rectangle,
                                      int xRel, int yRel, uint16_t color);
    static void drawAntialiasedText(String text, Rectangle rectangle, int xRel,
                                    int yRel, uint16_t color,
                                    const GFXfont *fontL, const GFXfont *fontD,
                                    const GFXfont *fontB);
    static bool isWarn(float value, Thresholds thresholds);
    static bool isRisk(float value, Thresholds thresholds);
    static void renderChart();
    static void renderTable();
    static float celsiusToFahrenheit(float celsius);
    static void clearBuffer();
    static void flushBuffer();

   public:
    static String SYMBOL__PM_M;
    static String SYMBOL__PM_D;
    static String SYMBOL__WIFI;
    static String SYMBOL_THEME;
    static String SYMBOL_TABLE;
    static String SYMBOL_CHART;
    static String SYMBOL__BEEP;
    static String SYMBOL_NBEEP;
    static String CONFIG_PATH;
    static int16_t EPD_BUSY;
    static config_status_t configStatus;
    static int64_t renderStateSeconds;
    static void begin();
    static void updateConfiguration();
    static void renderCalibration(String messageA, String messageB);
    static void renderState();
    static void renderMothInfo(String info);
    static void renderQRCode();
    static void setTheme(display_theme_t theme);
    static void toggleTheme();
    static display_theme_t getTheme();
    static void setState(display_state_t state);
    static void toggleState();
    static display_state_t getState();
    static void setValueTable(display_val_t_v value);
    static display_val_t_v getValueTable();
    static void setValueChart(display_val_c_v value);
    static void toggleValueFw();
    static void toggleValueBw();
    static ValuesCo2 getDisplayValuesCo2();
    static bool hasSignificantChange();
    static void toggleChartMeasurementHoursFw();
    static void toggleChartMeasurementHoursBw();
    static int getChartMeasurementHours();
    static void hibernate(bool isAwakeRequired);
    static int getCo2RiskHi();
};

#endif