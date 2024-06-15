#include "ButtonAction.h"

#include "modules/ModuleDisp.h"
#include "modules/ModuleSignal.h"
#include "sensors/SensorBme280.h"
#include "sensors/SensorTime.h"
#include "types/Define.h"

ButtonHelper ButtonAction::A(GPIO_NUM_9);
ButtonHelper ButtonAction::B(GPIO_NUM_6);
ButtonHelper ButtonAction::C(GPIO_NUM_5);
gpio_num_t ButtonAction::actionPin = GPIO_NUM_0;
std::function<void(std::function<bool(config_t& config)>)> ButtonAction::actionCompleteCallback = nullptr;

void ButtonAction::begin(std::function<void(std::function<bool(config_t& config)>)> actionCompleteCallback) {
    ButtonAction::actionCompleteCallback = actionCompleteCallback;
    ButtonAction::A.begin();
    ButtonAction::B.begin();
    ButtonAction::C.begin();
}

/**
 * updates the button actions based on the given config
 * @param config
 */
bool ButtonAction::adapt(config_t& config) {
    ButtonAction::A.buttonAction = getButtonActionFunctionWFBP(config);  // button A toggles wifi and sound
    ButtonAction::C.buttonAction = getButtonActionDisplayValTT(config);  // toggle table value
    return true;
}

button_action_t ButtonAction::getButtonActionDisplayValTT(config_t& config) {
    return {
        ">",                                // the symbol for a fast press
        "<",                                // the symbol for a slow press,
        ButtonAction::toggleDisplayValTFw,  // a function to be executed on fast press
        ButtonAction::toggleDisplayValTBw   // a function to be executed on slow press
    };
}

button_action_t ButtonAction::getButtonActionFunctionWFBP(config_t& config) {
    return {
        config.sign.signalValSound == SIGNAL__VAL______ON ? SYMBOL_NBEEP : SYMBOL_YBEEP,  // the symbol for a fast press
        SYMBOL__WIFI,                                                                     // the symbol for a slow press,
        ButtonAction::toggleBeep,                                                         // to a shorter interval
        ButtonAction::toggleWifi                                                          // a function to be executed on slow press
    };
}

void ButtonAction::attachWakeup() {
    attachInterrupt(ButtonAction::A.ipin, ButtonAction::handleInterruptA, ButtonAction::A.isPressed() ? RISING : FALLING);
    attachInterrupt(ButtonAction::B.ipin, ButtonAction::handleInterruptB, ButtonAction::B.isPressed() ? RISING : FALLING);
    attachInterrupt(ButtonAction::C.ipin, ButtonAction::handleInterruptC, ButtonAction::C.isPressed() ? RISING : FALLING);
}

void ButtonAction::detachWakeup() {
    detachInterrupt(ButtonAction::A.ipin);
    detachInterrupt(ButtonAction::B.ipin);
    detachInterrupt(ButtonAction::C.ipin);
}

void ButtonAction::handleInterruptA() {
    if (ButtonAction::A.isPressed()) {
        ButtonAction::createButtonAction(ButtonAction::A.gpin);
    } else {
        ButtonAction::actionCompleteCallback(nullptr);
    }
}
void ButtonAction::handleInterruptB() {
    if (ButtonAction::B.isPressed()) {
        ButtonAction::createButtonAction(ButtonAction::B.gpin);  // determine action depending on release time
    } else {
        ButtonAction::actionCompleteCallback(nullptr);
    }
}
void ButtonAction::handleInterruptC() {
    if (ButtonAction::C.isPressed()) {
        ButtonAction::createButtonAction(ButtonAction::C.gpin);
    } else {
        ButtonAction::actionCompleteCallback(nullptr);
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
 * button action :: toggle wifi on or off
 */
bool ButtonAction::toggleWifi(config_t& config) {
    if (config.wifi.wifiValPower == WIFI____VAL_P__CUR_N) {  // if it is actually off -> set to pending on
        config.wifi.wifiValPower = WIFI____VAL_P__PND_Y;
        return true;
    } else if (config.wifi.wifiValPower == WIFI____VAL_P__CUR_Y) {  // if it is actually on -> set to pending off
        config.wifi.wifiValPower = WIFI____VAL_P__PND_N;
        return true;
    } else {
        // already in one of the pending states, do nothing
        return false;
    }
}

/**
 * button action :: toggle beep on or off
 */
bool ButtonAction::toggleBeep(config_t& config) {
    if (config.sign.signalValSound == SIGNAL__VAL______ON) {
        config.sign.signalValSound = SIGNAL__VAL_____OFF;
    } else {
        config.sign.signalValSound = SIGNAL__VAL______ON;
    }
    return true;
}

/**
 * button action :: toggle the primary table display value forward
 */
bool ButtonAction::toggleDisplayValTFw(config_t& config) {
    uint8_t valueCount = DISPLAY_VAL_T____HPA + 1;
    config.disp.displayValTable = (display_val_t_e)((config.disp.displayValTable + 1) % valueCount);
    ButtonAction::adapt(config);
    return true;
}

/**
 * button action :: toggle the primary table display value backward
 */
bool ButtonAction::toggleDisplayValTBw(config_t& config) {
    uint8_t valueCount = DISPLAY_VAL_T____HPA + 1;
    config.disp.displayValTable = (display_val_t_e)((config.disp.displayValTable + valueCount - 1) % valueCount);
    ButtonAction::adapt(config);
    return true;
}

/**
 * create a new "detect button action" task
 */
void ButtonAction::createButtonAction(gpio_num_t actionPin) {
    if (ButtonAction::actionPin == 0 && actionPin > 0) {  // only if no other task is still pending
        ButtonAction::actionPin = actionPin;
        xTaskCreate(ButtonAction::detectButtonActionType, "detect button action", 3000, NULL, 2, NULL);
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
    std::function<bool(config_t & config)> actionFunction = nullptr;
    if (actionPin == ButtonAction::A.gpin) {
        actionFunction = ButtonAction::getActionFunction(A.buttonAction, buttonActionType);
    } else if (actionPin == ButtonAction::B.gpin) {
        actionFunction = ButtonAction::getActionFunction(B.buttonAction, buttonActionType);
    } else if (actionPin == ButtonAction::C.gpin) {
        actionFunction = ButtonAction::getActionFunction(C.buttonAction, buttonActionType);
    }
    ButtonAction::actionCompleteCallback(actionFunction);  // return the function to be executed to main, from where it will be called with the config instance
    actionPin = GPIO_NUM_0;
}

std::function<bool(config_t& config)> ButtonAction::getActionFunction(button_action_t buttonAction, button_action_e buttonActionType) {
    if (buttonActionType == BUTTON_ACTION_FAST) {
        return buttonAction.functionFast;
    } else if (buttonActionType == BUTTON_ACTION_SLOW) {
        return buttonAction.functionSlow;
    } else {
        return nullptr;
    }
}
