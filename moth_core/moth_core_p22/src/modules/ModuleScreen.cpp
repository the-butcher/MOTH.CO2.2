#include "ModuleScreen.h"

#include "ModuleClock.h"
#include "buttons/ButtonAction.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"

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

ModuleScreenBase ModuleScreen::baseDisplay;
uint64_t ModuleScreen::ext1Bitmask = 1ULL << PIN_EPD_BUSY;
std::function<void(void)> ModuleScreen::wakeupActionBusyHighCallback = nullptr;

void ModuleScreen::begin(std::function<void(void)> wakeupActionBusyHighCallback) {
    ModuleScreen::wakeupActionBusyHighCallback = wakeupActionBusyHighCallback;
}

void ModuleScreen::prepareSleep(wakeup_action_e wakeupType) {
    if (wakeupType == WAKEUP_ACTION_BUSY) {
        esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_EPD_BUSY, HIGH);
    }
}

/**
 * waits for the busy pin to become high, then calls WakeupActionBusyHighCallback --> depower the display
 * this function handles a "isDelayRequired = true" situation
 */
void ModuleScreen::detectBusyPinHigh(void *parameter) {
    uint64_t millisA = millis();
    while (!digitalRead(PIN_EPD_BUSY) == HIGH) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    wakeupActionBusyHighCallback();
    vTaskDelete(NULL);
}

void ModuleScreen::attachWakeup(wakeup_action_e wakeupType) {
    if (wakeupType == WAKEUP_ACTION_BUSY) {
        xTaskCreate(ModuleScreen::detectBusyPinHigh, "detect busy pin high", 5000, NULL, 2, NULL);
    }
}
void ModuleScreen::detachWakeup(wakeup_action_e wakeupType) {
    if (wakeupType == WAKEUP_ACTION_BUSY) {
        // do nothing
    }
}

void ModuleScreen::flushBuffer() {
    // >> flushBuffer
    ModuleScreen::baseDisplay.writeFrameBuffers();
    // << flushBuffer
}

void ModuleScreen::depower() {
    ModuleScreen::baseDisplay.begin(THINKINK_GRAYSCALE4, true);
    ModuleScreen::baseDisplay.depower();
}

void ModuleScreen::clearBuffer(config_t *config) {
    ModuleScreen::baseDisplay.begin(THINKINK_GRAYSCALE4, config->disp.displayValTheme == DISPLAY_THM___LIGHT);
    ModuleScreen::baseDisplay.clearBuffer();
}

void ModuleScreen::renderTable(values_all_t *measurement, config_t *config) {
    ModuleScreen::clearBuffer(config);
    ModuleScreen::drawOuterBorders(EPD_LIGHT);
    ModuleScreen::drawInnerBorders(EPD_LIGHT);

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

    if (config->disp.displayValTable == DISPLAY_VAL_T___CO2) {
        thresholds_co2_t thresholdsCo2 = config->disp.thresholdsCo2;
        uint16_t co2 = measurement->valuesCo2.co2;
        float stale = max(0.0, min(10.0, (co2 - thresholdsCo2.ref) / 380.0));  // don't allow negative stale values. max out at 10
        float staleWarn = max(0.0, min(10.0, (thresholdsCo2.wHi - thresholdsCo2.ref) / 380.0));
        float staleRisk = max(0.0, min(10.0, (thresholdsCo2.rHi - thresholdsCo2.ref) / 380.0));

        float staleMax = 2.5;
        if (stale > 4) {
            staleMax = 10;
        } else if (stale > 2) {
            staleMax = 5;
        }
        int staleDif = 70 / staleMax;  // height of 1 percent, 20 for staleMax 2.5, 10 for staleMax 5, 5 for staleMax 10

        textColor = getTextColor(co2, 0, 0, thresholdsCo2.wHi, thresholdsCo2.rHi);
        fillColor = getFillColor(co2, 0, 0, thresholdsCo2.wHi, thresholdsCo2.rHi);
        vertColor = getVertColor(co2, 0, 0, thresholdsCo2.wHi, thresholdsCo2.rHi);

        if (fillColor != EPD_WHITE) {
            ModuleScreen::fillRectangle(RECT_CO2, fillColor);
        }

        title = "CO² ppm";
        charPosFinalX = charPosValueX - CHAR_DIM_X6 * 7;
        ModuleScreen::drawAntialiasedText36(formatString(String(co2), FORMAT_4_DIGIT), RECT_CO2, xPosMainValue, 76, textColor);

        uint8_t yMax = RECT_CO2.ymax - 7;  // bottom limit of rebreathe indicator
        uint8_t yDim = round(stale * staleDif);
        uint8_t yDimWarn = round(staleWarn * staleDif);
        uint8_t yDimRisk = round(staleRisk * staleDif);
        int yDimMax = round(staleMax * staleDif);

        uint8_t xBarMin = 8;
        uint8_t xBarMax = 13;
        for (uint8_t i = xBarMin; i <= xBarMax; i++) {
            ModuleScreen::baseDisplay.drawFastVLine(i, yMax - yDimWarn, yDimWarn, EPD_WHITE);             // good
            ModuleScreen::baseDisplay.drawFastVLine(i, yMax - yDimRisk, yDimRisk - yDimWarn, vertColor);  // warn
            ModuleScreen::baseDisplay.drawFastVLine(i, yMax - yDimMax, yDimMax - yDimRisk, EPD_BLACK);    // risk
        }

        if (ModuleScreen::isRisk(co2, 0, 0, thresholdsCo2.wHi, thresholdsCo2.rHi)) {  // when in risk, the risk area needs to be outlined
            ModuleScreen::baseDisplay.drawFastVLine(xBarMin, yMax - yDimMax, yDimMax - yDimRisk, vertColor);
            ModuleScreen::baseDisplay.drawFastVLine(xBarMax, yMax - yDimMax, yDimMax - yDimRisk, vertColor);
            ModuleScreen::baseDisplay.drawFastHLine(xBarMin, yMax - yDimMax, xBarMax - xBarMin + 1, vertColor);
        } else if (!ModuleScreen::isWarn(co2, 0, 0, thresholdsCo2.wHi, thresholdsCo2.rHi)) {  // when not warn and not risk, the good area needs to be outlined
            ModuleScreen::baseDisplay.drawFastVLine(xBarMin, yMax - yDimWarn, yDimWarn, vertColor);
            ModuleScreen::baseDisplay.drawFastVLine(xBarMax, yMax - yDimWarn, yDimWarn, vertColor);
            ModuleScreen::baseDisplay.drawFastHLine(xBarMin, yMax, xBarMax - xBarMin + 1, vertColor);
        }

        uint8_t yTxt = RECT_CO2.ymax - RECT_CO2.ymin - yDim - 10;
        uint8_t yPrc = yTxt;
        uint8_t xPrc = xBarMax + 19;
        uint8_t xLin = 24;

        uint8_t stale10 = round(stale * 10.0);
        uint8_t staleFix = floor(stale10 / 10.0);
        uint8_t staleFrc = abs(stale10 % 10);

        if (stale > 8.5) {
            yTxt += 14;
            yPrc += 14;
        } else if (co2 >= 1000) {  // wrap percent, shorten line
            yPrc += 14;
            xPrc = xBarMax + 2;
            xLin -= CHAR_DIM_X6;
        }

        // draw a narrowed percentage number
        ModuleScreen::baseDisplay.drawFastHLine(xBarMax + 3, yMax - yDim, xLin, textColor);
        ModuleScreen::drawAntialiasedText06(String(staleFix), RECT_CO2, xBarMax + 2, yTxt, textColor);
        if (stale < 10) {
            ModuleScreen::drawAntialiasedText06(".", RECT_CO2, xBarMax + 7, yTxt, textColor);
            ModuleScreen::drawAntialiasedText06(String(staleFrc), RECT_CO2, xBarMax + 12, yTxt, textColor);
        }
        ModuleScreen::drawAntialiasedText06("%", RECT_CO2, xPrc, yPrc, textColor);

    } else if (config->disp.displayValTable == DISPLAY_VAL_T___HPA) {
        float pressure = measurement->valuesBme.pressure;
        textColor = EPD_BLACK;
        fillColor = EPD_WHITE;
        vertColor = EPD_DARK;
        ModuleScreen::fillRectangle(RECT_CO2, fillColor);
        title = "pressure hPa";
        charPosFinalX = charPosValueX - CHAR_DIM_X6 * title.length();
        ModuleScreen::drawAntialiasedText36(formatString(String(pressure, 0), FORMAT_4_DIGIT), RECT_CO2, xPosMainValue, 76, textColor);
    } else if (config->disp.displayValTable == DISPLAY_VAL_T___ALT) {
        float altitude = SensorBme280::getAltitude(config->pressureZerolevel, measurement->valuesBme.pressure);
        textColor = EPD_BLACK;
        fillColor = EPD_WHITE;
        vertColor = EPD_DARK;
        ModuleScreen::fillRectangle(RECT_CO2, fillColor);
        title = "altitude m";
        charPosFinalX = charPosValueX - CHAR_DIM_X6 * title.length();
        ModuleScreen::drawAntialiasedText36(formatString(String(altitude, 0), FORMAT_4_DIGIT), RECT_CO2, xPosMainValue, 76, textColor);
    }
    ModuleScreen::drawAntialiasedText06(title, RECT_CO2, charPosFinalX, TEXT_OFFSET_Y, textColor);

    thresholds_deg_t thresholdsDeg = config->disp.thresholdsDeg;
    textColor = getTextColor(deg, thresholdsDeg.rLo, thresholdsDeg.wLo, thresholdsDeg.wHi, thresholdsDeg.rHi);
    fillColor = getFillColor(deg, thresholdsDeg.rLo, thresholdsDeg.wLo, thresholdsDeg.wHi, thresholdsDeg.rHi);
    if (config->disp.displayDegScale == DISPLAY_DEG_FAHRENH) {
        deg = ModuleScreen::celsiusToFahrenheit(deg);
    }
    int temperature10 = round(deg * 10.0);
    int temperatureFix = floor(temperature10 / 10.0);
    int temperatureFrc = abs(temperature10 % 10);
    if (fillColor != EPD_WHITE) {
        ModuleScreen::fillRectangle(RECT_DEG, fillColor);
    }
    ModuleScreen::drawAntialiasedText18(formatString(String(temperatureFix), FORMAT_3_DIGIT), RECT_DEG, 0, 35, textColor);
    ModuleScreen::drawAntialiasedText06(config->disp.displayDegScale == DISPLAY_DEG_FAHRENH ? "°F" : "°C", RECT_DEG, 80 - CHAR_DIM_X6 * 2, TEXT_OFFSET_Y + 2, textColor);
    ModuleScreen::drawAntialiasedText08(".", RECT_DEG, 63, 35, textColor);
    ModuleScreen::drawAntialiasedText08(String(temperatureFrc), RECT_DEG, 72, 35, textColor);

    thresholds_hum_t thresholdsHum = config->disp.thresholdsHum;
    textColor = getTextColor(hum, thresholdsHum.rLo, thresholdsHum.wLo, thresholdsHum.wHi, thresholdsHum.rHi);
    fillColor = getFillColor(hum, thresholdsHum.rLo, thresholdsHum.wLo, thresholdsHum.wHi, thresholdsHum.rHi);
    int humidity10 = round(hum * 10.0);
    int humidityFix = floor(humidity10 / 10.0);
    int humidityFrc = abs(humidity10 % 10);
    if (fillColor != EPD_WHITE) {
        ModuleScreen::fillRectangle(RECT_HUM, fillColor);
    }
    ModuleScreen::drawAntialiasedText18(formatString(String(humidityFix), FORMAT_3_DIGIT), RECT_HUM, 0, 35, textColor);
    ModuleScreen::drawAntialiasedText06("%", RECT_HUM, 80 - CHAR_DIM_X6 * 1, TEXT_OFFSET_Y + 2, textColor);
    ModuleScreen::drawAntialiasedText08(".", RECT_HUM, 63, 35, textColor);
    ModuleScreen::drawAntialiasedText08(String(humidityFrc), RECT_HUM, 72, 35, textColor);

    ModuleScreen::renderHeader();
    ModuleScreen::renderFooter(config);

    ModuleScreen::flushBuffer();
}

/**
 * measurement is reference for building the measurement table backwards
 */
void ModuleScreen::renderChart(values_all_t history[60], config_t *config) {
    ModuleScreen::clearBuffer(config);
    ModuleScreen::drawOuterBorders(EPD_LIGHT);

    display_val_c_e displayValChart = config->disp.displayValChart;

    values_all_t measurement;
    uint16_t minValue = 0;
    uint16_t maxValue = 1500;
    if (displayValChart == DISPLAY_VAL_C___CO2) {
        for (uint8_t i = 0; i < HISTORY_____BUFFER_SIZE; i++) {
            measurement = history[i];
            if (measurement.valuesCo2.co2 > 3600) {
                maxValue = 6000;  // 0, 2000, 4000, (6000)
            } else if (measurement.valuesCo2.co2 > 2400) {
                maxValue = 4500;  // 0, 1500, 3000, (4500)
            } else if (measurement.valuesCo2.co2 > 1200) {
                maxValue = 3000;  // 0, 1000, 2000, (3000)
            }
        }
    } else if (displayValChart == DISPLAY_VAL_C___DEG) {
        if (config->disp.displayDegScale == DISPLAY_DEG_FAHRENH) {
            minValue = 60;
            maxValue = 120;
        } else {
            minValue = 10;
            maxValue = 40;
        }
    } else if (displayValChart == DISPLAY_VAL_C___HUM) {
        minValue = 20;
        maxValue = 80;  // 20, 40, 60, (80)
    } else if (displayValChart == DISPLAY_VAL_C___HPA) {
        double pressureAvg = 0;
        for (uint8_t i = 0; i < HISTORY_____BUFFER_SIZE; i++) {
            measurement = history[i];
            pressureAvg += measurement.valuesBme.pressure / 100.0;
        }
        pressureAvg /= HISTORY_____BUFFER_SIZE;          // lets say it is 998
        minValue = floor(pressureAvg / 10.0) * 10 - 10;  // 99.8 -> 99 -> 990 -> 980
        maxValue = ceil(pressureAvg / 10.0) * 10 + 10;   // 99.8 -> 100 -> 1000 -> 1010
    } else if (displayValChart == DISPLAY_VAL_C___ALT) {
        maxValue = 450;
        float altitude;
        for (uint8_t i = 0; i < HISTORY_____BUFFER_SIZE; i++) {
            measurement = history[i];
            altitude = SensorBme280::getAltitude(config->pressureZerolevel, measurement.valuesBme.pressure);
            if (altitude > 1600) {
                maxValue = 3600;  // 0, 1200, 2400, 3600
            } else if (altitude > 800) {
                maxValue = 1800;  // 0, 600, 1200, (1800)
            } else if (altitude > 400) {
                maxValue = 900;  // 0, 300, 600, (900)
            }
        }
    }

    String label2 = String(minValue + (maxValue - minValue) * 2 / 3);
    String label1 = String(minValue + (maxValue - minValue) / 3);
    String label0 = String(minValue);

    uint8_t charPosValueX = 41;
    uint8_t charPosLabelY = 12;

    ModuleScreen::drawAntialiasedText06(label2, RECT_CO2, charPosValueX - CHAR_DIM_X6 * label2.length(), 24, EPD_BLACK);
    ModuleScreen::baseDisplay.drawFastHLine(1, 49, 296, EPD_LIGHT);
    ModuleScreen::drawAntialiasedText06(label1, RECT_CO2, charPosValueX - CHAR_DIM_X6 * label1.length(), 52, EPD_BLACK);
    ModuleScreen::baseDisplay.drawFastHLine(1, 77, 296, EPD_LIGHT);
    ModuleScreen::drawAntialiasedText06(label0, RECT_CO2, charPosValueX - CHAR_DIM_X6 * label0.length(), 80, EPD_BLACK);

    String title;

    if (displayValChart == DISPLAY_VAL_C___CO2) {
        title = "CO² ppm," + String(config->disp.displayHrsChart) + "h";  // the sup 2 has been modified in the font to display as sub
    } else if (displayValChart == DISPLAY_VAL_C___DEG) {
        title = config->disp.displayDegScale == DISPLAY_DEG_FAHRENH ? "temperature °F," : "temperature °C," + String(config->disp.displayHrsChart) + "h";
    } else if (displayValChart == DISPLAY_VAL_C___HUM) {
        title = "humidity %," + String(config->disp.displayHrsChart) + "h";
    } else if (displayValChart == DISPLAY_VAL_C___HPA) {
        title = "pressure hPa," + String(config->disp.displayHrsChart) + "h";
    } else if (displayValChart == DISPLAY_VAL_C___ALT) {
        title = "altitude m," + String(config->disp.displayHrsChart) + "h";
    }
    ModuleScreen::drawAntialiasedText06(title, RECT_CO2, LIMIT_POS_X - title.length() * CHAR_DIM_X6 + 3, charPosLabelY, EPD_BLACK);

    uint16_t minX;
    uint8_t minY;
    uint8_t maxY = 103;
    uint8_t dimY;
    uint8_t limY = 78.0;
    float curValue;

    uint8_t displayableBarWidth = 240 / HISTORY_____BUFFER_SIZE;
    uint8_t basX = LIMIT_POS_X - HISTORY_____BUFFER_SIZE * displayableBarWidth;
    for (uint8_t i = 0; i < HISTORY_____BUFFER_SIZE; i++) {
        measurement = history[i];

        if (displayValChart == DISPLAY_VAL_C___CO2) {
            curValue = measurement.valuesCo2.co2;
        } else if (displayValChart == DISPLAY_VAL_C___DEG) {
            curValue = SensorScd041::toFloatDeg(measurement.valuesCo2.deg);
        } else if (displayValChart == DISPLAY_VAL_C___HUM) {
            curValue = SensorScd041::toFloatHum(measurement.valuesCo2.hum);
        } else if (displayValChart == DISPLAY_VAL_C___HPA) {
            curValue = measurement.valuesBme.pressure;
        } else if (displayValChart == DISPLAY_VAL_C___ALT) {
            curValue = SensorBme280::getAltitude(config->pressureZerolevel, measurement.valuesBme.pressure);
        }

        minX = basX + i * displayableBarWidth;
        dimY = max(0, min((int)limY, (int)round((curValue - minValue) * limY / (maxValue - minValue))));
        minY = maxY - dimY;

        if (dimY > 0) {
            for (int b = 0; b < displayableBarWidth - 1; b++) {
                baseDisplay.drawFastVLine(minX + b, minY, dimY, EPD_BLACK);
            }
        } else {
            baseDisplay.drawFastHLine(minX, minY - 1, displayableBarWidth - 1, EPD_BLACK);
        }
    }

    ModuleScreen::renderHeader();
    ModuleScreen::renderFooter(config);

    ModuleScreen::flushBuffer();
}

void ModuleScreen::renderHeader() {
    renderButton(ButtonAction::A.buttonAction, 6);
    renderButton(ButtonAction::B.buttonAction, 135);
    renderButton(ButtonAction::C.buttonAction, 264);
}

void ModuleScreen::renderButton(button_action_t buttonAction, uint16_t x) {
    ModuleScreen::drawAntialiasedText08(buttonAction.symbolSlow, RECT_TOP, x, TEXT_OFFSET_Y, EPD_BLACK);
    ModuleScreen::drawAntialiasedText08(buttonAction.symbolFast, RECT_TOP, x + CHAR_DIM_X6 * 2, TEXT_OFFSET_Y, EPD_BLACK);
    ModuleScreen::drawAntialiasedText06(buttonAction.extraLabel, RECT_TOP, x + CHAR_DIM_X6 * 4, TEXT_OFFSET_Y - 2, EPD_BLACK);
}

void ModuleScreen::renderFooter(config_t *config) {
    float percent = SensorEnergy::toFloatPercent(SensorEnergy::readval().percent);

    String cellPercentFormatted = formatString(String(percent, 0), FORMAT_CELL_PERCENT);
    ModuleScreen::drawAntialiasedText06(cellPercentFormatted, RECT_BOT, LIMIT_POS_X - 14 - CHAR_DIM_X6 * cellPercentFormatted.length(), TEXT_OFFSET_Y, EPD_BLACK);

    // main battery frame
    ModuleScreen::fillRectangle(RECT_NRG, EPD_LIGHT);

    // percentage
    uint8_t yMin = RECT_NRG.ymax - 3 - (uint8_t)round((RECT_NRG.ymax - RECT_NRG.ymin - 3) * percent * 0.01f);
    fillRectangle({RECT_NRG.xmin, yMin, RECT_NRG.xmax, RECT_NRG.ymax}, EPD_BLACK);

    // battery contact clip
    ModuleScreen::baseDisplay.drawFastHLine(RECT_NRG.xmin + 1, RECT_NRG.ymin + 1, 2, EPD_WHITE);
    ModuleScreen::baseDisplay.drawFastHLine(RECT_NRG.xmax - 3, RECT_NRG.ymin + 1, 2, EPD_WHITE);

    int charPosFooter = 7;
    if (config->isBeep) {
        ModuleScreen::drawAntialiasedText08(SYMBOL_YBEEP, RECT_BOT, 6, TEXT_OFFSET_Y + 1, EPD_BLACK);
        charPosFooter += 13;
    }

    if (config->wifi.isActive && ModuleWifi::isConnected()) {
        String address = ModuleWifi::getAddress();
        ModuleScreen::drawAntialiasedText06(address, RECT_BOT, charPosFooter, TEXT_OFFSET_Y, EPD_BLACK);
        ModuleScreen::drawAntialiasedText06(",", RECT_BOT, charPosFooter + address.length() * CHAR_DIM_X6, TEXT_OFFSET_Y, EPD_BLACK);
        charPosFooter += (address.length() + 1) * CHAR_DIM_X6;
    }
    ModuleScreen::drawAntialiasedText06(ModuleClock::getDateTimeDisplayString(ModuleClock::getSecondstime()), RECT_BOT, charPosFooter, TEXT_OFFSET_Y, EPD_BLACK);
}

void ModuleScreen::renderEntry(config_t *config) {
    ModuleScreen::clearBuffer(config);
    ModuleScreen::drawOuterBorders(EPD_LIGHT);

    drawAntialiasedText18("moth", RECT_TOP, 8, 98, EPD_BLACK);
    drawAntialiasedText06(VNUM, RECT_TOP, 105, 98, EPD_BLACK);

    // skip header for clean screen
    ModuleScreen::renderFooter(config);

    ModuleScreen::flushBuffer();
}

void ModuleScreen::drawAntialiasedText06(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color) {
    drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb06pt_l, &smb06pt_d, &smb06pt_b);
}

void ModuleScreen::drawAntialiasedText08(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color) {
    drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb08pt_l, &smb08pt_d, &smb08pt_b);
}

void ModuleScreen::drawAntialiasedText18(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color) {
    drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb18pt_l, &smb18pt_d, &smb18pt_b);
}

void ModuleScreen::drawAntialiasedText36(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color) {
    drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb36pt_l, &smb36pt_d, &smb36pt_b);
}

/**
 * when switching fonts, setFont(...) must be called before setCursor(...), or there may be a y-offset on the very first text
 */
void ModuleScreen::drawAntialiasedText(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color, const GFXfont *fontL, const GFXfont *fontD, const GFXfont *fontB) {
    ModuleScreen::baseDisplay.setFont(fontL);
    ModuleScreen::baseDisplay.setCursor(rectangle.xmin + xRel, rectangle.ymin + yRel);
    ModuleScreen::baseDisplay.setTextColor(color == EPD_BLACK ? EPD_LIGHT : EPD_DARK);
    ModuleScreen::baseDisplay.print(text);

    ModuleScreen::baseDisplay.setFont(fontD);
    ModuleScreen::baseDisplay.setCursor(rectangle.xmin + xRel, rectangle.ymin + yRel);
    ModuleScreen::baseDisplay.setTextColor(color == EPD_BLACK ? EPD_DARK : EPD_LIGHT);
    ModuleScreen::baseDisplay.print(text);

    ModuleScreen::baseDisplay.setFont(fontB);
    ModuleScreen::baseDisplay.setCursor(rectangle.xmin + xRel, rectangle.ymin + yRel);
    ModuleScreen::baseDisplay.setTextColor(color);
    ModuleScreen::baseDisplay.print(text);
}

void ModuleScreen::drawOuterBorders(uint16_t color) {
    // top
    ModuleScreen::baseDisplay.drawFastHLine(0, 0, 296, color);
    ModuleScreen::baseDisplay.drawFastHLine(0, 1, 296, color);
    // header
    ModuleScreen::baseDisplay.drawFastHLine(0, 21, 296, color);
    ModuleScreen::baseDisplay.drawFastHLine(0, 22, 296, color);
    // footer
    ModuleScreen::baseDisplay.drawFastHLine(0, 105, 296, color);
    ModuleScreen::baseDisplay.drawFastHLine(0, 106, 296, color);
    // bottom
    ModuleScreen::baseDisplay.drawFastHLine(0, 126, 296, color);
    ModuleScreen::baseDisplay.drawFastHLine(0, 127, 296, color);
    // left
    ModuleScreen::baseDisplay.drawFastVLine(0, 0, 128, color);
    ModuleScreen::baseDisplay.drawFastVLine(1, 0, 128, color);
    // right
    ModuleScreen::baseDisplay.drawFastVLine(294, 0, 128, color);
    ModuleScreen::baseDisplay.drawFastVLine(295, 0, 128, color);
}

void ModuleScreen::drawInnerBorders(uint16_t color) {
    // horizontal center
    ModuleScreen::baseDisplay.drawFastHLine(206, 63, 100, color);
    ModuleScreen::baseDisplay.drawFastHLine(206, 64, 100, color);
    // vertical center
    ModuleScreen::baseDisplay.drawFastVLine(206, 21, 86, color);
    ModuleScreen::baseDisplay.drawFastVLine(207, 22, 86, color);
}

void ModuleScreen::fillRectangle(rectangle_t rectangle, uint8_t color) {
    ModuleScreen::baseDisplay.fillRect(rectangle.xmin + 1, rectangle.ymin + 1, rectangle.xmax - rectangle.xmin - 2, rectangle.ymax - rectangle.ymin - 2, color);
}

bool ModuleScreen::isWarn(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi) {
    return value < warnLo || value >= wHi;
}

bool ModuleScreen::isRisk(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi) {
    return value < rLo || value >= rHi;
}

uint8_t ModuleScreen::getTextColor(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi) {
    if (ModuleScreen::isRisk(value, rLo, warnLo, wHi, rHi)) {
        return EPD_WHITE;
    } else {
        return EPD_BLACK;
    }
}

uint8_t ModuleScreen::getFillColor(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi) {
    if (ModuleScreen::isRisk(value, rLo, warnLo, wHi, rHi)) {
        return EPD_BLACK;
    } else if (ModuleScreen::isWarn(value, rLo, warnLo, wHi, rHi)) {
        return EPD_LIGHT;
    } else {
        return EPD_WHITE;
    }
}

uint8_t ModuleScreen::getVertColor(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi) {
    if (ModuleScreen::isRisk(value, rLo, warnLo, wHi, rHi)) {
        return EPD_LIGHT;
    } else {
        return EPD_DARK;
    }
}

String ModuleScreen::formatString(String value, char const *format) {
    char padBuffer[16];
    sprintf(padBuffer, format, value);
    return padBuffer;
}

float ModuleScreen::celsiusToFahrenheit(float celsius) {
    return celsius * 9.0f / 5.0f + 32.0f;
}
