#include "ButtonAction.h"

ButtonHelper ButtonAction::A(GPIO_NUM_11);
ButtonHelper ButtonAction::B(GPIO_NUM_12);
ButtonHelper ButtonAction::C(GPIO_NUM_6);
gpio_num_t ButtonAction::actionPin = GPIO_NUM_0;
std::function<void(std::function<bool(config_t* config)>)> ButtonAction::buttonActionCompleteCallback = nullptr;
uint64_t ButtonAction::ext1Bitmask = 1ULL << ButtonAction::A.gpin | 1ULL << ButtonAction::B.gpin | 1ULL << ButtonAction::C.gpin;

void ButtonAction::begin(std::function<void(std::function<bool(config_t* config)>)> buttonActionCompleteCallback) {
    ButtonAction::buttonActionCompleteCallback = buttonActionCompleteCallback;
    ButtonAction::A.begin();
    ButtonAction::B.begin();
    ButtonAction::C.begin();
}

bool ButtonAction::configure(config_t* config) {
    // TODO :: reassign button_action depending on config
    if (config->displayValModus == DISPLAY_VAL_M_TABLE) {
        if (config->displayValTable == DISPLAY_VAL_T___ALT) {
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

button_action_t ButtonAction::getButtonActionDisplayValTT(config_t* config) {
    return {
        ">",                                // the symbol for a fast press
        "<",                                // the symbol for a slow press,
        "",                                 // extra information to be displayed for this button
        ButtonAction::toggleDisplayValTFw,  // a function to be executed on fast press
        ButtonAction::toggleDisplayValTBw   // a function to be executed on slow press
    };
}

button_action_t ButtonAction::getButtonActionDisplayValCC(config_t* config) {
    return {
        ">",                                // the symbol for a fast press
        "<",                                // the symbol for a slow press,
        "",                                 // extra information to be displayed for this button
        ButtonAction::toggleDisplayValCFw,  // a function to be executed on fast press
        ButtonAction::toggleDisplayValCBw   // a function to be executed on slow press
    };
}

button_action_t ButtonAction::getButtonActionDisplayValMT(config_t* config) {
    return {
        config->displayValModus == DISPLAY_VAL_M_TABLE ? SYMBOL_CHART : SYMBOL_TABLE,  // the symbol for a fast press
        SYMBOL_THEME,                                                                  // the symbol for a slow press,
        "",                                                                            // extra information to be displayed for this button
        ButtonAction::toggleDisplayValMod,                                             // a function to be executed on slow press
        ButtonAction::toggleDisplayValThm                                              // a function to be executed on fast press
    };
}

button_action_t ButtonAction::getButtonActionDisplayValHR(config_t* config) {
    return {
        config->displayHrsChart != DISPLAY_HRS_C____01 ? "-" : "",                                     // the symbol for a fast press
        config->displayHrsChart != DISPLAY_HRS_C____24 ? "+" : "",                                     // the symbol for a slow press,
        "",                                                                                            // extra information to be displayed for this button
        config->displayHrsChart != DISPLAY_HRS_C____01 ? ButtonAction::toggleDisplayValHBw : nullptr,  // to a shorter interval
        config->displayHrsChart != DISPLAY_HRS_C____24 ? ButtonAction::toggleDisplayValHFw : nullptr   // a function to be executed on slow press
    };
}

button_action_t ButtonAction::getButtonActionAltitude1010(config_t* config) {
    return {
        "-",                                // the symbol for a fast press
        "+",                                // the symbol for a slow press,
        "10",                               // extra information to be displayed for this button
        ButtonAction::decrementAltitude10,  // to a shorter interval
        ButtonAction::incrementAltitude10   // a function to be executed on slow press
    };
}

button_action_t ButtonAction::getButtonActionAltitude5050(config_t* config) {
    return {
        "-",                                // the symbol for a fast press
        "+",                                // the symbol for a slow press,
        "50",                               // extra information to be displayed for this button
        ButtonAction::decrementAltitude50,  // to a shorter interval
        ButtonAction::incrementAltitude50   // a function to be executed on slow press
    };
}

button_action_t ButtonAction::getButtonActionFunctionWFBP(config_t* config) {
    return {
        config->isBeep ? SYMBOL_NBEEP : SYMBOL_YBEEP,  // the symbol for a fast press
        SYMBOL__WIFI,                                  // the symbol for a slow press,
        "",                                            // extra information to be displayed for this button
        ButtonAction::toggleBeep,                      // to a shorter interval
        ButtonAction::toggleWifi                       // a function to be executed on slow press
    };
}

void ButtonAction::prepareSleep(wakeup_e wakeupType) {
    ButtonAction::A.prepareSleep(wakeupType);
    ButtonAction::B.prepareSleep(wakeupType);
    ButtonAction::C.prepareSleep(wakeupType);
    if (wakeupType == WAKEUP_BUTTONS) {
        esp_sleep_enable_ext1_wakeup(ButtonAction::ext1Bitmask, ESP_EXT1_WAKEUP_ANY_LOW);
    }
}

void ButtonAction::attachWakeup(wakeup_e wakeupType) {
    if (wakeupType == WAKEUP_BUTTONS) {
        attachInterrupt(ButtonAction::A.ipin, ButtonAction::handleInterruptA, FALLING);
        attachInterrupt(ButtonAction::B.ipin, ButtonAction::handleInterruptB, FALLING);
        attachInterrupt(ButtonAction::C.ipin, ButtonAction::handleInterruptC, FALLING);
    }
}
void ButtonAction::detachWakeup(wakeup_e wakeupType) {
    if (wakeupType == WAKEUP_BUTTONS) {
        detachInterrupt(ButtonAction::A.ipin);
        detachInterrupt(ButtonAction::B.ipin);
        detachInterrupt(ButtonAction::C.ipin);
    }
}

void ButtonAction::handleInterruptA() {
    ButtonAction::createButtonAction(ButtonAction::A.gpin);
}
void ButtonAction::handleInterruptB() {
    ButtonAction::createButtonAction(ButtonAction::B.gpin);
}
void ButtonAction::handleInterruptC() {
    ButtonAction::createButtonAction(ButtonAction::C.gpin);
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
bool ButtonAction::toggleDisplayValHFw(config_t* config) {
    if (config->displayHrsChart == DISPLAY_HRS_C____01) {
        config->displayHrsChart = DISPLAY_HRS_C____03;
    } else if (config->displayHrsChart == DISPLAY_HRS_C____03) {
        config->displayHrsChart = DISPLAY_HRS_C____06;
    } else if (config->displayHrsChart == DISPLAY_HRS_C____06) {
        config->displayHrsChart = DISPLAY_HRS_C____12;
    } else if (config->displayHrsChart == DISPLAY_HRS_C____12) {
        config->displayHrsChart = DISPLAY_HRS_C____24;
    } else {
        // already at DISPLAY_HRS_C____24
        return false;
    }
    return true;
}

/**
 * button action :: toggle the chart hours to the next lower value
 */
bool ButtonAction::toggleDisplayValHBw(config_t* config) {
    if (config->displayHrsChart == DISPLAY_HRS_C____24) {
        config->displayHrsChart = DISPLAY_HRS_C____12;
    } else if (config->displayHrsChart == DISPLAY_HRS_C____12) {
        config->displayHrsChart = DISPLAY_HRS_C____06;
    } else if (config->displayHrsChart == DISPLAY_HRS_C____06) {
        config->displayHrsChart = DISPLAY_HRS_C____03;
    } else if (config->displayHrsChart == DISPLAY_HRS_C____03) {
        config->displayHrsChart = DISPLAY_HRS_C____01;
    } else {
        // already at DISPLAY_HRS_C____01
        return false;
    }
    return true;
}

/**
 * button action :: toggle wifi on or off
 */
bool ButtonAction::toggleWifi(config_t* config) {
    config->isWifi = !config->isWifi;
    return true;
}

/**
 * button action :: toggle beep on or off
 */
bool ButtonAction::toggleBeep(config_t* config) {
    config->isBeep = !config->isBeep;
    return true;
}

/**
 * button action :: decrement the base altitude by 10m
 */
bool ButtonAction::decrementAltitude10(config_t* config) {
    config->altitudeBaselevel = config->altitudeBaselevel - 10;
    config->pressureZerolevel = SensorBme280::getPressureZerolevel(config->altitudeBaselevel, SensorBme280::readval().pressure);
    return true;
}

/**
 * button action :: increment the base altitude by 10m
 */
bool ButtonAction::incrementAltitude10(config_t* config) {
    config->altitudeBaselevel = config->altitudeBaselevel + 10;
    config->pressureZerolevel = SensorBme280::getPressureZerolevel(config->altitudeBaselevel, SensorBme280::readval().pressure);
    return true;
}

/**
 * button action :: decrement the base altitude by 50m
 */
bool ButtonAction::decrementAltitude50(config_t* config) {
    config->altitudeBaselevel = config->altitudeBaselevel - 50;
    config->pressureZerolevel = SensorBme280::getPressureZerolevel(config->altitudeBaselevel, SensorBme280::readval().pressure);
    return true;
}

/**
 * button action :: increment the base altitude by 10m
 */
bool ButtonAction::incrementAltitude50(config_t* config) {
    config->altitudeBaselevel = config->altitudeBaselevel + 50;
    config->pressureZerolevel = SensorBme280::getPressureZerolevel(config->altitudeBaselevel, SensorBme280::readval().pressure);
    return true;
}

/**
 * button action :: toggle the primary table display value forward
 */
bool ButtonAction::toggleDisplayValTFw(config_t* config) {
    uint8_t valueCount = DISPLAY_VAL_T___ALT + 1;
    config->displayValTable = (display_val_t_e)((config->displayValTable + 1) % valueCount);
    return true;
}

/**
 * button action :: toggle the primary table display value backward
 */
bool ButtonAction::toggleDisplayValTBw(config_t* config) {
    uint8_t valueCount = DISPLAY_VAL_T___ALT + 1;
    config->displayValTable = (display_val_t_e)((config->displayValTable + valueCount - 1) % valueCount);
    return true;
}

/**
 * button action :: toggle the primary table display value forward
 */
bool ButtonAction::toggleDisplayValCFw(config_t* config) {
    uint8_t valueCount = DISPLAY_VAL_C___ALT + 1;
    config->displayValChart = (display_val_c_e)((config->displayValChart + 1) % valueCount);
    return true;
}

/**
 * button action :: toggle the primary table display value backward
 */
bool ButtonAction::toggleDisplayValCBw(config_t* config) {
    uint8_t valueCount = DISPLAY_VAL_C___ALT + 1;
    config->displayValChart = (display_val_c_e)((config->displayValChart + valueCount - 1) % valueCount);
    return true;
}

/**
 * button action :: toggle between table and chart
 */
bool ButtonAction::toggleDisplayValMod(config_t* config) {
    if (config->displayValModus == DISPLAY_VAL_M_TABLE) {
        config->displayValModus = DISPLAY_VAL_M_CHART;
    } else {
        config->displayValModus = DISPLAY_VAL_M_TABLE;
    }
    return true;
}

bool ButtonAction::toggleDisplayValThm(config_t* config) {
    if (config->displayValTheme == DISPLAY_THM___LIGHT) {
        config->displayValTheme = DISPLAY_THM____DARK;
    } else {
        config->displayValTheme = DISPLAY_THM___LIGHT;
    }
    return true;
}

// bool ButtonAction::accepts(gpio_num_t actionPin) {
//     return actionPin == ButtonAction::A.gpin || actionPin == ButtonAction::B.gpin || actionPin == ButtonAction::C.gpin;
// }

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
    std::function<bool(config_t * config)> actionFunction = nullptr;
    if (actionPin == ButtonAction::A.gpin) {
        actionFunction = ButtonAction::getActionFunction(A.buttonAction, buttonActionType);
    } else if (actionPin == ButtonAction::B.gpin) {
        actionFunction = ButtonAction::getActionFunction(B.buttonAction, buttonActionType);
    } else if (actionPin == ButtonAction::C.gpin) {
        actionFunction = ButtonAction::getActionFunction(C.buttonAction, buttonActionType);
    }
    buttonActionCompleteCallback(actionFunction);  // return the function to be executed to main, from where it will be called with the config instance
    actionPin = GPIO_NUM_0;
}

std::function<bool(config_t* config)> ButtonAction::getActionFunction(button_action_t buttonAction, button_action_e buttonActionType) {
    if (buttonActionType == BUTTON_ACTION_FAST) {
        return buttonAction.functionFast;
    } else if (buttonActionType == BUTTON_ACTION_SLOW) {
        return buttonAction.functionSlow;
    } else {
        return nullptr;
    }
}
