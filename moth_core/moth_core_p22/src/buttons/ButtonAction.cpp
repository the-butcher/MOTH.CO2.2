#include "ButtonAction.h"

ButtonHelper ButtonAction::A(GPIO_NUM_11);
ButtonHelper ButtonAction::B(GPIO_NUM_12);
ButtonHelper ButtonAction::C(GPIO_NUM_6);
gpio_num_t ButtonAction::actionPin = GPIO_NUM_0;
std::function<void(std::function<bool(config_t* config)>)> ButtonAction::buttonActionCompleteCallback = nullptr;
uint64_t ButtonAction::ext1Bitmask = 1ULL << ButtonAction::A.gpin | 1ULL << ButtonAction::B.gpin | 1ULL << ButtonAction::C.gpin;
// start with void actions for the beginning
button_action_t ButtonAction::buttonActionA = {"", "", "", nullptr, nullptr};
button_action_t ButtonAction::buttonActionB = {"", "", "", nullptr, nullptr};
button_action_t ButtonAction::buttonActionC = {"", "", "", nullptr, nullptr};

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
            // button B toggles table/chart and light/dark
        }
        ButtonAction::buttonActionC = {
            ">",                                // the symbol for a fast press
            "<",                                // the symbol for a slow press,
            "",                                 // extra information to be displayed for this button
            ButtonAction::toggleDisplayValTFw,  // a function to be executed on fast press
            ButtonAction::toggleDisplayValTBw   // a function to be executed on slow press
        };
    } else {
        // button A changes chart hours
        // button B toggles table/chart and light/dark
    }

    // button C toggles value depending on modus
    return true;
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
 * toggle the primary table display value forward
 */
bool ButtonAction::toggleDisplayValTFw(config_t* config) {
    uint8_t valueCount = DISPLAY_VAL_T___ALT + 1;
    config->displayValTable = (display_val_t_e)((config->displayValTable + 1) % valueCount);
    return true;
}

/**
 * toggle the primary table display value backward
 */
bool ButtonAction::toggleDisplayValTBw(config_t* config) {
    uint8_t valueCount = DISPLAY_VAL_T___ALT + 1;
    config->displayValTable = (display_val_t_e)((config->displayValTable + valueCount - 1) % valueCount);
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
        xTaskCreate(ButtonAction::detectButtonAction, "detect button action", 5000, NULL, 2, NULL);
    }
}

void ButtonAction::detectButtonAction(void* parameter) {
    uint64_t millisA = millis();
    while (millis() - millisA < 1000) {
        if (digitalRead(actionPin) == HIGH) {  // already released
            ButtonAction::handleButtonAction(BUTTON_ACTION_FAST);
            vTaskDelete(NULL);
            return;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    ButtonAction::handleButtonAction(BUTTON_ACTION_SLOW);
    vTaskDelete(NULL);
    return;
}

void ButtonAction::handleButtonAction(button_action_e buttonActionType) {
    std::function<bool(config_t * config)> actionFunction = nullptr;
    if (actionPin == ButtonAction::A.gpin) {
        actionFunction = ButtonAction::getActionFunction(buttonActionA, buttonActionType);
    } else if (actionPin == ButtonAction::B.gpin) {
        actionFunction = ButtonAction::getActionFunction(buttonActionB, buttonActionType);
    } else if (actionPin == ButtonAction::C.gpin) {
        actionFunction = ButtonAction::getActionFunction(buttonActionC, buttonActionType);
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
