#include "ModuleDisplay.h"

#include <qrcode.h>

#include "buttons/ButtonAction.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"
#include "sensors/SensorTime.h"
#include "types/Define.h"

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

ModuleDisplayBase ModuleDisplay::baseDisplay;
bool ModuleDisplay::interrupted = false;

void ModuleDisplay::begin() {
    // do nothing
}

void ModuleDisplay::prepareSleep(wakeup_action_e wakeupType) {
    if (wakeupType == WAKEUP_ACTION_BUSY) {
        esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_EPD_BUSY, HIGH);
    }
}

void ModuleDisplay::attachWakeup(wakeup_action_e wakeupType) {
    if (wakeupType == WAKEUP_ACTION_BUSY) {
        attachInterrupt(digitalPinToInterrupt(PIN_EPD_BUSY), ModuleDisplay::handleInterrupt, FALLING);
        ModuleDisplay::interrupted = false;
    }
}
void ModuleDisplay::detachWakeup(wakeup_action_e wakeupType) {
    if (wakeupType == WAKEUP_ACTION_BUSY) {
        detachInterrupt(digitalPinToInterrupt(PIN_EPD_BUSY));
        ModuleDisplay::interrupted = false;
    }
}

void ModuleDisplay::handleInterrupt() {
    ModuleDisplay::interrupted = true;
}

bool ModuleDisplay::isInterrupted() {
    return ModuleDisplay::interrupted;
}

void ModuleDisplay::flushBuffer() {
    ModuleDisplay::baseDisplay.writeFrameBuffers();
}

void ModuleDisplay::depower() {
    ModuleDisplay::baseDisplay.begin(THINKINK_GRAYSCALE4, true);
    ModuleDisplay::baseDisplay.depower();
}

void ModuleDisplay::clearBuffer(config_t& config) {
    ModuleDisplay::baseDisplay.begin(THINKINK_GRAYSCALE4, config.disp.displayValTheme == DISPLAY_THM____LIGHT);
    ModuleDisplay::baseDisplay.clearBuffer();
}

void ModuleDisplay::renderTable(values_all_t& measurement, config_t& config) {
    ModuleDisplay::clearBuffer(config);
    ModuleDisplay::drawOuterBorders(EPD_LIGHT);
    ModuleDisplay::drawInnerBorders(EPD_LIGHT);

    // values for temperature and humidity
    float deg = SensorScd041::toFloatDeg(measurement.valuesCo2.deg);
    float hum = SensorScd041::toFloatHum(measurement.valuesCo2.hum);

    // variables needed
    String title;
    uint8_t textColor;
    uint8_t fillColor;
    uint8_t vertColor;
    uint16_t xPosMainValue = 36;
    uint16_t charPosValueX = 193;
    uint16_t charPosFinalX;

    if (config.disp.displayValTable == DISPLAY_VAL_T____CO2) {

        thresholds_co2_t thresholdsCo2 = config.disp.thresholdsCo2;
        uint16_t co2Lpf = measurement.valuesCo2.co2Lpf;

        float stale = max(0.0, min(10.0, (co2Lpf - thresholdsCo2.ref) / 380.0));  // don't allow negative stale values. max out at 10
        float staleWarn = max(0.0, min(10.0, (thresholdsCo2.wHi - thresholdsCo2.ref) / 380.0));
        float staleRisk = max(0.0, min(10.0, (thresholdsCo2.rHi - thresholdsCo2.ref) / 380.0));

        // #ifdef USE___SERIAL
        //         Serial.printf("co2Lpf %d, thresholdsCo2.ref: %d, stale: %f\n", co2Lpf, thresholdsCo2.ref, stale);
        // #endif

        float staleMax = 2.5;
        if (stale > 4) {
            staleMax = 10;
        } else if (stale > 2) {
            staleMax = 5;
        }
        int staleDif = 70 / staleMax;  // height of 1 percent, 20 for staleMax 2.5, 10 for staleMax 5, 5 for staleMax 10

        textColor = getTextColor(co2Lpf, 0, 0, thresholdsCo2.wHi, thresholdsCo2.rHi);
        fillColor = getFillColor(co2Lpf, 0, 0, thresholdsCo2.wHi, thresholdsCo2.rHi);
        vertColor = getVertColor(co2Lpf, 0, 0, thresholdsCo2.wHi, thresholdsCo2.rHi);

        if (fillColor != EPD_WHITE) {
            ModuleDisplay::fillRectangle(RECT_CO2, fillColor);
        }

        title = "CO² ppm";
        charPosFinalX = charPosValueX - CHAR_DIM_X6 * 7;
        ModuleDisplay::drawAntialiasedText36(formatString(String(co2Lpf), FORMAT_4_DIGIT), RECT_CO2, xPosMainValue, 76, textColor);

        uint8_t yMax = RECT_CO2.ymax - 7;  // bottom limit of rebreathe indicator
        uint8_t yDim = round(stale * staleDif);
        uint8_t yDimWarn = round(staleWarn * staleDif);
        uint8_t yDimRisk = round(staleRisk * staleDif);
        int yDimMax = round(staleMax * staleDif);

        uint8_t xBarMin = 8;
        uint8_t xBarMax = 13;
        for (uint8_t i = xBarMin; i <= xBarMax; i++) {
            ModuleDisplay::baseDisplay.drawFastVLine(i, yMax - yDimWarn, yDimWarn, EPD_WHITE);             // good
            ModuleDisplay::baseDisplay.drawFastVLine(i, yMax - yDimRisk, yDimRisk - yDimWarn, vertColor);  // warn
            ModuleDisplay::baseDisplay.drawFastVLine(i, yMax - yDimMax, yDimMax - yDimRisk, EPD_BLACK);    // risk
        }

        if (ModuleDisplay::isRisk(co2Lpf, 0, 0, thresholdsCo2.wHi, thresholdsCo2.rHi)) {  // when in risk, the risk area needs to be outlined
            ModuleDisplay::baseDisplay.drawFastVLine(xBarMin, yMax - yDimMax, yDimMax - yDimRisk, vertColor);
            ModuleDisplay::baseDisplay.drawFastVLine(xBarMax, yMax - yDimMax, yDimMax - yDimRisk, vertColor);
            ModuleDisplay::baseDisplay.drawFastHLine(xBarMin, yMax - yDimMax, xBarMax - xBarMin + 1, vertColor);
        } else if (!ModuleDisplay::isWarn(co2Lpf, 0, 0, thresholdsCo2.wHi, thresholdsCo2.rHi)) {  // when not warn and not risk, the good area needs to be outlined
            ModuleDisplay::baseDisplay.drawFastVLine(xBarMin, yMax - yDimWarn, yDimWarn, vertColor);
            ModuleDisplay::baseDisplay.drawFastVLine(xBarMax, yMax - yDimWarn, yDimWarn, vertColor);
            ModuleDisplay::baseDisplay.drawFastHLine(xBarMin, yMax, xBarMax - xBarMin + 1, vertColor);
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
        } else if (co2Lpf >= 1000) {  // wrap percent, shorten line
            yPrc += 14;
            xPrc = xBarMax + 2;
            xLin -= CHAR_DIM_X6;
        }

        // draw a narrowed percentage number
        ModuleDisplay::baseDisplay.drawFastHLine(xBarMax + 3, yMax - yDim, xLin, textColor);
        ModuleDisplay::drawAntialiasedText06(String(staleFix), RECT_CO2, xBarMax + 2, yTxt, textColor);
        if (stale < 10) {
            ModuleDisplay::drawAntialiasedText06(".", RECT_CO2, xBarMax + 7, yTxt, textColor);
            ModuleDisplay::drawAntialiasedText06(String(staleFrc), RECT_CO2, xBarMax + 12, yTxt, textColor);
        }
        ModuleDisplay::drawAntialiasedText06("%", RECT_CO2, xPrc, yPrc, textColor);

    } else if (config.disp.displayValTable == DISPLAY_VAL_T____HPA) {
        float pressure = measurement.valuesBme.pressure;
        textColor = EPD_BLACK;
        fillColor = EPD_WHITE;
        vertColor = EPD_DARK;
        ModuleDisplay::fillRectangle(RECT_CO2, fillColor);
        title = "pressure hPa";
        charPosFinalX = charPosValueX - CHAR_DIM_X6 * title.length();
        ModuleDisplay::drawAntialiasedText36(formatString(String(pressure, 0), FORMAT_4_DIGIT), RECT_CO2, xPosMainValue, 76, textColor);
    } else if (config.disp.displayValTable == DISPLAY_VAL_T____ALT) {
        float altitude = SensorBme280::getAltitude(config.pressureZerolevel, measurement.valuesBme.pressure);
        textColor = EPD_BLACK;
        fillColor = EPD_WHITE;
        vertColor = EPD_DARK;
        ModuleDisplay::fillRectangle(RECT_CO2, fillColor);
        title = "altitude m";
        charPosFinalX = charPosValueX - CHAR_DIM_X6 * title.length();
        ModuleDisplay::drawAntialiasedText36(formatString(String(altitude, 0), FORMAT_4_DIGIT), RECT_CO2, xPosMainValue, 76, textColor);
    } else {
#ifdef USE___SERIAL
        Serial.printf("unhandled displayValTable: %d\n", config.disp.displayValTable);
#endif
    }
    ModuleDisplay::drawAntialiasedText06(title, RECT_CO2, charPosFinalX, TEXT_OFFSET_Y, textColor);

    thresholds_deg_t thresholdsDeg = config.disp.thresholdsDeg;
    textColor = getTextColor(deg, thresholdsDeg.rLo, thresholdsDeg.wLo, thresholdsDeg.wHi, thresholdsDeg.rHi);
    fillColor = getFillColor(deg, thresholdsDeg.rLo, thresholdsDeg.wLo, thresholdsDeg.wHi, thresholdsDeg.rHi);
    if (config.disp.displayDegScale == DISPLAY_VAL_D______F) {
        deg = SensorScd041::toFahrenheit(deg);
    }
    int temperature10 = round(deg * 10.0);
    int temperatureFix = floor(temperature10 / 10.0);
    int temperatureFrc = abs(temperature10 % 10);
    if (fillColor != EPD_WHITE) {
        ModuleDisplay::fillRectangle(RECT_DEG, fillColor);
    }
    ModuleDisplay::drawAntialiasedText18(formatString(String(temperatureFix), FORMAT_3_DIGIT), RECT_DEG, 0, 35, textColor);
    ModuleDisplay::drawAntialiasedText06(config.disp.displayDegScale == DISPLAY_VAL_D______F ? "°F" : "°C", RECT_DEG, 80 - CHAR_DIM_X6 * 2, TEXT_OFFSET_Y + 2, textColor);
    ModuleDisplay::drawAntialiasedText08(".", RECT_DEG, 63, 35, textColor);
    ModuleDisplay::drawAntialiasedText08(String(temperatureFrc), RECT_DEG, 72, 35, textColor);

    thresholds_hum_t thresholdsHum = config.disp.thresholdsHum;
    textColor = getTextColor(hum, thresholdsHum.rLo, thresholdsHum.wLo, thresholdsHum.wHi, thresholdsHum.rHi);
    fillColor = getFillColor(hum, thresholdsHum.rLo, thresholdsHum.wLo, thresholdsHum.wHi, thresholdsHum.rHi);
    int humidity10 = round(hum * 10.0);
    int humidityFix = floor(humidity10 / 10.0);
    int humidityFrc = abs(humidity10 % 10);
    if (fillColor != EPD_WHITE) {
        ModuleDisplay::fillRectangle(RECT_HUM, fillColor);
    }
    ModuleDisplay::drawAntialiasedText18(formatString(String(humidityFix), FORMAT_3_DIGIT), RECT_HUM, 0, 35, textColor);
    ModuleDisplay::drawAntialiasedText06("%", RECT_HUM, 80 - CHAR_DIM_X6 * 1, TEXT_OFFSET_Y + 2, textColor);
    ModuleDisplay::drawAntialiasedText08(".", RECT_HUM, 63, 35, textColor);
    ModuleDisplay::drawAntialiasedText08(String(humidityFrc), RECT_HUM, 72, 35, textColor);

    ModuleDisplay::renderHeader();
    ModuleDisplay::renderFooter(config);

    ModuleDisplay::flushBuffer();
}

/**
 * measurement is reference for building the measurement table backwards
 */
void ModuleDisplay::renderChart(values_all_t history[60], config_t& config) {
    ModuleDisplay::clearBuffer(config);
    ModuleDisplay::drawOuterBorders(EPD_LIGHT);

    display_val_c_e displayValChart = config.disp.displayValChart;

    values_all_t measurement;
    uint16_t minValue = 0;
    uint16_t maxValue = 1500;
    if (displayValChart == DISPLAY_VAL_C____CO2) {
        for (uint8_t i = 0; i < HISTORY_____BUFFER_SIZE; i++) {
            measurement = history[i];
            if (measurement.valuesCo2.co2Lpf > 3600) {
                maxValue = 6000;  // 0, 2000, 4000, (6000)
            } else if (measurement.valuesCo2.co2Lpf > 2400) {
                maxValue = 4500;  // 0, 1500, 3000, (4500)
            } else if (measurement.valuesCo2.co2Lpf > 1200) {
                maxValue = 3000;  // 0, 1000, 2000, (3000)
            }
        }
    } else if (displayValChart == DISPLAY_VAL_C____DEG) {
        if (config.disp.displayDegScale == DISPLAY_VAL_D______F) {
            minValue = 60;
            maxValue = 120;
        } else {
            minValue = 10;
            maxValue = 40;
        }
    } else if (displayValChart == DISPLAY_VAL_C____HUM) {
        minValue = 20;
        maxValue = 80;  // 20, 40, 60, (80)
    } else if (displayValChart == DISPLAY_VAL_C____HPA) {
        double pressureAvg = 0;
        uint8_t validValueCount = 0;
        for (uint8_t i = 0; i < HISTORY_____BUFFER_SIZE; i++) {
            measurement = history[i];
            if (measurement.valuesBme.pressure > 0) {
                pressureAvg += measurement.valuesBme.pressure;
                validValueCount++;
            }
        }
        pressureAvg /= validValueCount;                  // lets say it is 998
        minValue = floor(pressureAvg / 10.0) * 10 - 10;  // 99.8 -> 99 -> 990 -> 980
        maxValue = ceil(pressureAvg / 10.0) * 10 + 10;   // 99.8 -> 100 -> 1000 -> 1010
    } else if (displayValChart == DISPLAY_VAL_C____ALT) {
        maxValue = 450;
        float altitude;
        for (uint8_t i = 0; i < HISTORY_____BUFFER_SIZE; i++) {
            measurement = history[i];
            if (measurement.valuesBme.pressure > 0) {
                altitude = SensorBme280::getAltitude(config.pressureZerolevel, measurement.valuesBme.pressure);
            } else {
                altitude = 0.0f;
            }
            if (altitude > 1600) {
                maxValue = 3600;  // 0, 1200, 2400, 3600
            } else if (altitude > 800) {
                maxValue = 1800;  // 0, 600, 1200, (1800)
            } else if (altitude > 400) {
                maxValue = 900;  // 0, 300, 600, (900)
            }
        }
    } else if (displayValChart == DISPLAY_VAL_C____NRG) {
        minValue = 0;
        maxValue = 150;  // 0, 50, 100, (150)
    }

    String label2 = String(minValue + (maxValue - minValue) * 2 / 3);
    String label1 = String(minValue + (maxValue - minValue) / 3);
    String label0 = String(minValue);

    uint8_t charPosValueX = 41;
    uint8_t charPosLabelY = 12;

    ModuleDisplay::drawAntialiasedText06(label2, RECT_CO2, charPosValueX - CHAR_DIM_X6 * label2.length(), 24, EPD_BLACK);
    ModuleDisplay::baseDisplay.drawFastHLine(1, 49, 296, EPD_LIGHT);
    ModuleDisplay::drawAntialiasedText06(label1, RECT_CO2, charPosValueX - CHAR_DIM_X6 * label1.length(), 52, EPD_BLACK);
    ModuleDisplay::baseDisplay.drawFastHLine(1, 77, 296, EPD_LIGHT);
    ModuleDisplay::drawAntialiasedText06(label0, RECT_CO2, charPosValueX - CHAR_DIM_X6 * label0.length(), 80, EPD_BLACK);

    String title;

    if (displayValChart == DISPLAY_VAL_C____CO2) {
        title = "CO² ppm," + String(config.disp.displayHrsChart) + "h";  // the sup 2 has been modified in the font to display as sub
    } else if (displayValChart == DISPLAY_VAL_C____DEG) {
        title = config.disp.displayDegScale == DISPLAY_VAL_D______F ? "temperature °F," : "temperature °C," + String(config.disp.displayHrsChart) + "h";
    } else if (displayValChart == DISPLAY_VAL_C____HUM) {
        title = "humidity %," + String(config.disp.displayHrsChart) + "h";
    } else if (displayValChart == DISPLAY_VAL_C____HPA) {
        title = "pressure hPa," + String(config.disp.displayHrsChart) + "h";
    } else if (displayValChart == DISPLAY_VAL_C____ALT) {
        title = "altitude m," + String(config.disp.displayHrsChart) + "h";
    } else if (displayValChart == DISPLAY_VAL_C____NRG) {
        title = "battery %," + String(config.disp.displayHrsChart) + "h";
    }
    ModuleDisplay::drawAntialiasedText06(title, RECT_CO2, LIMIT_POS_X - title.length() * CHAR_DIM_X6 + 3, charPosLabelY, EPD_BLACK);

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

        if (displayValChart == DISPLAY_VAL_C____CO2) {
            curValue = measurement.valuesCo2.co2Lpf;
        } else if (displayValChart == DISPLAY_VAL_C____DEG) {
            curValue = SensorScd041::toFloatDeg(measurement.valuesCo2.deg);
            if (config.disp.displayDegScale == DISPLAY_VAL_D______F) {
                curValue = SensorScd041::toFahrenheit(curValue);
            }
        } else if (displayValChart == DISPLAY_VAL_C____HUM) {
            curValue = SensorScd041::toFloatHum(measurement.valuesCo2.hum);
        } else if (displayValChart == DISPLAY_VAL_C____HPA) {
            curValue = measurement.valuesBme.pressure;
        } else if (displayValChart == DISPLAY_VAL_C____ALT) {
            if (measurement.valuesBme.pressure > 0) {
                curValue = SensorBme280::getAltitude(config.pressureZerolevel, measurement.valuesBme.pressure);
            } else {
                curValue = 0.0f;
            }
        } else if (displayValChart == DISPLAY_VAL_C____NRG) {
            curValue = SensorEnergy::toFloatPercent(measurement.valuesNrg.percent);
        }

        minX = basX + i * displayableBarWidth;
        dimY = max(0, min((int)limY, (int)round((curValue - minValue) * limY / (maxValue - minValue))));
        minY = maxY - dimY;

        if (dimY > 0) {
            for (int b = 0; b < displayableBarWidth - 1; b++) {
                baseDisplay.drawFastVLine(minX + b, minY, dimY, EPD_BLACK);
            }
        } else {
            // indicator lines
            // baseDisplay.drawFastHLine(minX, minY - 1, displayableBarWidth - 1, EPD_BLACK);
        }
    }

    ModuleDisplay::renderHeader();
    ModuleDisplay::renderFooter(config);

    ModuleDisplay::flushBuffer();
}

void ModuleDisplay::renderHeader() {
    renderButton(ButtonAction::A.buttonAction, 6);
    renderButton(ButtonAction::B.buttonAction, 135);
    renderButton(ButtonAction::C.buttonAction, 264);
}

void ModuleDisplay::renderButton(button_action_t buttonAction, uint16_t x) {
    ModuleDisplay::drawAntialiasedText08(buttonAction.symbolSlow, RECT_TOP, x, TEXT_OFFSET_Y, EPD_BLACK);
    ModuleDisplay::drawAntialiasedText08(buttonAction.symbolFast, RECT_TOP, x + CHAR_DIM_X6 * 2, TEXT_OFFSET_Y, EPD_BLACK);
    ModuleDisplay::drawAntialiasedText06(buttonAction.extraLabel, RECT_TOP, x + CHAR_DIM_X6 * 4, TEXT_OFFSET_Y - 2, EPD_BLACK);
}

void ModuleDisplay::renderFooter(config_t& config) {
    float percent = SensorEnergy::toFloatPercent(SensorEnergy::readval().percent);

    String cellPercentFormatted = formatString(String(percent, 0), FORMAT_CELL_PERCENT);
    ModuleDisplay::drawAntialiasedText06(cellPercentFormatted, RECT_BOT, LIMIT_POS_X - 14 - CHAR_DIM_X6 * cellPercentFormatted.length(), TEXT_OFFSET_Y, EPD_BLACK);

    // main battery frame
    ModuleDisplay::fillRectangle(RECT_NRG, EPD_LIGHT);

    // percentage
    uint8_t yMin = RECT_NRG.ymax - 3 - (uint8_t)round((RECT_NRG.ymax - RECT_NRG.ymin - 3) * percent * 0.01f);
    fillRectangle({RECT_NRG.xmin, yMin, RECT_NRG.xmax, RECT_NRG.ymax}, EPD_BLACK);

    // battery contact clip
    ModuleDisplay::baseDisplay.drawFastHLine(RECT_NRG.xmin + 1, RECT_NRG.ymin + 1, 2, EPD_WHITE);
    ModuleDisplay::baseDisplay.drawFastHLine(RECT_NRG.xmax - 3, RECT_NRG.ymin + 1, 2, EPD_WHITE);

    int charPosFooter = 7;
    if (config.sign.signalValSound == SIGNAL__VAL______ON) {
        ModuleDisplay::drawAntialiasedText08(SYMBOL_YBEEP, RECT_BOT, 6, TEXT_OFFSET_Y + 1, EPD_BLACK);
        charPosFooter += 13;
    }

    if (config.wifi.wifiValPower == WIFI____VAL_P_CUR_Y) {
        String address = ModuleWifi::getAddress();
        ModuleDisplay::drawAntialiasedText06(address, RECT_BOT, charPosFooter, TEXT_OFFSET_Y, EPD_BLACK);
        ModuleDisplay::drawAntialiasedText06(",", RECT_BOT, charPosFooter + address.length() * CHAR_DIM_X6, TEXT_OFFSET_Y, EPD_BLACK);
        charPosFooter += (address.length() + 1) * CHAR_DIM_X6;
    }
    ModuleDisplay::drawAntialiasedText06(SensorTime::getDateTimeDisplayString(SensorTime::getSecondstime()), RECT_BOT, charPosFooter, TEXT_OFFSET_Y, EPD_BLACK);
}

void ModuleDisplay::renderEntry(config_t& config) {
    ModuleDisplay::clearBuffer(config);
    ModuleDisplay::drawOuterBorders(EPD_LIGHT);

    drawAntialiasedText18("moth", RECT_TOP, 8, 98, EPD_BLACK);
    drawAntialiasedText06(VNUM, RECT_TOP, 105, 98, EPD_BLACK);

    // skip header for clean screen
    ModuleDisplay::renderFooter(config);

    ModuleDisplay::flushBuffer();
}

void ModuleDisplay::renderCo2(config_t& config, calibration_t calibration) {

    ModuleDisplay::clearBuffer(config);
    ModuleDisplay::drawOuterBorders(EPD_LIGHT);

    char titleBuf[32];
    sprintf(titleBuf, "%s (%s)", calibration.action == ACTION___CALIBRATION ? "CALIBRATION" : "RESET", calibration.success ? "success" : "failure");

    drawAntialiasedText08(String(titleBuf), RECT_TOP, 8, 40, EPD_BLACK);
    if (calibration.action == ACTION___CALIBRATION) {
        drawAntialiasedText08("req: ", RECT_TOP, 8, 60, EPD_BLACK);
        drawAntialiasedText08(String(calibration.requestedCo2Ref), RECT_TOP, 50, 60, EPD_BLACK);
        drawAntialiasedText08("cor: ", RECT_TOP, 8, 78, EPD_BLACK);
        drawAntialiasedText08(String(calibration.correctedCo2Ref), RECT_TOP, 50, 78, EPD_BLACK);
        drawAntialiasedText08("off: ", RECT_TOP, 8, 96, EPD_BLACK);
        drawAntialiasedText08(String(calibration.calibrationResult), RECT_TOP, 50, 96, EPD_BLACK);
    }

    ModuleDisplay::renderHeader();
    ModuleDisplay::renderFooter(config);

    ModuleDisplay::flushBuffer();
}

void ModuleDisplay::renderQRCodes(config_t& config) {
    ModuleDisplay::clearBuffer(config);
    ModuleDisplay::drawOuterBorders(EPD_LIGHT);

    // either http://ap_ip/login or [PREF_A_WIFI] + IP
    String address = ModuleWifi::getRootUrl();
    String networkName = ModuleWifi::getNetworkName();
    String networkPass = ModuleWifi::getNetworkPass();

    int qrCodeX = 12;
    int qrCodeY = 35;

    // render the network connection link
    if (networkName != "") {
        char networkBuf[networkName.length() + 1];
        networkName.toCharArray(networkBuf, networkName.length() + 1);

        QRCode qrcodeNetwork;
        uint8_t qrcodeDataNetwork[qrcode_getBufferSize(3)];
        qrcode_initText(&qrcodeNetwork, qrcodeDataNetwork, 3, 0, networkBuf);

        for (uint8_t y = 0; y < qrcodeNetwork.size; y++) {
            for (uint8_t x = 0; x < qrcodeNetwork.size; x++) {
                if (qrcode_getModule(&qrcodeNetwork, x, y)) {
                    baseDisplay.fillRect(x * 2 + qrCodeX, y * 2 + qrCodeY, 2, 2, EPD_BLACK);
                }
            }
        }

        qrCodeX = 228;

        ModuleDisplay::drawAntialiasedText06("wlan", RECT_TOP, 75, 45, EPD_BLACK);
        if (networkPass != "") {
            ModuleDisplay::drawAntialiasedText06(ModuleWifi::getNetworkPass(), RECT_TOP, 75, 60, EPD_BLACK);
        }
        ModuleDisplay::drawAntialiasedText06("open", RECT_TOP, 193, 89, EPD_BLACK);
    }

    char addressBuf[address.length() + 1];
    address.toCharArray(addressBuf, address.length() + 1);

    QRCode qrcodeAddress;
    uint8_t qrcodeDataAddress[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcodeAddress, qrcodeDataAddress, 3, 0, addressBuf);

    // render the ip address within the network
    for (uint8_t y = 0; y < qrcodeAddress.size; y++) {
        for (uint8_t x = 0; x < qrcodeAddress.size; x++) {
            if (qrcode_getModule(&qrcodeAddress, x, y)) {
                baseDisplay.fillRect(x * 2 + qrCodeX, y * 2 + qrCodeY, 2, 2, EPD_BLACK);
            }
        }
    }

    ModuleDisplay::renderHeader();
    ModuleDisplay::renderFooter(config);

    ModuleDisplay::flushBuffer();
}

void ModuleDisplay::drawAntialiasedText06(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color) {
    drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb06pt_l, &smb06pt_d, &smb06pt_b);
}

void ModuleDisplay::drawAntialiasedText08(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color) {
    drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb08pt_l, &smb08pt_d, &smb08pt_b);
}

void ModuleDisplay::drawAntialiasedText18(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color) {
    drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb18pt_l, &smb18pt_d, &smb18pt_b);
}

void ModuleDisplay::drawAntialiasedText36(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color) {
    drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb36pt_l, &smb36pt_d, &smb36pt_b);
}

/**
 * when switching fonts, setFont(...) must be called before setCursor(...), or there may be a y-offset on the very first text
 */
void ModuleDisplay::drawAntialiasedText(String text, rectangle_t rectangle, int xRel, int yRel, uint8_t color, const GFXfont* fontL, const GFXfont* fontD, const GFXfont* fontB) {
    ModuleDisplay::baseDisplay.setFont(fontL);
    ModuleDisplay::baseDisplay.setCursor(rectangle.xmin + xRel, rectangle.ymin + yRel);
    ModuleDisplay::baseDisplay.setTextColor(color == EPD_BLACK ? EPD_LIGHT : EPD_DARK);
    ModuleDisplay::baseDisplay.print(text);

    ModuleDisplay::baseDisplay.setFont(fontD);
    ModuleDisplay::baseDisplay.setCursor(rectangle.xmin + xRel, rectangle.ymin + yRel);
    ModuleDisplay::baseDisplay.setTextColor(color == EPD_BLACK ? EPD_DARK : EPD_LIGHT);
    ModuleDisplay::baseDisplay.print(text);

    ModuleDisplay::baseDisplay.setFont(fontB);
    ModuleDisplay::baseDisplay.setCursor(rectangle.xmin + xRel, rectangle.ymin + yRel);
    ModuleDisplay::baseDisplay.setTextColor(color);
    ModuleDisplay::baseDisplay.print(text);
}

void ModuleDisplay::drawOuterBorders(uint16_t color) {
    // top
    ModuleDisplay::baseDisplay.drawFastHLine(0, 0, 296, color);
    ModuleDisplay::baseDisplay.drawFastHLine(0, 1, 296, color);
    // header
    ModuleDisplay::baseDisplay.drawFastHLine(0, 21, 296, color);
    ModuleDisplay::baseDisplay.drawFastHLine(0, 22, 296, color);
    // footer
    ModuleDisplay::baseDisplay.drawFastHLine(0, 105, 296, color);
    ModuleDisplay::baseDisplay.drawFastHLine(0, 106, 296, color);
    // bottom
    ModuleDisplay::baseDisplay.drawFastHLine(0, 126, 296, color);
    ModuleDisplay::baseDisplay.drawFastHLine(0, 127, 296, color);
    // left
    ModuleDisplay::baseDisplay.drawFastVLine(0, 0, 128, color);
    ModuleDisplay::baseDisplay.drawFastVLine(1, 0, 128, color);
    // right
    ModuleDisplay::baseDisplay.drawFastVLine(294, 0, 128, color);
    ModuleDisplay::baseDisplay.drawFastVLine(295, 0, 128, color);
}

void ModuleDisplay::drawInnerBorders(uint16_t color) {
    // horizontal center
    ModuleDisplay::baseDisplay.drawFastHLine(206, 63, 100, color);
    ModuleDisplay::baseDisplay.drawFastHLine(206, 64, 100, color);
    // vertical center
    ModuleDisplay::baseDisplay.drawFastVLine(206, 21, 86, color);
    ModuleDisplay::baseDisplay.drawFastVLine(207, 22, 86, color);
}

void ModuleDisplay::fillRectangle(rectangle_t rectangle, uint8_t color) {
    ModuleDisplay::baseDisplay.fillRect(rectangle.xmin + 1, rectangle.ymin + 1, rectangle.xmax - rectangle.xmin - 2, rectangle.ymax - rectangle.ymin - 2, color);
}

bool ModuleDisplay::isWarn(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi) {
    return value < warnLo || value >= wHi;
}

bool ModuleDisplay::isRisk(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi) {
    return value < rLo || value >= rHi;
}

uint8_t ModuleDisplay::getTextColor(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi) {
    if (ModuleDisplay::isRisk(value, rLo, warnLo, wHi, rHi)) {
        return EPD_WHITE;
    } else {
        return EPD_BLACK;
    }
}

uint8_t ModuleDisplay::getFillColor(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi) {
    if (ModuleDisplay::isRisk(value, rLo, warnLo, wHi, rHi)) {
        return EPD_BLACK;
    } else if (ModuleDisplay::isWarn(value, rLo, warnLo, wHi, rHi)) {
        return EPD_LIGHT;
    } else {
        return EPD_WHITE;
    }
}

uint8_t ModuleDisplay::getVertColor(float value, uint16_t rLo, uint16_t warnLo, uint16_t wHi, uint16_t rHi) {
    if (ModuleDisplay::isRisk(value, rLo, warnLo, wHi, rHi)) {
        return EPD_LIGHT;
    } else {
        return EPD_DARK;
    }
}

String ModuleDisplay::formatString(String value, char const* format) {
    char padBuffer[16];
    sprintf(padBuffer, format, value);
    return padBuffer;
}
