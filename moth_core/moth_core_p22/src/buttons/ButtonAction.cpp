#include "ButtonAction.h"

#include "modules/ModuleDisplay.h"
#include "modules/ModuleSignal.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorTime.h"
#include "types/Define.h"

ButtonHelper ButtonAction::A(GPIO_NUM_11);
ButtonHelper ButtonAction::B(GPIO_NUM_12);
ButtonHelper ButtonAction::C(GPIO_NUM_6);
gpio_num_t ButtonAction::actionPin = GPIO_NUM_0;
std::function<void(std::function<void(config_t& config)>)> ButtonAction::buttonActionCompleteCallback = nullptr;
uint64_t ButtonAction::ext1Bitmask = 1ULL << ButtonAction::A.gpin | 1ULL << ButtonAction::B.gpin | 1ULL << ButtonAction::C.gpin;

void ButtonAction::begin(std::function<void(std::function<void(config_t& config)>)> buttonActionCompleteCallback) {
    ButtonAction::buttonActionCompleteCallback = buttonActionCompleteCallback;
    ButtonAction::A.begin();
    ButtonAction::B.begin();
    ButtonAction::C.begin();
}

/**
 * updates the button actions based on the given config
 * @param config
 */
bool ButtonAction::configure(config_t& config) {
    if (config.disp.displayValModus == DISPLAY_VAL_M__TABLE) {
        if (config.disp.displayValTable == DISPLAY_VAL_T____ALT) {
            ButtonAction::A.buttonAction = getButtonActionAltitude5050(config);  // buttonA changes alt by 50m
            ButtonAction::B.buttonAction = getButtonActionAltitude1010(config);  // buttonB changes alt by 10m
        } else {
            ButtonAction::A.buttonAction = getButtonActionFunctionWFBP(config);  // button A toggles wifi and sound
            ButtonAction::B.buttonAction = getButtonActionDisplayValMT(config);  // toggle modus and theme
        }
        ButtonAction::C.buttonAction = getButtonActionDisplayValTT(config);  // toggle table value
    } else {
        ButtonAction::A.buttonAction = getButtonActionDisplayValHR(config);  // toggle chart hours
        ButtonAction::B.buttonAction = getButtonActionDisplayValMT(config);  // toggle modus and theme
        ButtonAction::C.buttonAction = getButtonActionDisplayValCC(config);  // toggle chart value
    }
    return true;
}

button_action_t ButtonAction::getButtonActionDisplayValTT(config_t& config) {
    return {
        ">",                                // the symbol for a fast press
        "<",                                // the symbol for a slow press,
        "",                                 // extra information to be displayed for this button
        ButtonAction::toggleDisplayValTFw,  // a function to be executed on fast press
        ButtonAction::toggleDisplayValTBw   // a function to be executed on slow press
    };
}

button_action_t ButtonAction::getButtonActionDisplayValCC(config_t& config) {
    return {
        ">",                                // the symbol for a fast press
        "<",                                // the symbol for a slow press,
        "",                                 // extra information to be displayed for this button
        ButtonAction::toggleDisplayValCFw,  // a function to be executed on fast press
        ButtonAction::toggleDisplayValCBw   // a function to be executed on slow press
    };
}

button_action_t ButtonAction::getButtonActionDisplayValMT(config_t& config) {
    return {
        config.disp.displayValModus == DISPLAY_VAL_M__TABLE ? SYMBOL_CHART : SYMBOL_TABLE,  // the symbol for a fast press
        SYMBOL_THEME,                                                                       // the symbol for a slow press,
        "",                                                                                 // extra information to be displayed for this button
        ButtonAction::toggleDisplayValMod,                                                  // a function to be executed on slow press
        ButtonAction::toggleDisplayValThm                                                   // a function to be executed on fast press
    };
}

button_action_t ButtonAction::getButtonActionDisplayValHR(config_t& config) {
    return {
        config.disp.displayHrsChart != DISPLAY_HRS_C_____01 ? "-" : "",                                     // the symbol for a fast press
        config.disp.displayHrsChart != DISPLAY_HRS_C_____24 ? "+" : "",                                     // the symbol for a slow press,
        "",                                                                                                 // extra information to be displayed for this button
        config.disp.displayHrsChart != DISPLAY_HRS_C_____01 ? ButtonAction::toggleDisplayValHBw : nullptr,  // to a shorter interval
        config.disp.displayHrsChart != DISPLAY_HRS_C_____24 ? ButtonAction::toggleDisplayValHFw : nullptr   // a function to be executed on slow press
    };
}

button_action_t ButtonAction::getButtonActionAltitude1010(config_t& config) {
    return {
        "-",                                // the symbol for a fast press
        "+",                                // the symbol for a slow press,
        "10",                               // extra information to be displayed for this button
        ButtonAction::decrementAltitude10,  // to a shorter interval
        ButtonAction::incrementAltitude10   // a function to be executed on slow press
    };
}

button_action_t ButtonAction::getButtonActionAltitude5050(config_t& config) {
    return {
        "-",                                // the symbol for a fast press
        "+",                                // the symbol for a slow press,
        "50",                               // extra information to be displayed for this button
        ButtonAction::decrementAltitude50,  // to a shorter interval
        ButtonAction::incrementAltitude50   // a function to be executed on slow press
    };
}

button_action_t ButtonAction::getButtonActionFunctionWFBP(config_t& config) {
    return {
        config.sign.signalValSound == SIGNAL__VAL______ON ? SYMBOL_NBEEP : SYMBOL_YBEEP,  // the symbol for a fast press
        SYMBOL__WIFI,                                                                     // the symbol for a slow press,
        "",                                                                               // extra information to be displayed for this button
        ButtonAction::toggleBeep,                                                         // to a shorter interval
        ButtonAction::toggleWifi                                                          // a function to be executed on slow press
    };
}

/**
 * sets any gpio holds required, then returns the ext1Bitmask value suitable to setup ext1 wakeup
 */
void ButtonAction::prepareSleep(wakeup_action_e wakeupType) {
    ButtonAction::A.prepareSleep(wakeupType);
    ButtonAction::B.prepareSleep(wakeupType);
    ButtonAction::C.prepareSleep(wakeupType);
    // if (wakeupType == WAKEUP_ACTION_BUTN) {
    //     esp_sleep_enable_ext1_wakeup(ButtonAction::ext1Bitmask, ESP_EXT1_WAKEUP_ANY_LOW);
    // }
}

void ButtonAction::attachWakeup(wakeup_action_e wakeupType) {
    if (wakeupType == WAKEUP_ACTION_BUTN) {
        attachInterrupt(ButtonAction::A.ipin, ButtonAction::handleInterruptA, ButtonAction::A.isPressed() ? RISING : FALLING);
        attachInterrupt(ButtonAction::B.ipin, ButtonAction::handleInterruptB, ButtonAction::B.isPressed() ? RISING : FALLING);
        attachInterrupt(ButtonAction::C.ipin, ButtonAction::handleInterruptC, ButtonAction::C.isPressed() ? RISING : FALLING);
    }
}

void ButtonAction::detachWakeup(wakeup_action_e wakeupType) {
    if (wakeupType == WAKEUP_ACTION_BUTN) {
        detachInterrupt(ButtonAction::A.ipin);
        detachInterrupt(ButtonAction::B.ipin);
        detachInterrupt(ButtonAction::C.ipin);
    }
}

void ButtonAction::handleInterruptA() {
    if (ButtonAction::A.isPressed()) {
        ButtonAction::createButtonAction(ButtonAction::A.gpin);
    } else {
        ButtonAction::buttonActionCompleteCallback(nullptr);
    }
}
void ButtonAction::handleInterruptB() {
    if (ButtonAction::B.isPressed()) {
        ButtonAction::createButtonAction(ButtonAction::B.gpin);  // determine action depending on release time
    } else {
        ButtonAction::buttonActionCompleteCallback(nullptr);
    }
}
void ButtonAction::handleInterruptC() {
    if (ButtonAction::C.isPressed()) {
        ButtonAction::createButtonAction(ButtonAction::C.gpin);
    } else {
        ButtonAction::buttonActionCompleteCallback(nullptr);
    }
}

gpio_num_t ButtonAction::getActionPin() {
    return ButtonAction::actionPin;
}

gpio_num_t ButtonAction::getPressedPin() {
    if (ButtonAction::A.isPressed()) {
        return ButtonAction::A.gpin;
    } else if (ButtonAction::B.isPressed()) {
        return ButtonAction::B.gpin;
    } else if (ButtonAction::C.isPressed()) {
        return ButtonAction::C.gpin;
    }
    return GPIO_NUM_0;
}

/**
 * button action :: toggle the chart hours to the next higher value
 */
void ButtonAction::toggleDisplayValHFw(config_t& config) {
    if (config.disp.displayHrsChart == DISPLAY_HRS_C_____01) {
        config.disp.displayHrsChart = DISPLAY_HRS_C_____03;
    } else if (config.disp.displayHrsChart == DISPLAY_HRS_C_____03) {
        config.disp.displayHrsChart = DISPLAY_HRS_C_____06;
    } else if (config.disp.displayHrsChart == DISPLAY_HRS_C_____06) {
        config.disp.displayHrsChart = DISPLAY_HRS_C_____12;
    } else if (config.disp.displayHrsChart == DISPLAY_HRS_C_____12) {
        config.disp.displayHrsChart = DISPLAY_HRS_C_____24;
    } else {
        // already at DISPLAY_HRS_C_____24
    }
    ButtonAction::configure(config);
}

/**
 * button action :: toggle the chart hours to the next lower value
 */
void ButtonAction::toggleDisplayValHBw(config_t& config) {
    if (config.disp.displayHrsChart == DISPLAY_HRS_C_____24) {
        config.disp.displayHrsChart = DISPLAY_HRS_C_____12;
    } else if (config.disp.displayHrsChart == DISPLAY_HRS_C_____12) {
        config.disp.displayHrsChart = DISPLAY_HRS_C_____06;
    } else if (config.disp.displayHrsChart == DISPLAY_HRS_C_____06) {
        config.disp.displayHrsChart = DISPLAY_HRS_C_____03;
    } else if (config.disp.displayHrsChart == DISPLAY_HRS_C_____03) {
        config.disp.displayHrsChart = DISPLAY_HRS_C_____01;
    } else {
        // already at DISPLAY_HRS_C_____01
    }
    ButtonAction::configure(config);
}

/**
 * button action :: toggle wifi on or off
 */
void ButtonAction::toggleWifi(config_t& config) {
    if (config.wifi.wifiValPower == WIFI____VAL_P_CUR_N) {  // if it is actually off -> set to pending on
        config.wifi.wifiValPower = WIFI____VAL_P_PND_Y;
    } else if (config.wifi.wifiValPower == WIFI____VAL_P_CUR_Y) {  // if it is actually on -> set to pending off
        config.wifi.wifiValPower = WIFI____VAL_P_PND_Y;
    } else {
        // already in one of the pending states, do nothing
    }
}

/**
 * button action :: toggle beep on or off
 */
void ButtonAction::toggleBeep(config_t& config) {
    if (config.sign.signalValSound == SIGNAL__VAL______ON) {
        config.sign.signalValSound = SIGNAL__VAL_____OFF;
    } else {
        config.sign.signalValSound = SIGNAL__VAL______ON;
    }
}

/**
 * button action :: decrement the base altitude by 10m
 */
void ButtonAction::decrementAltitude10(config_t& config) {
    config.altitudeBaselevel = config.altitudeBaselevel - 10;
    config.pressureZerolevel = SensorBme280::getPressureZerolevel(config.altitudeBaselevel, SensorBme280::readval().pressure);
}

/**
 * button action :: increment the base altitude by 10m
 */
void ButtonAction::incrementAltitude10(config_t& config) {
    config.altitudeBaselevel = config.altitudeBaselevel + 10;
    config.pressureZerolevel = SensorBme280::getPressureZerolevel(config.altitudeBaselevel, SensorBme280::readval().pressure);
}

/**
 * button action :: decrement the base altitude by 50m
 */
void ButtonAction::decrementAltitude50(config_t& config) {
    config.altitudeBaselevel = config.altitudeBaselevel - 50;
    config.pressureZerolevel = SensorBme280::getPressureZerolevel(config.altitudeBaselevel, SensorBme280::readval().pressure);
}

/**
 * button action :: increment the base altitude by 10m
 */
void ButtonAction::incrementAltitude50(config_t& config) {
    config.altitudeBaselevel = config.altitudeBaselevel + 50;
    config.pressureZerolevel = SensorBme280::getPressureZerolevel(config.altitudeBaselevel, SensorBme280::readval().pressure);
}

/**
 * button action :: toggle the primary table display value forward
 */
void ButtonAction::toggleDisplayValTFw(config_t& config) {
    uint8_t valueCount = DISPLAY_VAL_T____ALT + 1;
    config.disp.displayValTable = (display_val_t_e)((config.disp.displayValTable + 1) % valueCount);
    ButtonAction::configure(config);
}

/**
 * button action :: toggle the primary table display value backward
 */
void ButtonAction::toggleDisplayValTBw(config_t& config) {
    uint8_t valueCount = DISPLAY_VAL_T____ALT + 1;
    config.disp.displayValTable = (display_val_t_e)((config.disp.displayValTable + valueCount - 1) % valueCount);
    ButtonAction::configure(config);
}

/**
 * button action :: toggle the primary table display value forward
 */
void ButtonAction::toggleDisplayValCFw(config_t& config) {
    uint8_t valueCount = DISPLAY_VAL_C____NRG + 1;
    config.disp.displayValChart = (display_val_c_e)((config.disp.displayValChart + 1) % valueCount);
}

/**
 * button action :: toggle the primary table display value backward
 */
void ButtonAction::toggleDisplayValCBw(config_t& config) {
    uint8_t valueCount = DISPLAY_VAL_C____NRG + 1;
    config.disp.displayValChart = (display_val_c_e)((config.disp.displayValChart + valueCount - 1) % valueCount);
}

/**
 * button action :: toggle between table and chart
 */
void ButtonAction::toggleDisplayValMod(config_t& config) {
    if (config.disp.displayValModus == DISPLAY_VAL_M__TABLE) {
        config.disp.displayValModus = DISPLAY_VAL_M__CHART;
    } else {
        config.disp.displayValModus = DISPLAY_VAL_M__TABLE;
    }
    ButtonAction::configure(config);
}

void ButtonAction::toggleDisplayValThm(config_t& config) {
    if (config.disp.displayValTheme == DISPLAY_THM____LIGHT) {
        config.disp.displayValTheme = DISPLAY_THM_____DARK;
    } else {
        config.disp.displayValTheme = DISPLAY_THM____LIGHT;
    }
}

/**
 * create a new "detect button action" task
 */
void ButtonAction::createButtonAction(gpio_num_t actionPin) {
    if (ButtonAction::actionPin == 0 && actionPin > 0) {  // only if no other task is still pending
        ButtonAction::actionPin = actionPin;
        xTaskCreate(ButtonAction::detectButtonActionType, "detect button action", 5000, NULL, 2, NULL);
    }
}

/**
 * waits max 1 seconds or until action pin becomes HIGH (is released), whichever happens first
 */
void ButtonAction::detectButtonActionType(void* parameter) {
    uint64_t millisA = millis();
    while (millis() - millisA < MILLISECONDS_PER______SECOND) {
        if (digitalRead(actionPin) == HIGH) {  // already released
            ButtonAction::handleButtonActionType(BUTTON_ACTION_FAST);
            vTaskDelete(NULL);
            return;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    ButtonAction::handleButtonActionType(BUTTON_ACTION_SLOW);
    vTaskDelete(NULL);
    return;
}

/**
 * once it is clear if the button has been pressed short or long retrieve function currently attached to the button, then pass it back to the callback
 */
void ButtonAction::handleButtonActionType(button_action_e buttonActionType) {
    std::function<void(config_t & config)> actionFunction = nullptr;
    if (actionPin == ButtonAction::A.gpin) {
        actionFunction = ButtonAction::getActionFunction(A.buttonAction, buttonActionType);
    } else if (actionPin == ButtonAction::B.gpin) {
        actionFunction = ButtonAction::getActionFunction(B.buttonAction, buttonActionType);
    } else if (actionPin == ButtonAction::C.gpin) {
        actionFunction = ButtonAction::getActionFunction(C.buttonAction, buttonActionType);
    }
    ButtonAction::buttonActionCompleteCallback(actionFunction);  // return the function to be executed to main, from where it will be called with the config instance
    actionPin = GPIO_NUM_0;
}

std::function<void(config_t& config)> ButtonAction::getActionFunction(button_action_t buttonAction, button_action_e buttonActionType) {
    if (buttonActionType == BUTTON_ACTION_FAST) {
        return buttonAction.functionFast;
    } else if (buttonActionType == BUTTON_ACTION_SLOW) {
        return buttonAction.functionSlow;
    } else {
        return nullptr;
    }
}
