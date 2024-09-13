#include "ModuleDisp.h"

#include "ModuleCard.h"
#include "buttons/ButtonAction.h"

Adafruit_SH1107 ModuleDisp::display(64, 128, &Wire, -1, 200000, 100000);

const char FORMAT_3_DIGIT[] = "%3s";
const char FORMAT_4_DIGIT[] = "%4s";
const uint8_t CHAR_DIM_X6 = 6;

void ModuleDisp::configure(config_t& config) {

    // ModuleCard::begin();

    File dispFileJson = LittleFS.open(DISP_CONFIG_JSON.c_str(), FILE_READ);
    if (dispFileJson) {
        JsonDocument jsonDocument;
        DeserializationError error = deserializeJson(jsonDocument, dispFileJson);
        if (!error) {

            config.disp.configStatus = CONFIG_STAT__APPLIED;

            // display co2 threshold and reference
            config.disp.thresholdsPms.wHi = jsonDocument[JSON_KEY_______PMS][JSON_KEY_WARN_HIGH] | config.disp.thresholdsPms.wHi;
            config.disp.thresholdsPms.rHi = jsonDocument[JSON_KEY_______PMS][JSON_KEY_RISK_HIGH] | config.disp.thresholdsPms.rHi;
            config.spms.lpFilterRatioCurr = jsonDocument[JSON_KEY_______PMS][JSON_KEY_______LPA] | config.spms.lpFilterRatioCurr;

            // display deg thresholds
            config.disp.thresholdsDeg.rLo = jsonDocument[JSON_KEY_______DEG][JSON_KEY_RISK__LOW] | config.disp.thresholdsDeg.rLo;
            config.disp.thresholdsDeg.wLo = jsonDocument[JSON_KEY_______DEG][JSON_KEY_WARN__LOW] | config.disp.thresholdsDeg.wLo;
            config.disp.thresholdsDeg.wHi = jsonDocument[JSON_KEY_______DEG][JSON_KEY_WARN_HIGH] | config.disp.thresholdsDeg.wHi;
            config.disp.thresholdsDeg.rHi = jsonDocument[JSON_KEY_______DEG][JSON_KEY_RISK_HIGH] | config.disp.thresholdsDeg.rHi;
            config.sbme.temperatureOffset = jsonDocument[JSON_KEY_______DEG][JSON_KEY____OFFSET] | config.sbme.temperatureOffset;

            // display deg scale (unit)
            bool isFahrenheit = jsonDocument[JSON_KEY_______DEG][JSON_KEY_______C2F] | config.disp.displayDegScale == DISPLAY_VAL_D______F;
            config.disp.displayDegScale = isFahrenheit ? DISPLAY_VAL_D______F : DISPLAY_VAL_D______C;

            // display hum thresholds
            config.disp.thresholdsHum.rLo = jsonDocument[JSON_KEY_______HUM][JSON_KEY_RISK__LOW] | config.disp.thresholdsHum.rLo;
            config.disp.thresholdsHum.wLo = jsonDocument[JSON_KEY_______HUM][JSON_KEY_WARN__LOW] | config.disp.thresholdsHum.wLo;
            config.disp.thresholdsHum.wHi = jsonDocument[JSON_KEY_______HUM][JSON_KEY_WARN_HIGH] | config.disp.thresholdsHum.wHi;
            config.disp.thresholdsHum.rHi = jsonDocument[JSON_KEY_______HUM][JSON_KEY_RISK_HIGH] | config.disp.thresholdsHum.rHi;

            config.sbme.lpFilterRatioCurr = jsonDocument[JSON_KEY_______BME][JSON_KEY_______LPA] | config.sbme.lpFilterRatioCurr;

            String timezone = jsonDocument[JSON_KEY__TIMEZONE] | "";
            if (timezone != "") {
                timezone.toCharArray(config.time.timezone, 64);
            }

        } else {
            // TODO :: handle this condition
        }
        dispFileJson.close();
    }
}

void ModuleDisp::begin() {
    display.begin(0x3C, true);  // Address 0x3C default , true
    delay(250);
    display.display();  // splashscreen
    display.setRotation(1);
}

void ModuleDisp::renderHeader() {
    renderButton(ButtonAction::A.buttonAction, 0);
    renderButton(ButtonAction::B.buttonAction, 28);
    renderButton(ButtonAction::C.buttonAction, 56);
}

void ModuleDisp::renderFooter(config_t& config) {

    display.setTextSize(1);

    int charPosFooter = 24;
    if (config.sign.signalValSound == SIGNAL__VAL______ON) {
        display.setCursor(charPosFooter, 56);
        display.print(SYMBOL_YBEEP);
        charPosFooter += 12;
    }

    if (config.wifi.wifiValPower == WIFI____VAL_P__CUR_Y) {
        String address = ModuleWifi::getAddress();
        display.setCursor(128 - address.length() * CHAR_DIM_X6, 56);
        display.print(address);
        // charPosFooter += (address.length() + 1) * CHAR_DIM_X6;
    }
    // ModuleDisp::drawAntialiasedText06(SensorTime::getDateTimeDisplayString(SensorTime::getSecondstime()), RECT_BOT, charPosFooter, TEXT_OFFSET_Y, EPD_BLACK);
}

void ModuleDisp::renderButton(button_action_t buttonAction, uint16_t y) {
    display.setTextSize(1);
    display.setCursor(0, y);
    display.print(buttonAction.symbolSlow);
    display.print(buttonAction.symbolFast);
}

void ModuleDisp::renderTable(values_all_t& measurement, config_t& config) {

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);

    ModuleDisp::renderHeader();

    display.setTextSize(1);

    String unit;
    String value;
    if (config.disp.displayValTable == DISPLAY_VAL_T____010) {
        unit = "PM 1.0 ug/m3";
        value = ModuleDisp::formatString(String(measurement.valuesPms.pm010), FORMAT_4_DIGIT);
    } else if (config.disp.displayValTable == DISPLAY_VAL_T____025) {
        unit = "PM 2.5 ug/m3";
        value = ModuleDisp::formatString(String(measurement.valuesPms.pm025), FORMAT_4_DIGIT);
    } else if (config.disp.displayValTable == DISPLAY_VAL_T____100) {
        unit = "PM 10.0 ug/m3";
        value = ModuleDisp::formatString(String(measurement.valuesPms.pm100), FORMAT_4_DIGIT);
    } else if (config.disp.displayValTable == DISPLAY_VAL_T____HPA) {
        unit = "pressure hPa";
        value = ModuleDisp::formatString(String(measurement.valuesBme.pressure, 0), FORMAT_4_DIGIT);
    }

    display.setCursor(128 - unit.length() * CHAR_DIM_X6, 0);
    display.print(unit);

    display.setCursor(32, 38);

    // set font here
    display.setFont(&smb36pt);
    // display.setTextSize(4);

    display.println(value);

    // unset font here
    display.setFont(NULL);  // reset to default 6x8 font

    ModuleDisp::renderFooter(config);

    delay(10);
    yield();
    display.display();
}

void ModuleDisp::renderEntry(config_t& config) {
    // nothing
}

String ModuleDisp::formatString(String value, char const* format) {
    char padBuffer[16];
    sprintf(padBuffer, format, value);
    return padBuffer;
}
