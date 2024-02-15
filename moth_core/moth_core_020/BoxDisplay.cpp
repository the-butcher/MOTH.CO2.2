#include "BoxDisplay.h"
#include "BoxPack.h"
#include "BoxClock.h"
#include "BoxFiles.h"
#include "BoxMqtt.h"
#include "BoxConn.h"
#include "Measurements.h"
#include "SensorScd041.h"
#include "SensorBme280.h"
#include "SensorPmsa003i.h"
#include "ButtonHandlers.h"
#include "WiFi.h"
#include <SdFat.h>
#include <ArduinoJson.h>

/**
 * ################################################
 * ## constants
 * ################################################
 */
const String JSON_KEY___MINUTES = "min";
const String JSON_KEY_______SSC = "ssc";
const String JSON_KEY__ALTITUDE = "alt";
const String JSON_KEY____OFFSET = "off";
const String JSON_KEY__TIMEZONE = "tzn";
const String JSON_KEY_______CO2 = "co2";
const String JSON_KEY_______PMS = "pms";
const String JSON_KEY_______DEG = "deg";
const String JSON_KEY_______HUM = "hum";
const String JSON_KEY_RISK__LOW = "rLo";
const String JSON_KEY_WARN__LOW = "wLo";
const String JSON_KEY_WARN_HIGH = "wHi";
const String JSON_KEY_RISK_HIGH = "rHi";
const String JSON_KEY_______C2F = "c2f";
const String JSON_KEY_______COR = "cor";

const char FORMAT_3_DIGIT[] = "%3s";
const char FORMAT_4_DIGIT[] = "%4s";
const char FORMAT_5_DIGIT[] = "%5s";
const char FORMAT_6_DIGIT[] = "%6s";
const char FORMAT_CELL_PERCENT[] = "%5s%%";
const char FORMAT_STALE[] = "%3s%% stale";

const int limitPosX = 287;
const int charDimX6 = 7;
const float zero = 0;
const float one = 1;

const Rectangle RECT_TOP = {
    1,
    1,
    295,
    22
};
const Rectangle RECT_BOT = {
    1,
    106,
    295,
    127
};
const Rectangle RECT_CO2 = {
    1,
    22,
    197,
    106
};
const Rectangle RECT_DEG = {
    197,
    22,
    295,
    64
};
const Rectangle RECT_HUM = {
    197,
    64,
    295,
    106
};
const Rectangle RECT_BAT = {
    RECT_BOT.xmax - 60,
    RECT_BOT.ymin + 7,
    RECT_BOT.xmax - 38,
    RECT_BOT.ymin + 18
};
// const Rectangle RECT__SD = {
//     RECT_TOP.xmax - 50,
//     RECT_TOP.ymin + 9,
//     RECT_TOP.xmax - 40,
//     RECT_TOP.ymin + 18
// };
int TEXT_OFFSET_Y = 16;

/**
 * ################################################
 * ## mutable variables
 * ################################################
 */
display_state_t displayState = DISPLAY_STATE_TABLE;
display_theme_t displayTheme = DISPLAY_THEME_LIGHT;

display_val_t_v displayValueTable = DISPLAY_VAL_T___CO2;
display_val_c_v displayValueChart = DISPLAY_VAL_C___CO2;

int textColor;
int fillColor;
int vertColor;

Thresholds thresholdsCo2;
Thresholds thresholdsPms;
Thresholds thresholdsTemperature;
Thresholds thresholdsHumidity;
Thresholds thresholdsHighlight;

int chartMeasurementCount = 60;
int chartMeasurementHours = 1;
bool ssc = false;
bool c2f = false; // if true show values in fahrenheit
float cor = 0.0;  // if true try to use the bme280 sensor's temperature for display temperature correction

int16_t tbx, tby;
uint16_t tbw, tbh;

const int16_t EPD_DC = 10;
const int16_t EPD_CS = 9;
const int16_t EPD_BUSY = 14; // A4 -> has been solder-connected to the busy pad
const int16_t SRAM_CS = -1;
const int16_t EPD_RESET = SensorPmsa003i::ACTIVE ? 18 : 8; // A0 -> has been bridged to the reset pin

/**
 * ################################################
 * ## static class variabales
 * ################################################
 */
String BoxDisplay::SYMBOL__PM_M = "¦";
String BoxDisplay::SYMBOL__PM_D = "§";
String BoxDisplay::SYMBOL__WIFI = "¥";
String BoxDisplay::SYMBOL_THEME = "¤";
String BoxDisplay::SYMBOL_TABLE = "£";
String BoxDisplay::SYMBOL_CHART = "¢";

String BoxDisplay::CONFIG_PATH = "/config/disp.json";
config_status_t BoxDisplay::configStatus = CONFIG_STATUS_PENDING;
int64_t BoxDisplay::renderStateSeconds = 180; // default :: 3 minutes, will be overridden by config
ThinkInk_290_Grayscale4_T5_Clone BoxDisplay::baseDisplay(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

void BoxDisplay::begin() {
  BoxDisplay::updateConfiguration();
}

void BoxDisplay::flushBuffer() {
  baseDisplay.display(true);
  baseDisplay.hibernate();
}

void BoxDisplay::clearBuffer() {
  baseDisplay.begin(THINKINK_GRAYSCALE4, displayTheme == DISPLAY_THEME_LIGHT);
  baseDisplay.clearBuffer();
}

void BoxDisplay::updateConfiguration() {

  BoxDisplay::configStatus = CONFIG_STATUS_PENDING;

  int co2WarnHi = 800;
  int co2RiskHi = 1000;

  int pmsWarnHi = 15;
  int pmsRiskHi = 50;

  int degRiskLo = 14;
  int degWarnLo = 19;
  int degWarnHi = 25;
  int degRiskHi = 30;

  int humRiskLo = 25;
  int humWarnLo = 30;
  int humWarnHi = 60;
  int humRiskHi = 65;

  int displayUpdateMinutes = 3;
  float temperatureOffsetScd041 = SensorScd041::getTemperatureOffset();
  float temperatureOffsetBme280 = SensorBme280::getTemperatureOffset();
  float altitudeOffsetBme280 = 0;

  String timezone = BoxClock::getTimezone();

  if (BoxFiles::existsPath(BoxDisplay::CONFIG_PATH)) {

    BoxDisplay::configStatus = CONFIG_STATUS_PRESENT;

    File32 dispFile;
    bool fileSuccess = dispFile.open(BoxDisplay::CONFIG_PATH.c_str(), O_RDONLY);
    if (fileSuccess) {

      BoxDisplay::configStatus = CONFIG_STATUS__LOADED;

      StaticJsonBuffer<1024> jsonBuffer;
      JsonObject &root = jsonBuffer.parseObject(dispFile);
      if (root.success()) {

        displayUpdateMinutes = root[JSON_KEY___MINUTES] | displayUpdateMinutes;
        ssc = root[JSON_KEY_______SSC] | ssc; // show significant change
        timezone = root[JSON_KEY__TIMEZONE] | timezone;

        co2WarnHi = root[JSON_KEY_______CO2][JSON_KEY_WARN_HIGH] | co2WarnHi;
        co2RiskHi = root[JSON_KEY_______CO2][JSON_KEY_RISK_HIGH] | co2RiskHi;

        pmsWarnHi = root[JSON_KEY_______PMS][JSON_KEY_WARN_HIGH] | pmsWarnHi;
        pmsRiskHi = root[JSON_KEY_______PMS][JSON_KEY_RISK_HIGH] | pmsRiskHi;

        degRiskLo = root[JSON_KEY_______DEG][JSON_KEY_RISK__LOW] | degRiskLo;
        degWarnLo = root[JSON_KEY_______DEG][JSON_KEY_WARN__LOW] | degWarnLo;
        degWarnHi = root[JSON_KEY_______DEG][JSON_KEY_WARN_HIGH] | degWarnHi;
        degRiskHi = root[JSON_KEY_______DEG][JSON_KEY_RISK_HIGH] | degRiskHi;

        temperatureOffsetScd041 = root[JSON_KEY_______DEG][JSON_KEY____OFFSET][0] | temperatureOffsetScd041;
        temperatureOffsetBme280 = root[JSON_KEY_______DEG][JSON_KEY____OFFSET][1] | temperatureOffsetBme280;
        altitudeOffsetBme280 = root[JSON_KEY__ALTITUDE] | altitudeOffsetBme280;
        c2f = root[JSON_KEY_______DEG][JSON_KEY_______C2F] | c2f;
        cor = root[JSON_KEY_______DEG][JSON_KEY_______COR] | cor;

        humRiskLo = root[JSON_KEY_______HUM][JSON_KEY_RISK__LOW] | humRiskLo;
        humWarnLo = root[JSON_KEY_______HUM][JSON_KEY_WARN__LOW] | humWarnLo;
        humWarnHi = root[JSON_KEY_______HUM][JSON_KEY_WARN_HIGH] | humWarnHi;
        humRiskHi = root[JSON_KEY_______HUM][JSON_KEY_RISK_HIGH] | humRiskHi;

        BoxDisplay::configStatus = CONFIG_STATUS__PARSED;
      }

      dispFile.close();
    }
  } else {
    BoxDisplay::configStatus = CONFIG_STATUS_MISSING;
  }

  thresholdsCo2 = {
      0,
      0,
      co2WarnHi,
      co2RiskHi,
  };
  thresholdsPms = {
      0,
      0,
      pmsWarnHi,
      pmsRiskHi,
  };
  thresholdsTemperature = {
      degRiskLo,
      degWarnLo,
      degWarnHi,
      degRiskHi
  };
  thresholdsHumidity = {
      humRiskLo,
      humWarnLo,
      humWarnHi,
      humRiskHi
  };
  thresholdsHighlight = {
      0,
      0,
      10,
      20
  };

  SensorScd041::setTemperatureOffset(temperatureOffsetScd041);
  SensorBme280::setTemperatureOffset(temperatureOffsetBme280);
  SensorBme280::setAltitudeOffset(altitudeOffsetBme280);
  BoxClock::setTimezone(timezone);

  // min elapsed time before a display state update
  BoxDisplay::renderStateSeconds = max(60, displayUpdateMinutes * 60);
}

bool BoxDisplay::isWarn(float value, Thresholds thresholds) {
  return value < thresholds.warnLo || value > thresholds.warnHi;
}

bool BoxDisplay::isRisk(float value, Thresholds thresholds) {
  return value < thresholds.riskLo || value > thresholds.riskHi;
}

int BoxDisplay::getTextColor(float value, Thresholds thresholds) {
  if (BoxDisplay::isRisk(value, thresholds)) {
    return EPD_WHITE;
  } else {
    return EPD_BLACK;
  }
}

int BoxDisplay::getFillColor(float value, Thresholds thresholds) {
  if (BoxDisplay::isRisk(value, thresholds)) {
    return EPD_BLACK;
  } else if (BoxDisplay::isWarn(value, thresholds)) {
    return EPD_LIGHT;
  } else {
    return EPD_WHITE;
  }
}

int BoxDisplay::getVertColor(float value, Thresholds thresholds) {
  if (BoxDisplay::isRisk(value, thresholds)) {
    return EPD_LIGHT;
  } else {
    return EPD_DARK;
  }
}

void BoxDisplay::fillRectangle(Rectangle rectangle, uint16_t color) {
  baseDisplay.fillRect(rectangle.xmin + 1, rectangle.ymin + 1, rectangle.xmax - rectangle.xmin - 2, rectangle.ymax - rectangle.ymin - 2, color);
}

void BoxDisplay::renderQRCode() {

  BoxDisplay::clearBuffer();

  BoxDisplay::drawOuterBorders(EPD_LIGHT);

  // either http://ap_ip/login or [PREF_A_WIFI] + IP
  String address = BoxConn::getRootUrl();
  String networkName = BoxConn::getNetworkName();
  String networkPass = BoxConn::getNetworkPass();

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

    BoxDisplay::drawAntialiasedText06("wlan", RECT_TOP, 75, 45, EPD_BLACK);
    if (networkPass != "") {
      BoxDisplay::drawAntialiasedText06(BoxConn::getNetworkPass(), RECT_TOP, 75, 60, EPD_BLACK);
    }
    BoxDisplay::drawAntialiasedText06("open", RECT_TOP, 193, 89, EPD_BLACK);
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

  BoxDisplay::renderHeader();
  BoxDisplay::renderFooter();

  BoxDisplay::flushBuffer();

}

void BoxDisplay::renderMothInfo(String info) {

  BoxDisplay::clearBuffer();

  BoxDisplay::drawOuterBorders(EPD_LIGHT);

  // skip header for clean screen
  BoxDisplay::renderFooter();

  drawAntialiasedText20("moth", RECT_TOP, 8, 100, EPD_BLACK);
  drawAntialiasedText06(info, RECT_TOP, 110, 100, EPD_BLACK);

  BoxDisplay::flushBuffer();

}

void BoxDisplay::drawOuterBorders(uint16_t color) {

  baseDisplay.drawFastHLine(0, 0, 296, color);
  baseDisplay.drawFastHLine(0, 1, 296, color);
  baseDisplay.drawFastHLine(0, 21, 296, color);
  baseDisplay.drawFastHLine(0, 22, 296, color);
  baseDisplay.drawFastHLine(0, 105, 296, color);
  baseDisplay.drawFastHLine(0, 106, 296, color);
  baseDisplay.drawFastHLine(0, 126, 296, color);
  baseDisplay.drawFastHLine(0, 127, 296, color);

  baseDisplay.drawFastVLine(0, 0, 128, color);
  baseDisplay.drawFastVLine(1, 0, 128, color);
  baseDisplay.drawFastVLine(294, 0, 128, color);
  baseDisplay.drawFastVLine(295, 0, 128, color);

}

void BoxDisplay::drawInnerBorders(uint16_t color) {

  baseDisplay.drawFastHLine(196, 63, 100, color);
  baseDisplay.drawFastHLine(196, 64, 100, color);

  baseDisplay.drawFastVLine(196, 21, 86, color);
  baseDisplay.drawFastVLine(197, 22, 86, color);
  
}

void BoxDisplay::renderState() {
  if (displayState == DISPLAY_STATE_TABLE) {
    BoxDisplay::renderTable();
  } else {
    BoxDisplay::renderChart();
  }
}

float BoxDisplay::celsiusToFahrenheit(float celsius) {
  return celsius * 9.0 / 5.0 + 32.0;
}

bool BoxDisplay::hasSignificantChange() {
  if (ssc && displayState == DISPLAY_STATE_TABLE) {
    Measurement measurementA = Measurements::getOffsetMeasurement(0);
    Measurement measurementB = Measurements::getOffsetMeasurement(1);
    if (displayValueTable == DISPLAY_VAL_T___CO2) {
      return abs(measurementB.valuesCo2.co2 - measurementA.valuesCo2.co2) > 50;
    } else if (displayValueTable == DISPLAY_VAL_T___ALT) {
      return abs(measurementB.valuesBme.altitude - measurementA.valuesBme.altitude) > 10;
    } else if (displayValueTable == DISPLAY_VAL_T__P010) {
      return abs(measurementB.valuesPms.pm010 - measurementA.valuesPms.pm010) > 10;
    } else if (displayValueTable == DISPLAY_VAL_T__P025) {
      return abs(measurementB.valuesPms.pm025 - measurementA.valuesPms.pm025) > 10;
    } else if (displayValueTable == DISPLAY_VAL_T__P100) {
      return abs(measurementB.valuesPms.pm100 - measurementA.valuesPms.pm100) > 10;
    }
  }
  return false;
}

ValuesCo2 BoxDisplay::getDisplayValuesCo2() {

  Measurement latestMeasurement = Measurements::getLatestMeasurement();
  int valueCount = 14;

  if (cor > 0 && Measurements::memBufferIndx > valueCount) {

    float temperature = latestMeasurement.valuesCo2.temperature;
    float humidity = latestMeasurement.valuesCo2.humidity;

    // get an average over the bme280's latest 14 temperatures
    Measurement offsetMeasurement;
    float temperatureAvg = 0.0;
    for (int offset = 0; offset < valueCount; offset++) {
      offsetMeasurement = Measurements::getOffsetMeasurement(offset);
      temperatureAvg += offsetMeasurement.valuesBme.temperature;
    }
    temperatureAvg = temperatureAvg / valueCount;

    if (temperatureAvg > temperature) {

      // calculate absolute humidity
      float humidityAbs = humidity * Measurements::toMagnus(temperature); // (g/m3)

      // apply temperature correction, then calculate new relative humidity for that temperature
      temperature = temperature - (temperatureAvg - temperature) * cor;
      humidity = humidityAbs / Measurements::toMagnus(temperature);

      // return the corrected values
      return {
          latestMeasurement.valuesCo2.co2,
          temperature,
          humidity
      };
    } else {
      return latestMeasurement.valuesCo2;
    }
  } else {
    return latestMeasurement.valuesCo2;
  }
}

void BoxDisplay::renderTable() {

  BoxDisplay::clearBuffer();

  BoxDisplay::drawOuterBorders(EPD_LIGHT);
  BoxDisplay::drawInnerBorders(EPD_LIGHT);

  ValuesCo2 displayValuesCo2 = BoxDisplay::getDisplayValuesCo2();
  ValuesPms displayValuesPms = Measurements::getLatestMeasurement().valuesPms;

  float temperature = displayValuesCo2.temperature;
  float humidity = displayValuesCo2.humidity;

  int charPosValueX = 188;

  if (displayValueTable == DISPLAY_VAL_T___CO2) {

    int co2 = displayValuesCo2.co2;
    float stale = max(0.0, min(10.0, (co2 - 425.0) / 380.0)); // don't allow negative stale values

    textColor = getTextColor(co2, thresholdsCo2);
    fillColor = getFillColor(co2, thresholdsCo2);
    vertColor = getVertColor(co2, thresholdsCo2);

    if (fillColor != EPD_WHITE) {
      BoxDisplay::fillRectangle(RECT_CO2, fillColor);
    }

    BoxDisplay::drawAntialiasedText36(formatString(String(co2), FORMAT_4_DIGIT), RECT_CO2, 28, 76, textColor);
    BoxDisplay::drawAntialiasedText06("CO²", RECT_CO2, 6, TEXT_OFFSET_Y, textColor); // font has been altered to display superscript 2 as subscript 2
    BoxDisplay::drawAntialiasedText06("ppm", RECT_CO2, charPosValueX - charDimX6 * 3, TEXT_OFFSET_Y, textColor);

    int yScale = 5;
    int yMax = RECT_CO2.ymax - 7;
    for (int percent = 0; percent <= 10; percent++) {
      baseDisplay.drawFastHLine(RECT_CO2.xmin + 6, yMax - percent * yScale, 12, percent % 5 == 0 ? textColor : vertColor);
    }

    int yDim = round(stale * yScale);
    for (int y = -1; y <= 1; y++) {
      baseDisplay.drawFastHLine(RECT_CO2.xmin + 8, yMax - yDim + y, 8, textColor);
    }
  } else if (displayValueTable == DISPLAY_VAL_T___HPA) {

    int hpa = Measurements::getLatestMeasurement().valuesBme.pressure / 100.0;
    textColor = EPD_BLACK;
    fillColor = EPD_WHITE;
    vertColor = EPD_DARK;

    BoxDisplay::fillRectangle(RECT_CO2, fillColor);

    BoxDisplay::drawAntialiasedText06("pressure", RECT_CO2, 6, TEXT_OFFSET_Y, textColor);
    BoxDisplay::drawAntialiasedText36(formatString(String(hpa), FORMAT_4_DIGIT), RECT_CO2, 28, 76, textColor);

    BoxDisplay::drawAntialiasedText06("hPa", RECT_CO2, charPosValueX - charDimX6 * 3, TEXT_OFFSET_Y, textColor);
  } else if (displayValueTable == DISPLAY_VAL_T___ALT) {

    int alt = round(Measurements::getLatestMeasurement().valuesBme.altitude);
    textColor = EPD_BLACK;
    fillColor = EPD_WHITE;
    vertColor = EPD_DARK;

    BoxDisplay::fillRectangle(RECT_CO2, fillColor);

    BoxDisplay::drawAntialiasedText06("altitude", RECT_CO2, 6, TEXT_OFFSET_Y, textColor);
    BoxDisplay::drawAntialiasedText36(formatString(String(alt), FORMAT_4_DIGIT), RECT_CO2, 28, 76, textColor);

    BoxDisplay::drawAntialiasedText06("m", RECT_CO2, charPosValueX - charDimX6 * 1, TEXT_OFFSET_Y, textColor);
  } else {

    int pms;
    String pmsLabel;
    if (displayValueTable == DISPLAY_VAL_T__P010) {
      pms = round(displayValuesPms.pm010);
      pmsLabel = "pm 1.0";
    } else if (displayValueTable == DISPLAY_VAL_T__P025) {
      pms = round(displayValuesPms.pm025);
      pmsLabel = "pm 2.5";
    } else if (displayValueTable == DISPLAY_VAL_T__P100) {
      pms = round(displayValuesPms.pm100);
      pmsLabel = "pm 10.0";
    }

    textColor = getTextColor(pms, thresholdsPms);
    fillColor = getFillColor(pms, thresholdsPms);
    vertColor = getVertColor(pms, thresholdsPms);

    if (fillColor != EPD_WHITE) {
      BoxDisplay::fillRectangle(RECT_CO2, fillColor);
    }

    BoxDisplay::drawAntialiasedText06(pmsLabel, RECT_CO2, 6, TEXT_OFFSET_Y, textColor);
    BoxDisplay::drawAntialiasedText36(formatString(String(max(0, pms)), FORMAT_4_DIGIT), RECT_CO2, 28, 76, textColor);

    BoxDisplay::drawAntialiasedText06("µg/m3", RECT_CO2, charPosValueX - charDimX6 * 5, TEXT_OFFSET_Y, textColor);

  }

  textColor = getTextColor(temperature, thresholdsTemperature);
  fillColor = getFillColor(temperature, thresholdsTemperature);
  if (c2f) {
    temperature = BoxDisplay::celsiusToFahrenheit(temperature);
  }
  int temperature10 = round(temperature * 10.0);
  int temperatureFix = floor(temperature10 / 10.0);
  int temperatureFrc = abs(temperature10 % 10);
  if (fillColor != EPD_WHITE) {
    BoxDisplay::fillRectangle(RECT_DEG, fillColor);
  }
  BoxDisplay::drawAntialiasedText20(formatString(String(temperatureFix), FORMAT_3_DIGIT), RECT_DEG, 4, 35, textColor);
  BoxDisplay::drawAntialiasedText06(c2f ? "°F" : "°C", RECT_DEG, 90 - charDimX6 * 2, TEXT_OFFSET_Y, textColor);
  BoxDisplay::drawAntialiasedText08(".", RECT_DEG, 72, 35, textColor);
  BoxDisplay::drawAntialiasedText08(String(temperatureFrc), RECT_DEG, 82, 35, textColor);

  textColor = getTextColor(humidity, thresholdsHumidity);
  fillColor = getFillColor(humidity, thresholdsHumidity);
  int humidity10 = round(humidity * 10.0);
  int humidityFix = floor(humidity10 / 10.0);
  int humidityFrc = abs(humidity10 % 10);
  if (fillColor != EPD_WHITE) {
    BoxDisplay::fillRectangle(RECT_HUM, fillColor);
  }
  BoxDisplay::drawAntialiasedText20(formatString(String(humidityFix), FORMAT_3_DIGIT), RECT_HUM, 4, 35, textColor);
  BoxDisplay::drawAntialiasedText06("%", RECT_HUM, 90 - charDimX6 * 1, TEXT_OFFSET_Y, textColor);
  BoxDisplay::drawAntialiasedText08(".", RECT_HUM, 72, 35, textColor);
  BoxDisplay::drawAntialiasedText08(String(humidityFrc), RECT_HUM, 82, 35, textColor);

  BoxDisplay::renderHeader();
  BoxDisplay::renderFooter();

  BoxDisplay::flushBuffer();
}

void BoxDisplay::renderChart() {

  BoxDisplay::clearBuffer();
  BoxDisplay::drawOuterBorders(EPD_LIGHT);

  int displayableMeasurementCount = min(chartMeasurementCount, (int)ceil(Measurements::memBufferIndx * 1.0 / chartMeasurementHours));
  Measurement measurement;

  int minValue = 0;
  int maxValue = 1500;
  if (displayValueChart == DISPLAY_VAL_C___CO2) {
    for (int i = 0; i < displayableMeasurementCount; i++) {
      measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours);
      if (measurement.valuesCo2.co2 > 3600) {
        maxValue = 6000; // 0, 2000, 4000, (6000)
      } else if (measurement.valuesCo2.co2 > 2400) {
        maxValue = 4500; // 0, 1500, 3000, (4500)
      } else if (measurement.valuesCo2.co2 > 1200) {
        maxValue = 3000; // 0, 1000, 2000, (3000)
      } 
    }
  } else if (displayValueChart == DISPLAY_VAL_C___DEG) {
    if (c2f) {
      minValue = 60;
      maxValue = 120;
    } else {
      minValue = 10;
      maxValue = 40;
    }
  } else if (displayValueChart == DISPLAY_VAL_C___HUM) {
    minValue = 20;
    maxValue = 80; // 20, 40, 60, (80)
  } else if (displayValueChart == DISPLAY_VAL_C___HPA) {
    double pressureAvg = 0;
    for (int i = 0; i < displayableMeasurementCount; i++) {
      measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours);
      pressureAvg += measurement.valuesBme.pressure / 100.0;
    }
    pressureAvg /= displayableMeasurementCount;     // lets say it is 998
    minValue = floor(pressureAvg / 10.0) * 10 - 10; // 99.8 -> 99 -> 990 -> 980
    maxValue = ceil(pressureAvg / 10.0) * 10 + 10;  // 99.8 -> 100 -> 1000 -> 1010
  } else if (displayValueChart == DISPLAY_VAL_C___ALT) {
    maxValue = 450;
    for (int i = 0; i < displayableMeasurementCount; i++) {
      measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours);
      if (measurement.valuesBme.altitude > 1600) {
        maxValue = 3600; // 0, 1200, 2400, 3600
      } else if (measurement.valuesBme.altitude > 800) {
        maxValue = 1800; // 0, 600, 1200, (1800)
      } else if (measurement.valuesBme.altitude > 400) {
        maxValue = 900; // 0, 300, 600, (900)
      } 
    }
  } else if (displayValueChart == DISPLAY_VAL_C__P010) {
    maxValue = 9;
    for (int i = 0; i < displayableMeasurementCount; i++) {
      for (int h = 0; h < chartMeasurementHours; h++) {
        measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours + h); // dont skip any measurements
        if (measurement.valuesPms.pm010 > 36) {
          maxValue = max(90, maxValue); // 0, 30, 60, (90)
        } else if (measurement.valuesPms.pm010 > 12) {
          maxValue = max(45, maxValue); // 0, 15, 30, (45)
        } else if (measurement.valuesPms.pm010 > 7) {
          maxValue = max(15, maxValue); // 0, 5, 10, (15)
        } 
      }
    }
  } else if (displayValueChart == DISPLAY_VAL_C__P025) {
    maxValue = 9; // 0, 3, 6, (9)
    for (int i = 0; i < displayableMeasurementCount; i++) {
      for (int h = 0; h < chartMeasurementHours; h++) {
        measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours + h); // dont skip any measurements
        if (measurement.valuesPms.pm025 > 36) {
          maxValue = max(90, maxValue); // 0, 30, 60, (90)
        } else if (measurement.valuesPms.pm025 > 12) {
          maxValue = max(45, maxValue); // 0, 15, 30, (45)
        } else if (measurement.valuesPms.pm025 > 7) {
          maxValue = max(15, maxValue); // 0, 5, 10, (15)
        } 
      }
    }
  } else if (displayValueChart == DISPLAY_VAL_C__P100) {
    maxValue = 9;
    for (int i = 0; i < displayableMeasurementCount; i++) {
      for (int h = 0; h < chartMeasurementHours; h++) {
        measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours + h); // dont skip any measurements
        if (measurement.valuesPms.pm100 > 36) {
          maxValue = max(90, maxValue); // 0, 30, 60, (90)
        } else if (measurement.valuesPms.pm100 > 12) {
          maxValue = max(45, maxValue); // 0, 15, 30, (45)
        } else if (measurement.valuesPms.pm100 > 7) {
          maxValue = max(15, maxValue); // 0, 5, 10, (15)
        } 
      }
    }
  }

  String label2 = String(minValue + (maxValue - minValue) * 2 / 3);
  String label1 = String(minValue + (maxValue - minValue) / 3);
  String label0 = String(minValue);

  int charPosValueX = 41;
  int charPosLabelY = 12;

  BoxDisplay::drawAntialiasedText06(label2, RECT_CO2, charPosValueX - charDimX6 * label2.length(), 24, EPD_BLACK);
  baseDisplay.drawFastHLine(1, 49, 296, EPD_LIGHT);
  BoxDisplay::drawAntialiasedText06(label1, RECT_CO2, charPosValueX - charDimX6 * label1.length(), 52, EPD_BLACK);
  baseDisplay.drawFastHLine(1, 77, 296, EPD_LIGHT);
  BoxDisplay::drawAntialiasedText06(label0, RECT_CO2, charPosValueX - charDimX6 * label0.length(), 80, EPD_BLACK);

  String title;
  if (displayValueChart == DISPLAY_VAL_C___CO2) {
    title = "CO² ppm," + String(chartMeasurementHours) + "h"; // the sup 2 has been modified in the font to display as sub
  } else if (displayValueChart == DISPLAY_VAL_C___DEG) {
    title = c2f ? "temperature °F," : "temperature °C," + String(chartMeasurementHours) + "h";
  } else if (displayValueChart == DISPLAY_VAL_C___HUM) {
    title = "humidity %," + String(chartMeasurementHours) + "h";
  } else if (displayValueChart == DISPLAY_VAL_C___HPA) {
    title = "pressure hPa," + String(chartMeasurementHours) + "h";
  } else if (displayValueChart == DISPLAY_VAL_C___ALT) {
    title = "altitude m," + String(chartMeasurementHours) + "h";
  } else if (displayValueChart == DISPLAY_VAL_C__P010) {
    title = "pm 1.0 µg/m3," + String(chartMeasurementHours) + "h";
  } else if (displayValueChart == DISPLAY_VAL_C__P025) {
    title = "pm 2.5 µg/m3," + String(chartMeasurementHours) + "h";
  } else if (displayValueChart == DISPLAY_VAL_C__P100) {
    title = "pm 10.0 µg/m3," + String(chartMeasurementHours) + "h";
  }
  BoxDisplay::drawAntialiasedText06(title, RECT_CO2, limitPosX - title.length() * charDimX6 + 3, charPosLabelY, EPD_BLACK);

  int minX;
  int minY;
  int maxY = 103;
  int dimY;
  float limY = 78.0;
  float curValue;

  int displayableBarWidth = 60 * 4 / chartMeasurementCount;
  for (int i = 0; i < displayableMeasurementCount; i++) {

    measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours);

    if (displayValueChart == DISPLAY_VAL_C___CO2) {
      curValue = zero;
      for (int h = 0; h < chartMeasurementHours; h++) {
        measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours + h);
        curValue += measurement.valuesCo2.co2;
      }
      curValue = curValue * one / chartMeasurementHours;
    } else if (displayValueChart == DISPLAY_VAL_C___DEG) {
      curValue = zero;
      for (int h = 0; h < chartMeasurementHours; h++) {
        measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours + h);
        curValue += measurement.valuesCo2.temperature;
      }
      curValue = curValue * one / chartMeasurementHours;      
    } else if (displayValueChart == DISPLAY_VAL_C___HUM) {
      curValue = zero;
      for (int h = 0; h < chartMeasurementHours; h++) {
        measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours + h);
        curValue += measurement.valuesCo2.humidity;
      }
      curValue = curValue * one / chartMeasurementHours;       
    } else if (displayValueChart == DISPLAY_VAL_C___HPA) {
      curValue = zero;
      for (int h = 0; h < chartMeasurementHours; h++) {
        measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours + h);
        curValue += measurement.valuesBme.pressure;
      }
      curValue = curValue * one / chartMeasurementHours / 100.0;       
    } else if (displayValueChart == DISPLAY_VAL_C___ALT) {
      curValue = zero;
      for (int h = 0; h < chartMeasurementHours; h++) {
        measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours + h);
        curValue += measurement.valuesBme.altitude;
      }
      curValue = curValue * one / chartMeasurementHours;         
    } else if (displayValueChart == DISPLAY_VAL_C__P010) {
      curValue = zero;
      for (int h = 0; h < chartMeasurementHours; h++) {
        measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours + h);
        curValue = max(curValue, measurement.valuesPms.pm010);
      }
    } else if (displayValueChart == DISPLAY_VAL_C__P025) {
      curValue = zero;
      for (int h = 0; h < chartMeasurementHours; h++) {
        measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours + h);
        curValue = max(curValue, measurement.valuesPms.pm025);
      }
    } else if (displayValueChart == DISPLAY_VAL_C__P100) {
      curValue = zero;
      for (int h = 0; h < chartMeasurementHours; h++) {
        measurement = Measurements::getOffsetMeasurement(i * chartMeasurementHours + h);
        curValue = max(curValue, measurement.valuesPms.pm100);
      }
    }

    minX = limitPosX + 1 - (i + 1) * displayableBarWidth;
    dimY = max(0, min((int)limY, (int)round((curValue - minValue) * limY / (maxValue - minValue))));
    minY = maxY - dimY;

    baseDisplay.drawFastVLine(minX, minY, dimY, EPD_BLACK);
    for (int b = 1; b < displayableBarWidth - 1; b++) {
      baseDisplay.drawFastVLine(minX + b, minY, dimY, EPD_BLACK);
    }
  }

  BoxDisplay::renderHeader();
  BoxDisplay::renderFooter();

  BoxDisplay::flushBuffer();

}

void BoxDisplay::drawAntialiasedText06(String text, Rectangle rectangle, int xRel, int yRel, uint16_t color) {
  drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb06pt_l, &smb06pt_d, &smb06pt_b);
}

void BoxDisplay::drawAntialiasedText08(String text, Rectangle rectangle, int xRel, int yRel, uint16_t color) {
  drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb08pt_l, &smb08pt_d, &smb08pt_b);
}

void BoxDisplay::drawAntialiasedText20(String text, Rectangle rectangle, int xRel, int yRel, uint16_t color) {
  drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb20pt_l, &smb20pt_d, &smb20pt_b);
}

void BoxDisplay::drawAntialiasedText36(String text, Rectangle rectangle, int xRel, int yRel, uint16_t color) {
  drawAntialiasedText(text, rectangle, xRel, yRel, color, &smb36pt_l, &smb36pt_d, &smb36pt_b);
}

void BoxDisplay::drawAntialiasedText(String text, Rectangle rectangle, int xRel, int yRel, uint16_t color, const GFXfont *fontL, const GFXfont *fontD, const GFXfont *fontB) {

  baseDisplay.setCursor(rectangle.xmin + xRel, rectangle.ymin + yRel);
  baseDisplay.setFont(fontL);
  baseDisplay.setTextColor(color == EPD_BLACK ? EPD_LIGHT : EPD_DARK);
  baseDisplay.print(text);

  baseDisplay.setCursor(rectangle.xmin + xRel, rectangle.ymin + yRel);
  baseDisplay.setFont(fontD);
  baseDisplay.setTextColor(color == EPD_BLACK ? EPD_DARK : EPD_LIGHT);
  baseDisplay.print(text);

  baseDisplay.setCursor(rectangle.xmin + xRel, rectangle.ymin + yRel);
  baseDisplay.setFont(fontB);
  baseDisplay.setTextColor(color);
  baseDisplay.print(text);

}

void BoxDisplay::renderHeader() {

  BoxDisplay::drawAntialiasedText08(ButtonHandlers::A.buttonActionSlow.label, RECT_TOP, 6, TEXT_OFFSET_Y, EPD_BLACK);
  BoxDisplay::drawAntialiasedText08(ButtonHandlers::A.buttonActionFast.label, RECT_TOP, 6 + charDimX6 * 2, TEXT_OFFSET_Y, EPD_BLACK);
  BoxDisplay::drawAntialiasedText06(ButtonHandlers::A.extraLabel,             RECT_TOP, 6 + charDimX6 * 4, TEXT_OFFSET_Y - 2, EPD_BLACK);

  BoxDisplay::drawAntialiasedText08(ButtonHandlers::B.buttonActionSlow.label, RECT_TOP, 136, TEXT_OFFSET_Y, EPD_BLACK);
  BoxDisplay::drawAntialiasedText08(ButtonHandlers::B.buttonActionFast.label, RECT_TOP, 136 + charDimX6 * 2, TEXT_OFFSET_Y, EPD_BLACK);
  BoxDisplay::drawAntialiasedText06(ButtonHandlers::B.extraLabel,             RECT_TOP, 136 + charDimX6 * 4, TEXT_OFFSET_Y - 2, EPD_BLACK);

  BoxDisplay::drawAntialiasedText08(ButtonHandlers::C.buttonActionSlow.label, RECT_TOP, 264, TEXT_OFFSET_Y, EPD_BLACK);
  BoxDisplay::drawAntialiasedText08(ButtonHandlers::C.buttonActionFast.label, RECT_TOP, 264 + charDimX6 * 2, TEXT_OFFSET_Y, EPD_BLACK);

  // String csvCountFormatted = String(Measurements::getCsvBufferIndex());
  // int publishableCount = BoxMqtt::isConfiguredToBeActive() ? Measurements::getPublishableCount() : 0;
  // if (publishableCount > 0) {
  //   String mqttCountFormatted = String(Measurements::getPublishableCount());
  //   int pendingDataSize = csvCountFormatted.length() + mqttCountFormatted.length() + 1; // total string length
  //   char pendingDataBuf[csvCountFormatted.length() + mqttCountFormatted.length() + 1];
  //   sprintf(pendingDataBuf, "%s|%s", csvCountFormatted, mqttCountFormatted);
  //   BoxDisplay::drawAntialiasedText06(pendingDataBuf, RECT_TOP, limitPosX - pendingDataSize * charDimX6, TEXT_OFFSET_Y, EPD_BLACK);
  // } else {
  //   BoxDisplay::drawAntialiasedText06(csvCountFormatted.c_str(), RECT_TOP, limitPosX - csvCountFormatted.length() * charDimX6, TEXT_OFFSET_Y, EPD_BLACK);
  // }

}

void BoxDisplay::renderFooter() {

  ValuesBat valuesBat = BoxPack::values;

  String cellPercentFormatted = formatString(String(valuesBat.percent, 0), FORMAT_CELL_PERCENT);
  BoxDisplay::drawAntialiasedText06(cellPercentFormatted, RECT_BOT, limitPosX - charDimX6 * 6, TEXT_OFFSET_Y, EPD_BLACK);

  // main battery frame
  BoxDisplay::fillRectangle(RECT_BAT, EPD_BLACK);
  // right border battery contact
  baseDisplay.drawFastVLine(RECT_BAT.xmax - 1, RECT_BAT.ymin + 3, (RECT_BAT.ymax - RECT_BAT.ymin - 6), EPD_BLACK);
  // battery internal
  BoxDisplay::fillRectangle({RECT_BAT.xmin + 1,
                             RECT_BAT.ymin + 1,
                             RECT_BAT.xmax - 1,
                             RECT_BAT.ymax - 1},
                            EPD_WHITE);
  // percentage
  BoxDisplay::fillRectangle({RECT_BAT.xmin + 2,
                             RECT_BAT.ymin + 2,
                             RECT_BAT.xmin + 2 + round((RECT_BAT.xmax - RECT_BAT.xmin - 4) * valuesBat.percent * 0.01), // need to add padding
                             RECT_BAT.ymax - 2},
                            EPD_BLACK);


  bool isPmsM = SensorPmsa003i::getMode() == PMS____ON_M || SensorPmsa003i::getMode() == PMS_PAUSE_M;
  bool isPmsD = SensorPmsa003i::getMode() == PMS____ON_D || SensorPmsa003i::getMode() == PMS_PAUSE_D;
  bool isCon = BoxConn::getMode() != WIFI_OFF;

  int charPosFooter = 6;
  if (isPmsM || isPmsD) {
    BoxDisplay::drawAntialiasedText08(isPmsM ? BoxDisplay::SYMBOL__PM_M : BoxDisplay::SYMBOL__PM_D, RECT_BOT, 6, TEXT_OFFSET_Y + 1, EPD_BLACK);
    charPosFooter += 13;
  } 

  if (isCon) {
    String address = BoxConn::getAddress();
    BoxDisplay::drawAntialiasedText06(BoxConn::getAddress(), RECT_BOT, charPosFooter, TEXT_OFFSET_Y, EPD_BLACK);
    String timeFormatted = BoxClock::getDateTimeDisplayString(BoxClock::getDate());
    BoxDisplay::drawAntialiasedText06(",", RECT_BOT, charPosFooter + address.length() * charDimX6, TEXT_OFFSET_Y, EPD_BLACK);
    BoxDisplay::drawAntialiasedText06(timeFormatted, RECT_BOT, charPosFooter + (address.length() + 1) * charDimX6, TEXT_OFFSET_Y, EPD_BLACK);
  } else {
    String timeFormatted = BoxClock::getDateTimeDisplayString(BoxClock::getDate());
    BoxDisplay::drawAntialiasedText06(timeFormatted, RECT_BOT, charPosFooter, TEXT_OFFSET_Y, EPD_BLACK);
  }

}

void BoxDisplay::setState(display_state_t state) {
  displayState = state;
}

void BoxDisplay::toggleState() {
  if (displayState == DISPLAY_STATE_TABLE) {
    displayState = DISPLAY_STATE_CHART;
  } else {
    displayState = DISPLAY_STATE_TABLE;
  }
}

display_state_t BoxDisplay::getState() {
  return displayState;
}

void BoxDisplay::setTheme(display_theme_t theme) {
  displayTheme = theme;
}

void BoxDisplay::toggleTheme() {
  if (displayTheme == DISPLAY_THEME_LIGHT) {
    displayTheme = DISPLAY_THEME__DARK;
  } else {
    displayTheme = DISPLAY_THEME_LIGHT;
  }
}

display_theme_t BoxDisplay::getTheme() {
  return displayTheme;
}

void BoxDisplay::setValueTable(display_val_t_v value) {
  displayValueTable = value;
}

display_val_t_v BoxDisplay::getValueTable() {
  return displayValueTable;
}

void BoxDisplay::setValueChart(display_val_c_v value) {
  displayValueChart = value;
}

void BoxDisplay::toggleValueFw() {

  if (displayState == DISPLAY_STATE_TABLE) {

    display_val_t_v maxValue = DISPLAY_VAL_T___ALT;
    if (SensorPmsa003i::getMode() != PMS_____OFF) {
      maxValue = DISPLAY_VAL_T__P100;
    }
    displayValueTable = (display_val_t_v)(displayValueTable + 1);
    while (displayValueTable > maxValue) {
      displayValueTable = (display_val_t_v)(displayValueTable - maxValue - 1);
    }

  } else { // chart

    display_val_c_v maxValue = DISPLAY_VAL_C___ALT;
    if (SensorPmsa003i::ACTIVE) {
      maxValue = DISPLAY_VAL_C__P100;
    }
    displayValueChart = (display_val_c_v)(displayValueChart + 1);
    while (displayValueChart > maxValue) {
      displayValueChart = (display_val_c_v)(displayValueChart - maxValue - 1);
    }    

  }

}

void BoxDisplay::toggleValueBw() {

  if (displayState == DISPLAY_STATE_TABLE) {

    display_val_t_v maxValue = DISPLAY_VAL_T___ALT;
    if (SensorPmsa003i::getMode() != PMS_____OFF) {
      maxValue = DISPLAY_VAL_T__P100;
    }
    displayValueTable = (display_val_t_v)(displayValueTable - 1);
    while (displayValueTable < 0) {
      displayValueTable = (display_val_t_v)(displayValueTable + maxValue + 1);
    }

  } else { // chart

    display_val_c_v maxValue = DISPLAY_VAL_C___ALT;
    if (SensorPmsa003i::ACTIVE) {
      maxValue = DISPLAY_VAL_C__P100;
    }
    displayValueChart = (display_val_c_v)(displayValueChart - 1);
    while (displayValueChart < 0) {
      displayValueChart = (display_val_c_v)(displayValueChart + maxValue + 1);
    }    

  }

}

void BoxDisplay::toggleChartMeasurementHoursFw() {
  if (chartMeasurementHours == 1) {
    chartMeasurementHours = 3;
  } else if(chartMeasurementHours == 3) {
    chartMeasurementHours = 6;
  } else if(chartMeasurementHours == 6) {
    chartMeasurementHours = 12;
  } else if (chartMeasurementHours == 12) {
    chartMeasurementHours = 24;
  } 
}
void BoxDisplay::toggleChartMeasurementHoursBw() {
  if (chartMeasurementHours == 24) {
    chartMeasurementHours = 12;
  } else if(chartMeasurementHours == 12) {
    chartMeasurementHours = 6;
  } else if(chartMeasurementHours == 6) {
    chartMeasurementHours = 3;
  } else if (chartMeasurementHours == 3) {
    chartMeasurementHours = 1;
  } 
}
int BoxDisplay::getChartMeasurementHours() {
  return chartMeasurementHours;
}

String BoxDisplay::formatString(String value, char const *format) {
  char padBuffer[16];
  sprintf(padBuffer, format, value);
  return padBuffer;
}
