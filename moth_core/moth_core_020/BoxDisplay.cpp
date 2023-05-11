#include "BoxDisplay.h"
#include "BoxPack.h"
#include "BoxClock.h"
#include "BoxFiles.h"
#include "BoxMqtt.h"
#include "BoxConn.h"
#include "Measurements.h"
#include "SensorScd041.h"
#include "WiFi.h"
#include <SdFat.h>

#define EPD_DC 10
#define EPD_CS 9
#define EPD_BUSY 14  // A4 -> has been solder-connected to the busy pad
#define SRAM_CS -1 
#define EPD_RESET 8 // A5 -> has been bridged to the reset pin 

typedef enum {
  DISPLAY_STATE_TABLE,
  DISPLAY_STATE_CHART
} display_state_t;

typedef enum {
  DISPLAY_THEME_LIGHT,
  DISPLAY_THEME__DARK
} display_theme_t;

/**
 * ################################################
 * ## constants
 * ################################################
 */
const String JSON_KEY___MINUTES = "min";
const String JSON_KEY____OFFSET = "off";
const String JSON_KEY__TIMEZONE = "tzn";
const String JSON_KEY_______CO2 = "co2";
const String JSON_KEY_______DEG = "deg";
const String JSON_KEY_______HUM = "hum";
const String JSON_KEY_WARN__LOW = "rLo";
const String JSON_KEY_RISK__LOW = "wLo";
const String JSON_KEY_WARN_HIGH = "wHi";
const String JSON_KEY_RISK_HIGH = "rHi";
const String JSON_KEY_______C2F = "c2f";

const int64_t MICROSECONDS_PER_MINUTE = 60000000;
const char FORMAT_3_DIGIT[] = "%3s";
const char FORMAT_4_DIGIT[] = "%4s";
const char FORMAT_5_DIGIT[] = "%5s";
const char FORMAT_6_DIGIT[] = "%6s";
const char FORMAT_SLASH_CONCAT[] = "%s/%s";
const char FORMAT_CELL_PERCENT[] = "%5s%%";
const char FORMAT_STALE[] = "%3s%% stale";

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
int TEXT_OFFSET_Y = 16;

/**
 * ################################################
 * ## mutable variables
 * ################################################
 */
display_state_t displayState = DISPLAY_STATE_TABLE;
display_theme_t displayTheme = DISPLAY_THEME_LIGHT;

int textColor;
int fillColor;

Thresholds thresholdsTemperature;
Thresholds thresholdsHumidity;
Thresholds thresholdsHighlight;

bool c2f = false; // if true show values in fahrenheit

int16_t tbx, tby;
uint16_t tbw, tbh;

/**
 * ################################################
 * ## static class variabales                     
 * ################################################
 */
String BoxDisplay::CONFIG_PATH = "/config/disp.json";
config_status_t BoxDisplay::configStatus = CONFIG_STATUS_PENDING;
Thresholds BoxDisplay::thresholdsCo2;
int64_t BoxDisplay::microsecondsRenderStateInterval = 120000000; // default :: 2 minutes, will be overridden by config
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

  int co2WarnHi =  800;
  int co2RiskHi = 1000;

  int degRiskLo = 14;
  int degWarnLo = 19;
  int degWarnHi = 25;
  int degRiskHi = 30;

  int humRiskLo = 25;
  int humWarnLo = 30;
  int humWarnHi = 60;
  int humRiskHi = 65;  

  int displayUpdateMinutes = 3;
  float temperatureOffset = SensorScd041::getTemperatureOffset();
  String timezone = BoxClock::getTimezone();

  if (BoxFiles::existsFile32(BoxDisplay::CONFIG_PATH)) {

    BoxDisplay::configStatus = CONFIG_STATUS_PRESENT;

    File32 dispFile;
    dispFile.open(BoxDisplay::CONFIG_PATH.c_str(), O_RDONLY);

    BoxDisplay::configStatus = CONFIG_STATUS__LOADED;

    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(dispFile);    

    displayUpdateMinutes = root[JSON_KEY___MINUTES] | displayUpdateMinutes;
    timezone = root[JSON_KEY__TIMEZONE] | timezone;

    co2WarnHi = root[JSON_KEY_______CO2][JSON_KEY_WARN_HIGH] | co2WarnHi;
    co2RiskHi = root[JSON_KEY_______CO2][JSON_KEY_RISK_HIGH] | co2RiskHi;

    degRiskLo = root[JSON_KEY_______DEG][JSON_KEY_RISK__LOW] | degRiskLo;
    degWarnLo = root[JSON_KEY_______DEG][JSON_KEY_WARN__LOW] | degWarnLo;
    degWarnHi = root[JSON_KEY_______DEG][JSON_KEY_WARN_HIGH] | degWarnHi;
    degRiskHi = root[JSON_KEY_______DEG][JSON_KEY_RISK_HIGH] | degRiskHi;
    temperatureOffset = root[JSON_KEY_______DEG][JSON_KEY____OFFSET] | temperatureOffset;
    c2f = root[JSON_KEY_______DEG][JSON_KEY_______C2F] | false;

    humRiskLo = root[JSON_KEY_______HUM][JSON_KEY_RISK__LOW] | humRiskLo;
    humWarnLo = root[JSON_KEY_______HUM][JSON_KEY_WARN__LOW] | humWarnLo;
    humWarnHi = root[JSON_KEY_______HUM][JSON_KEY_WARN_HIGH] | humWarnHi;
    humRiskHi = root[JSON_KEY_______HUM][JSON_KEY_RISK_HIGH] | humRiskHi;

    dispFile.close();

    BoxDisplay::configStatus = CONFIG_STATUS__PARSED;

  } else {
    BoxDisplay::configStatus = CONFIG_STATUS_MISSING;
  }

  BoxDisplay::thresholdsCo2 = {
    0,
    0,
    co2WarnHi,
    co2RiskHi,
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

  SensorScd041::setTemperatureOffset(temperatureOffset);
  BoxClock::setTimezone(timezone);

  // min elapsed time before a display state update
  BoxDisplay::microsecondsRenderStateInterval = max(MICROSECONDS_PER_MINUTE, displayUpdateMinutes * MICROSECONDS_PER_MINUTE);

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

void BoxDisplay::fillRectangle(Rectangle rectangle, uint16_t color) {
  baseDisplay.fillRect(rectangle.xmin + 1, rectangle.ymin + 1, rectangle.xmax - rectangle.xmin - 2, rectangle.ymax - rectangle.ymin - 2, color);
}

void BoxDisplay::renderQRCode() {

  BoxDisplay::clearBuffer();

  BoxDisplay::drawOuterBorders(EPD_LIGHT);

  // either http://ap_ip/login or [PREF_A_WIFI] + IP
  String address = BoxConn::getRootUrl();
  String network = BoxConn::getNetwork();

  int qrCodeX = 12;
  int qrCodeY = 35;

  // render the network connection link
  if (network != "") {

    char networkBuf[network.length() + 1];
    network.toCharArray(networkBuf, network.length() + 1);

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

    BoxDisplay::drawAntialiasedText08("< CONNECT", RECT_TOP, 75, 47, EPD_BLACK);
    BoxDisplay::drawAntialiasedText08(BoxConn::getAPLogin(), RECT_TOP, 94, 63, EPD_BLACK);
    BoxDisplay::drawAntialiasedText08("OPEN >", RECT_TOP, 160, 91, EPD_BLACK);


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

  renderEnvironment();

  BoxDisplay::flushBuffer();

}

void BoxDisplay::renderMothInfo(String info) {

  BoxDisplay::clearBuffer();

  BoxDisplay::drawOuterBorders(EPD_LIGHT);

  drawAntialiasedText20("moth", RECT_TOP, 8, 100, EPD_BLACK);
  drawAntialiasedText08(info, RECT_TOP, 110, 100, EPD_BLACK);

  BoxDisplay::renderEnvironment();

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

void BoxDisplay::renderTable() {  

  Measurement measurement = Measurements::getLatestMeasurement();

  BoxDisplay::clearBuffer();

  BoxDisplay::drawOuterBorders(EPD_LIGHT);
  BoxDisplay::drawInnerBorders(EPD_LIGHT);

  int co2 = measurement.valuesCo2.co2;
  float stale = max(0.0, (co2 - 425.0) / 380.0); // don't allow negative values
  textColor = getTextColor(co2, BoxDisplay::thresholdsCo2);
  fillColor = getFillColor(co2, BoxDisplay::thresholdsCo2);
  if (fillColor != EPD_WHITE) {
    BoxDisplay::fillRectangle(RECT_CO2, fillColor);
  }
  BoxDisplay::drawAntialiasedText36(formatString(String(co2), FORMAT_4_DIGIT), RECT_CO2, 28, 76, textColor);

  BoxDisplay::drawAntialiasedText08("CO", RECT_CO2, 6, TEXT_OFFSET_Y, textColor);
  BoxDisplay::drawAntialiasedText08("2", RECT_CO2, 26, TEXT_OFFSET_Y + 4, textColor);
  if (co2 < 1000) {
    // upper left of CO2
    BoxDisplay::drawAntialiasedText08("ppm", RECT_CO2, 6, 76, textColor);
    BoxDisplay::drawAntialiasedText08(formatString(String(stale, 1), FORMAT_STALE), RECT_CO2, 91, TEXT_OFFSET_Y, textColor);
  } else {
    // lower left of CO2
    BoxDisplay::drawAntialiasedText08("ppm", RECT_CO2, 46, TEXT_OFFSET_Y, textColor);
    BoxDisplay::drawAntialiasedText08(formatString(String(stale, 1), FORMAT_CELL_PERCENT), RECT_CO2, 131, TEXT_OFFSET_Y, textColor);
  }
  

  // measurement.valuesBme.pressure;

  float temperature = measurement.valuesCo2.temperature;
  textColor = getTextColor(temperature, thresholdsTemperature);
  fillColor = getFillColor(temperature, thresholdsTemperature);
  if (c2f) {
    temperature = temperature * 9.0 / 5.0 + 32;
  }  
  int temperature10 = round(temperature * 10.0);
  int temperatureFix = floor(temperature10 / 10.0);
  int temperatureFrc = temperature10 % 10;
  if (fillColor != EPD_WHITE) {
    BoxDisplay::fillRectangle(RECT_DEG, fillColor);
  }
  BoxDisplay::drawAntialiasedText20(formatString(String(temperatureFix), FORMAT_3_DIGIT), RECT_DEG, 4, 35, textColor);
  BoxDisplay::drawAntialiasedText08(c2f ? "F" : "C", RECT_DEG, 82, TEXT_OFFSET_Y, textColor);
  BoxDisplay::drawAntialiasedText08(".", RECT_DEG, 72, 35, textColor);
  BoxDisplay::drawAntialiasedText08(String(temperatureFrc), RECT_DEG, 82, 35, textColor);
  baseDisplay.drawCircle(RECT_DEG.xmin + 79, RECT_DEG.ymin + 7, 2, textColor);

  float humidity = measurement.valuesCo2.humidity;
  textColor = getTextColor(humidity, thresholdsHumidity);
  fillColor = getFillColor(humidity, thresholdsHumidity);
  int humidity10 = round(humidity * 10.0);
  int humidityFix = floor(humidity10 / 10.0);
  int humidityFrc = humidity10 % 10;
  if (fillColor != EPD_WHITE) {
    BoxDisplay::fillRectangle(RECT_HUM, fillColor);
  }
  BoxDisplay::drawAntialiasedText20(formatString(String(humidityFix), FORMAT_3_DIGIT), RECT_HUM, 4, 35, textColor);
  BoxDisplay::drawAntialiasedText08("%", RECT_HUM, 82, TEXT_OFFSET_Y, textColor);
  BoxDisplay::drawAntialiasedText08(".", RECT_HUM, 72, 35, textColor);
  BoxDisplay::drawAntialiasedText08(String(humidityFrc), RECT_HUM, 82, 35, textColor);

  BoxDisplay::renderEnvironment();

  BoxDisplay::flushBuffer();

}

void BoxDisplay::renderChart() {

  BoxDisplay::clearBuffer();

  BoxDisplay::drawOuterBorders(EPD_LIGHT);

  int maxCo2 = 1500;

  int measurementCount = min(57, Measurements::memBufferIndx);
  Measurement measuremnt;
  for (int i = 0; i < measurementCount; i++) {
    measuremnt = Measurements::getOffsetMeasurement(i);
    if (measuremnt.valuesCo2.co2 > 1500) {
      maxCo2 = 3000;
    } else if (measuremnt.valuesCo2.co2 > 3000) {
      maxCo2 = 4500;
    }
  }

  String label2 = String(maxCo2 * 2 / 3);
  String label1 = String(maxCo2 / 3);

  BoxDisplay::drawAntialiasedText08(label2, RECT_CO2, 56 - 10 * label2.length(), 24, EPD_BLACK);
  baseDisplay.drawFastHLine(1, 49, 296, EPD_LIGHT);
  // baseDisplay.drawFastHLine(1, 50, 296, EPD_LIGHT);
  BoxDisplay::drawAntialiasedText08(label1, RECT_CO2, 56 - 10 * label1.length(), 52, EPD_BLACK);
  baseDisplay.drawFastHLine(1, 77, 296, EPD_LIGHT);
  // baseDisplay.drawFastHLine(1, 78, 296, EPD_LIGHT);
  BoxDisplay::drawAntialiasedText08("0", RECT_CO2, 56 - 10, 81, EPD_BLACK);

  int minX;
  int minY;
  int maxY = 103;
  int dimY;
  int limY = 78;
  for (int i = 0; i < measurementCount; i++) {

    measuremnt = Measurements::getOffsetMeasurement(i);

    minX = 290 - i * 4; // 290 is the left border of the right-most bar
    dimY = min(limY, (int)round(measuremnt.valuesCo2.co2 * limY / maxCo2));
    minY = maxY - dimY;

    baseDisplay.drawFastVLine(minX, minY, dimY, EPD_BLACK);
    baseDisplay.drawFastVLine(minX + 1, minY, dimY, EPD_BLACK);
    // baseDisplay.drawFastVLine(minX + 2, minY, dimY, EPD_BLACK);
    
  }

  BoxDisplay::renderEnvironment();

  BoxDisplay::flushBuffer();

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

void BoxDisplay::renderEnvironment() {

  ValuesBat valuesBat = BoxPack::values;

  String networkMessage = "";
  if (BoxConn::getMode() != WIFI_OFF) {
    networkMessage = BoxConn::getAddress();
  }

  BoxDisplay::drawAntialiasedText08(valuesBat.powered ? ">" : "<", RECT_BOT, 210, TEXT_OFFSET_Y, EPD_BLACK);

  String cellPercentFormatted = formatString(String(valuesBat.percent, 1), FORMAT_CELL_PERCENT);
  BoxDisplay::drawAntialiasedText08(cellPercentFormatted, RECT_BOT, 228, TEXT_OFFSET_Y, EPD_BLACK);

  String timeFormatted = BoxClock::getTimeString(BoxClock::getDate());
  BoxDisplay::drawAntialiasedText08(timeFormatted, RECT_BOT, 6, TEXT_OFFSET_Y, EPD_BLACK);

  BoxDisplay::drawAntialiasedText08(networkMessage, RECT_TOP, 6, TEXT_OFFSET_Y, EPD_BLACK);

  String csvCountFormatted = String(Measurements::getCsvBufferIndex());
  int publishableCount = BoxMqtt::isConfiguredToBeActive() ? Measurements::getPublishableCount() : 0;
  if (publishableCount > 0) {
    String mqttCountFormatted = String(Measurements::getPublishableCount());
    int pendingDataSize = csvCountFormatted.length() + mqttCountFormatted.length() + 1; // total string length
    char pendingDataBuf[csvCountFormatted.length() + mqttCountFormatted.length() + 1];
    sprintf(pendingDataBuf, "%s|%s", csvCountFormatted, mqttCountFormatted);
    BoxDisplay::drawAntialiasedText08(pendingDataBuf, RECT_TOP, 288 - pendingDataSize * 10, TEXT_OFFSET_Y, EPD_BLACK);
  } else {
    BoxDisplay::drawAntialiasedText08(csvCountFormatted.c_str(), RECT_TOP, 288 - csvCountFormatted.length() * 10, TEXT_OFFSET_Y, EPD_BLACK);
  }
  

}

void BoxDisplay::toggleState() {
  if (displayState == DISPLAY_STATE_TABLE) {
    displayState = DISPLAY_STATE_CHART;
  } else {
    displayState = DISPLAY_STATE_TABLE;
  }
}

void BoxDisplay::toggleTheme() {
  if (displayTheme == DISPLAY_THEME_LIGHT) {
    displayTheme = DISPLAY_THEME__DARK;
  } else {
    displayTheme = DISPLAY_THEME_LIGHT;
  }
}

String BoxDisplay::formatString(String value, char const *format) {
  char padBuffer[16];
  sprintf(padBuffer, format, value);
  return padBuffer;
}
