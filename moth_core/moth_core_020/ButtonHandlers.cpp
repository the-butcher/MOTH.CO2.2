#include "esp32-hal-gpio.h"
#include "pins_arduino.h"
#include "ButtonHandlers.h"
#include "BoxDisplay.h"
#include "SensorPmsa003i.h"
#include "BoxConn.h"

ButtonHandler ButtonHandlers::A = ButtonHandler(GPIO_NUM_11);
ButtonHandler ButtonHandlers::B = ButtonHandler(GPIO_NUM_12);
ButtonHandler ButtonHandlers::C = ButtonHandler(GPIO_NUM_6);

void ButtonHandlers::begin() {

  ButtonHandlers::A.begin();
  ButtonHandlers::B.begin();
  ButtonHandlers::C.begin();

  ButtonHandlers::assignWifiAndPms(); // button A
  ButtonHandlers::assignThemeAndState(); // button B

  // C will be value FW and BW at all times
  ButtonHandlers::C.buttonActionFast = {
    LOOP_REASON___TOGGLE_VALUE_FW,
    ">"
  };
  ButtonHandlers::C.buttonActionSlow = {
    LOOP_REASON___TOGGLE_VALUE_BW,
    "<"
  };

}

void ButtonHandlers::assignWifiAndPms() {
  ButtonHandlers::A.buttonActionFast = {
    SensorPmsa003i::ACTIVE ? LOOP_REASON______TOGGLE___PMS : LOOP_REASON___________UNKNOWN,
    SensorPmsa003i::ACTIVE ? BoxDisplay::SYMBOL__PM_M : ""
  };
  ButtonHandlers::A.buttonActionSlow = {
    BoxConn::getMode() == WIFI_OFF ? LOOP_REASON______WIFI______ON : LOOP_REASON______WIFI_____OFF,
    BoxDisplay::SYMBOL__WIFI
  };
  ButtonHandlers::A.extraLabel = "";
}

void ButtonHandlers::assignChartHours() {
  ButtonHandlers::A.buttonActionFast = {
    BoxDisplay::getChartMeasurementHours() > 1 ? LOOP_REASON___TOGGLE_HOURS_BW : LOOP_REASON___________UNKNOWN,
    BoxDisplay::getChartMeasurementHours() > 1 ? "-" : ""
  };
  ButtonHandlers::A.buttonActionSlow = {
    BoxDisplay::getChartMeasurementHours() < 24 ? LOOP_REASON___TOGGLE_HOURS_FW : LOOP_REASON___________UNKNOWN,
    BoxDisplay::getChartMeasurementHours() < 24 ? "+" : ""
  };
  ButtonHandlers::A.extraLabel = "";
}

void ButtonHandlers::assignThemeAndState() {
  ButtonHandlers::B.buttonActionFast = {
    LOOP_REASON______TOGGLE_STATE,
    BoxDisplay::getState() == DISPLAY_STATE_TABLE ? BoxDisplay::SYMBOL_CHART : BoxDisplay::SYMBOL_TABLE
  };
  ButtonHandlers::B.buttonActionSlow = {
    LOOP_REASON______TOGGLE_THEME,
    BoxDisplay::SYMBOL_THEME
  };
  ButtonHandlers::B.extraLabel = "";
}

void ButtonHandlers::assignAltitudeModifiers() {

  ButtonHandlers::A.buttonActionFast = {
    LOOP_REASON___DEL_50_ALTITUDE,
    "-"
  };
  ButtonHandlers::A.buttonActionSlow = {
    LOOP_REASON___ADD_50_ALTITUDE,
    "+"
  };  
  ButtonHandlers::A.extraLabel = "50";

  ButtonHandlers::B.buttonActionFast = {
    LOOP_REASON___DEL_10_ALTITUDE,
    "-"
  };
  ButtonHandlers::B.buttonActionSlow = {
    LOOP_REASON___ADD_10_ALTITUDE,
    "+"
  };
  ButtonHandlers::B.extraLabel = "10";

}


