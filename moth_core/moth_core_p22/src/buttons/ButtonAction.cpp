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
            // buttonA changes alt by 50m
            // buttonB changes alt by 10m
        } else {
            // button A toggles wifi and sound
            ButtonAction::B.buttonAction = getButtonActionDisplayValMT(config);  // toggle modus and theme
        }
        ButtonAction::C.buttonAction = getButtonActionDisplayValTT(config);  // toggle table value
    } else {
        ButtonAction::A.buttonAction = getButtonActionDisplayValHR(config);  // toggle chart hours
        ButtonAction::B.buttonAction = getButtonActionDisplayValMT(config);  // toggle modus and theme
        ButtonAction::C.buttonAction = getButtonActionDisplayValCC(config);  // toggle chart value
    }

    // button C toggles value depending on modus
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

void ButtonAction::prepareSleep(bool isExt1Wakeup) {
    ButtonAction::A.prepareSleep(isExt1Wakeup);
    ButtonAction::B.prepareSleep(isExt1Wakeup);
    ButtonAction::C.prepareSleep(isExt1Wakeup);
    if (isExt1Wakeup) {
        esp_sleep_enable_ext1_wakeup(ext1Bitmask, ESP_EXT1_WAKEUP_ANY_LOW);
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

void ButtonAction::attachInterrupts() {
    attachInterrupt(ButtonAction::A.ipin, ButtonAction::handleInterruptA, FALLING);
    attachInterrupt(ButtonAction::B.ipin, ButtonAction::handleInterruptB, FALLING);
    attachInterrupt(ButtonAction::C.ipin, ButtonAction::handleInterruptC, FALLING);
}
void ButtonAction::detachInterrupts() {
    detachInterrupt(ButtonAction::A.ipin);
    detachInterrupt(ButtonAction::B.ipin);
    detachInterrupt(ButtonAction::C.ipin);
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
