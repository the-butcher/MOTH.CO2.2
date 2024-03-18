#include "BoxDisplay.h"

const uint16_t LIMIT_POS_X = 287;
const uint8_t CHAR_DIM_X6 = 7;

const rectangle_t RECT_TOP = {1, 1, 295, 22};
const rectangle_t RECT_BOT = {1, 106, 295, 127};
const rectangle_t RECT_CO2 = {1, 22, 207, 106};
const rectangle_t RECT_DEG = {207, 22, 295, 64};
const rectangle_t RECT_HUM = {207, 64, 295, 106};
const rectangle_t RECT_NRG = {279, 112, 288, 125};
const int TEXT_OFFSET_Y = 16;

const char FORMAT_3_DIGIT[] = "%3s";
const char FORMAT_4_DIGIT[] = "%4s";
const char FORMAT_5_DIGIT[] = "%5s";
const char FORMAT_6_DIGIT[] = "%6s";
const char FORMAT_CELL_PERCENT[] = "%5s%%";
const char FORMAT_STALE[] = "%3s%% stale";

const String SYMBOL__PM_M = "¦";
const String SYMBOL__PM_D = "§";
const String SYMBOL__WIFI = "¥";
const String SYMBOL_THEME = "¤";
const String SYMBOL_TABLE = "£";
const String SYMBOL_CHART = "¢";
const String SYMBOL__BEEP = "©";
const String SYMBOL_NBEEP = "ª";

void BoxDisplay::begin() {
    // nothing
}

void BoxDisplay::flushBuffer() {
    // >> flushBuffer
    baseDisplay.writeFrameBuffers();
    // << flushBuffer
}

void BoxDisplay::hibernate() {
    baseDisplay.begin(THINKINK_GRAYSCALE4, true);
    baseDisplay.hibernate();
}

void BoxDisplay::clearBuffer() {
    baseDisplay.begin(THINKINK_GRAYSCALE4, true);
    baseDisplay.clearBuffer();
}

void BoxDisplay::renderMeasurement(values_all_t *measurement, config_t *config) {
    clearBuffer();
    drawOuterBorders(EPD_LIGHT);
    drawInnerBorders(EPD_LIGHT);

    // values for temperature and humidity
    float deg = SensorScd041::toFloatDeg(measurement->valuesCo2.deg);  // TODO reconvert to float
    float hum = SensorScd041::toFloatHum(measurement->valuesCo2.hum);  // TODO reconvert to float

    // variables needed
    String title;
    uint8_t textColor;
    uint8_t fillColor;
    uint8_t vertColor;
    uint16_t xPosMainValue = 36;
    uint16_t charPosValueX = 193;
    uint16_t charPosFinalX;

    if (config->displayValTable == DISPLAY_VAL_T___CO2) {
        thresholds_co2_t thresholdsCo2 = config->thresholdsCo2;
        uint16_t co2 = measurement->valuesCo2.co2;
        float stale = max(0.0, min(10.0, (co2 - thresholdsCo2.reference) / 380.0));  // don't allow negative stale values. max out at 10
        float staleWarn = max(0.0, min(10.0, (thresholdsCo2.warnHi - thresholdsCo2.reference) / 380.0));
        float staleRisk = max(0.0, min(10.0, (thresholdsCo2.riskHi - thresholdsCo2.reference) / 380.0));

        float staleMax = 2.5;
        if (stale > 4) {
            staleMax = 10;
        } else if (stale > 2) {
            staleMax = 5;
        }
        int staleDif = 70 / staleMax;  // height of 1 percent, 20 for staleMax 2.5, 10 for staleMax 5, 5 for staleMax 10

        textColor = getTextColor(co2, 0, 0, thresholdsCo2.warnHi, thresholdsCo2.riskHi);
        fillColor = getFillColor(co2, 0, 0, thresholdsCo2.warnHi, thresholdsCo2.riskHi);
        vertColor = getVertColor(co2, 0, 0, thresholdsCo2.warnHi, thresholdsCo2.riskHi);

        if (fillColor != EPD_WHITE) {
            BoxDisplay::fillRectangle(RECT_CO2, fillColor);
        }

        title = "CO² ppm";
        charPosFinalX = charPosValueX - CHAR_DIM_X6 * 7;
        BoxDisplay::drawAntialiasedText36(formatString(String(co2), FORMAT_4_DIGIT), RECT_CO2, xPosMainValue, 76, textColor);

        int yMax = RECT_CO2.ymax - 7;  // bottom limit of rebreathe indicator
        int yDim = round(stale * staleDif);
        int yDimWarn = round(staleWarn * staleDif);
        int yDimRisk = round(staleRisk * staleDif);
        int yDimMax = round(staleMax * staleDif);

        int xBarMin = 8;
        int xBarMax = 13;
        for (int i = xBarMin; i <= xBarMax; i++) {
            baseDisplay.drawFastVLine(i, yMax - yDimWarn, yDimWarn, EPD_WHITE);             // good
            baseDisplay.drawFastVLine(i, yMax - yDimRisk, yDimRisk - yDimWarn, vertColor);  // warn
            baseDisplay.drawFastVLine(i, yMax - yDimMax, yDimMax - yDimRisk, EPD_BLACK);    // risk
        }

        if (BoxDisplay::isRisk(co2, 0, 0, thresholdsCo2.warnHi, thresholdsCo2.riskHi)) {  // when in risk, the risk area needs to be outlined
            baseDisplay.drawFastVLine(xBarMin, yMax - yDimMax, yDimMax - yDimRisk, vertColor);
            baseDisplay.drawFastVLine(xBarMax, yMax - yDimMax, yDimMax - yDimRisk, vertColor);
            baseDisplay.drawFastHLine(xBarMin, yMax - yDimMax, xBarMax - xBarMin + 1, vertColor);
        } else if (!BoxDisplay::isWarn(co2, 0, 0, thresholdsCo2.warnHi, thresholdsCo2.riskHi)) {  // when not warn and not risk, the good area needs to be outlined
            baseDisplay.drawFastVLine(xBarMin, yMax - yDimWarn, yDimWarn, vertColor);
            baseDisplay.drawFastVLine(xBarMax, yMax - yDimWarn, yDimWarn, vertColor);
            baseDisplay.drawFastHLine(xBarMin, yMax, xBarMax - xBarMin + 1, vertColor);
        }

        int yTxt = RECT_CO2.ymax - RECT_CO2.ymin - yDim - 10;
        int yPrc = yTxt;
        int xPrc = xBarMax + 19;
        int xLin = 24;

        int stale10 = round(stale * 10.0);
        int staleFix = floor(stale10 / 10.0);
        int staleFrc = abs(stale10 % 10);

        if (stale > 8.5) {
            yTxt += 14;
            yPrc += 14;
        } else if (co2 >= 1000) {  // wrap percent, shorten line
            yPrc += 14;
            xPrc = xBarMax + 2;
            xLin -= CHAR_DIM_X6;
        }

        // draw a narrowed percentage number
        baseDisplay.drawFastHLine(xBarMax + 3, yMax - yDim, xLin, textColor);
        BoxDisplay::drawAntialiasedText06(String(staleFix), RECT_CO2, xBarMax + 2, yTxt, textColor);
        if (stale < 10) {
            BoxDisplay::drawAntialiasedText06(".", RECT_CO2, xBarMax + 7, yTxt, textColor);
            BoxDisplay::drawAntialiasedText06(String(staleFrc), RECT_CO2, xBarMax + 12, yTxt, textColor);
        }
        BoxDisplay::drawAntialiasedText06("%", RECT_CO2, xPrc, yPrc, textColor);

    } else if (config->displayValTable == DISPLAY_VAL_T___HPA) {
        int hpa = measurement->valuesBme.pressure;
        textColor = EPD_BLACK;
        fillColor = EPD_WHITE;
        vertColor = EPD_DARK;

        BoxDisplay::fillRectangle(RECT_CO2, fillColor);

        title = "pressure hPa";
        charPosFinalX = charPosValueX - CHAR_DIM_X6 * title.length();
        BoxDisplay::drawAntialiasedText36(formatString(String(hpa), FORMAT_4_DIGIT), RECT_CO2, xPosMainValue, 76, textColor);
    } else if (config->displayValTable == DISPLAY_VAL_T___ALT) {
        // TODO alt calculation here
        int alt = 0;  // round(Measurements::getLatestMeasurement().valuesBme.altitude);
        textColor = EPD_BLACK;
        fillColor = EPD_WHITE;
        vertColor = EPD_DARK;

        BoxDisplay::fillRectangle(RECT_CO2, fillColor);

        title = "altitude m";
        charPosFinalX = charPosValueX - CHAR_DIM_X6 * title.length();
        BoxDisplay::drawAntialiasedText36(formatString(String(alt), FORMAT_4_DIGIT), RECT_CO2, xPosMainValue, 76, textColor);
    }
    BoxDisplay::drawAntialiasedText06(title, RECT_CO2, charPosFinalX, TEXT_OFFSET_Y, textColor);

    thresholds_lh_t thresholdsDeg = config->thresholdsDeg;
    textColor = getTextColor(deg, thresholdsDeg.riskLo, thresholdsDeg.wanrLo, thresholdsDeg.warnHi, thresholdsDeg.riskHi);
    fillColor = getFillColor(deg, thresholdsDeg.riskLo, thresholdsDeg.wanrLo, thresholdsDeg.warnHi, thresholdsDeg.riskHi);
    if (config->isFahrenheit) {
        deg = BoxDisplay::celsiusToFahrenheit(deg);
    }
    int temperature10 = round(deg * 10.0);
    int temperatureFix = floor(temperature10 / 10.0);
    int temperatureFrc = abs(temperature10 % 10);
    if (fillColor != EPD_WHITE) {
        BoxDisplay::fillRectangle(RECT_DEG, fillColor);
    }
    BoxDisplay::drawAntialiasedText18(formatString(String(temperatureFix), FORMAT_3_DIGIT), RECT_DEG, 0, 35, textColor);
    BoxDisplay::drawAntialiasedText06(config->isFahrenheit ? "°F" : "°C", RECT_DEG, 80 - CHAR_DIM_X6 * 2, TEXT_OFFSET_Y + 2, textColor);
    BoxDisplay::drawAntialiasedText08(".", RECT_DEG, 63, 35, textColor);
    BoxDisplay::drawAntialiasedText08(String(temperatureFrc), RECT_DEG, 72, 35, textColor);

    thresholds_lh_t thresholdsHum = config->thresholdsHum;
    textColor = getTextColor(hum, thresholdsHum.riskLo, thresholdsHum.wanrLo, thresholdsHum.warnHi, thresholdsHum.riskHi);
    fillColor = getFillColor(hum, thresholdsHum.riskLo, thresholdsHum.wanrLo, thresholdsHum.warnHi, thresholdsHum.riskHi);
    int humidity10 = round(hum * 10.0);
    int humidityFix = floor(humidity10 / 10.0);
    int humidityFrc = abs(humidity10 % 10);
    if (fillColor != EPD_WHITE) {
        BoxDisplay::fillRectangle(RECT_HUM, fillColor);
    }
    BoxDisplay::drawAntialiasedText18(formatString(String(humidityFix), FORMAT_3_DIGIT), RECT_HUM, 0, 35, textColor);
    BoxDisplay::drawAntialiasedText06("%", RECT_HUM, 80 - CHAR_DIM_X6 * 1, TEXT_OFFSET_Y + 2, textColor);
    BoxDisplay::drawAntialiasedText08(".", RECT_HUM, 63, 35, textColor);
    BoxDisplay::drawAntialiasedText08(String(humidityFrc), RECT_HUM, 72, 35, textColor);

    BoxDisplay::renderHeader();
    BoxDisplay::renderFooter(measurement);

    BoxDisplay::flushBuffer();
}

void BoxDisplay::renderHeader() {
    // BoxDisplay::drawAntialiasedText08(ButtonHandlers::A.buttonActionSlow.label, RECT_TOP, 6, TEXT_OFFSET_Y, EPD_BLACK);
    // BoxDisplay::drawAntialiasedText08(ButtonHandlers::A.buttonActionFast.label, RECT_TOP, 6 + charDimX6 * 2, TEXT_OFFSET_Y, EPD_BLACK);
    // BoxDisplay::drawAntialiasedText06(ButtonHandlers::A.extraLabel, RECT_TOP, 6 + charDimX6 * 4, TEXT_OFFSET_Y - 2, EPD_BLACK);

    // BoxDisplay::drawAntialiasedText08(ButtonHandlers::B.buttonActionSlow.label, RECT_TOP, 135, TEXT_OFFSET_Y, EPD_BLACK);
    // BoxDisplay::drawAntialiasedText08(ButtonHandlers::B.buttonActionFast.label, RECT_TOP, 135 + charDimX6 * 2, TEXT_OFFSET_Y, EPD_BLACK);
    // BoxDisplay::drawAntialiasedText06(ButtonHandlers::B.extraLabel, RECT_TOP, 135 + charDimX6 * 4, TEXT_OFFSET_Y - 2, EPD_BLACK);

    // BoxDisplay::drawAntialiasedText08(ButtonHandlers::C.buttonActionSlow.label, RECT_TOP, 264, TEXT_OFFSET_Y, EPD_BLACK);
    // BoxDisplay::drawAntialiasedText08(ButtonHandlers::C.buttonActionFast.label, RECT_TOP, 264 + charDimX6 * 2, TEXT_OFFSET_Y, EPD_BLACK);
}

void BoxDisplay::renderFooter(values_all_t *measurement) {
    float percent = SensorEnergy::toFloatPercent(measurement->valuesNrg.percent);

    String cellPercentFormatted = formatString(String(percent, 0), FORMAT_CELL_PERCENT);
    BoxDisplay::drawAntialiasedText06(cellPercentFormatted, RECT_BOT, LIMIT_POS_X - 14 - CHAR_DIM_X6 * cellPercentFormatted.length(), TEXT_OFFSET_Y, EPD_BLACK);

    // main battery frame
    BoxDisplay::fillRectangle(RECT_NRG, EPD_LIGHT);

    // percentage
    uint8_t yMin = RECT_NRG.ymax - 3 - (uint8_t)round((RECT_NRG.ymax - RECT_NRG.ymin - 3) * percent * 0.01f);
    fillRectangle({RECT_NRG.xmin, yMin, RECT_NRG.xmax, RECT_NRG.ymax}, EPD_BLACK);

    // battery contact clip
    baseDisplay.drawFastHLine(RECT_NRG.xmin + 1, RECT_NRG.ymin + 1, 2, EPD_WHITE);
    baseDisplay.drawFastHLine(RECT_NRG.xmax - 3, RECT_NRG.ymin + 1, 2, EPD_WHITE);

    bool isConn = false;  // TODO :: maybe a third parameter "state" BoxConn::getMode() != WIFI_OFF;
    bool isBeep = false;  // TODO :: maybe a third parameter "state" BoxBeep::getSound() == SOUND__ON;

    int charPosFooter = 7;
    if (isBeep) {
        BoxDisplay::drawAntialiasedText08(SYMBOL__BEEP, RECT_BOT, 6, TEXT_OFFSET_Y + 1, EPD_BLACK);
        charPosFooter += 13;
    }

    if (isConn) {
        // String address = BoxConn::getAddress();
        // BoxDisplay::drawAntialiasedText06(BoxConn::getAddress(), RECT_BOT, charPosFooter, TEXT_OFFSET_Y, EPD_BLACK);
        // String timeFormatted = BoxClock::getDateTimeDisplayString(BoxClock::getDate());
        // BoxDisplay::drawAntialiasedText06(",", RECT_BOT, charPosFooter + address.length() * charDimX6, TEXT_OFFSET_Y, EPD_BLACK);
        // BoxDisplay::drawAntialiasedText06(timeFormatted, RECT_BOT, charPosFooter + (address.length() + 1) * charDimX6, TEXT_OFFSET_Y, EPD_BLACK);
    } else {
        String timeFormatted = BoxTime::getDateTimeDisplayString(measurement->secondstime);
        BoxDisplay::drawAntialiasedText06(timeFormatted, RECT_BOT, charPosFooter, TEXT_OFFSET_Y, EPD_BLACK);
    }
}

void BoxDisplay::drawAntialiasedText06(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color) {
    drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb06pt_l, &smb06pt_d, &smb06pt_b);
}

void BoxDisplay::drawAntialiasedText08(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color) {
    drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb08pt_l, &smb08pt_d, &smb08pt_b);
}

void BoxDisplay::drawAntialiasedText18(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color) {
    drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb18pt_l, &smb18pt_d, &smb18pt_b);
}

void BoxDisplay::drawAntialiasedText36(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color) {
    drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb36pt_l, &smb36pt_d, &smb36pt_b);
}

/**
 * when switching fonts, setFont(...) must be called before setCursor(...), or there may be a y-offset on the very first text
 */
void BoxDisplay::drawAntialiasedText(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color, const GFXfont *fontL, const GFXfont *fontD, const GFXfont *fontB) {
    baseDisplay.setFont(fontL);
    baseDisplay.setCursor(rectangle.xmin + xRel, rectangle.ymin + yRel);
    baseDisplay.setTextColor(color == EPD_BLACK ? EPD_LIGHT : EPD_DARK);
    baseDisplay.print(text);

    baseDisplay.setFont(fontD);
    baseDisplay.setCursor(rectangle.xmin + xRel, rectangle.ymin + yRel);
    baseDisplay.setTextColor(color == EPD_BLACK ? EPD_DARK : EPD_LIGHT);
    baseDisplay.print(text);

    baseDisplay.setFont(fontB);
    baseDisplay.setCursor(rectangle.xmin + xRel, rectangle.ymin + yRel);
    baseDisplay.setTextColor(color);
    baseDisplay.print(text);
}

void BoxDisplay::drawOuterBorders(uint16_t color) {
    // top
    baseDisplay.drawFastHLine(0, 0, 296, color);
    baseDisplay.drawFastHLine(0, 1, 296, color);
    // header
    baseDisplay.drawFastHLine(0, 21, 296, color);
    baseDisplay.drawFastHLine(0, 22, 296, color);
    // footer
    baseDisplay.drawFastHLine(0, 105, 296, color);
    baseDisplay.drawFastHLine(0, 106, 296, color);
    // bottom
    baseDisplay.drawFastHLine(0, 126, 296, color);
    baseDisplay.drawFastHLine(0, 127, 296, color);
    // left
    baseDisplay.drawFastVLine(0, 0, 128, color);
    baseDisplay.drawFastVLine(1, 0, 128, color);
    // right
    baseDisplay.drawFastVLine(294, 0, 128, color);
    baseDisplay.drawFastVLine(295, 0, 128, color);
}

void BoxDisplay::drawInnerBorders(uint16_t color) {
    // horizontal center
    baseDisplay.drawFastHLine(206, 63, 100, color);
    baseDisplay.drawFastHLine(206, 64, 100, color);
    // vertical center
    baseDisplay.drawFastVLine(206, 21, 86, color);
    baseDisplay.drawFastVLine(207, 22, 86, color);
}

void BoxDisplay::fillRectangle(rectangle_t rectangle, uint8_t color) {
    baseDisplay.fillRect(rectangle.xmin + 1, rectangle.ymin + 1, rectangle.xmax - rectangle.xmin - 2, rectangle.ymax - rectangle.ymin - 2, color);
}

bool BoxDisplay::isWarn(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi) {
    return value < warnLo || value >= warnHi;
}

bool BoxDisplay::isRisk(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi) {
    return value < riskLo || value >= riskHi;
}

uint8_t BoxDisplay::getTextColor(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi) {
    if (BoxDisplay::isRisk(value, riskLo, warnLo, warnHi, riskHi)) {
        return EPD_WHITE;
    } else {
        return EPD_BLACK;
    }
}

uint8_t BoxDisplay::getFillColor(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi) {
    if (BoxDisplay::isRisk(value, riskLo, warnLo, warnHi, riskHi)) {
        return EPD_BLACK;
    } else if (BoxDisplay::isWarn(value, riskLo, warnLo, warnHi, riskHi)) {
        return EPD_LIGHT;
    } else {
        return EPD_WHITE;
    }
}

uint8_t BoxDisplay::getVertColor(float value, uint16_t riskLo, uint16_t warnLo, uint16_t warnHi, uint16_t riskHi) {
    if (BoxDisplay::isRisk(value, riskLo, warnLo, warnHi, riskHi)) {
        return EPD_LIGHT;
    } else {
        return EPD_DARK;
    }
}

String BoxDisplay::formatString(String value, char const *format) {
    char padBuffer[16];
    sprintf(padBuffer, format, value);
    return padBuffer;
}

float BoxDisplay::celsiusToFahrenheit(float celsius) {
    return celsius * 9.0f / 5.0f + 32.0f;
}
