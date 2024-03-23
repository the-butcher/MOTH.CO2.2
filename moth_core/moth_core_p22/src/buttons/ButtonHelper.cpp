#include "ButtonHelper.h"

ButtonHelper::ButtonHelper(gpio_num_t gpin) {
    this->gpin = gpin;
    this->ipin = digitalPinToInterrupt(gpin);
    this->buttonAction = {"", "", "", nullptr, nullptr};
}

void ButtonHelper::begin() {
    pinMode(gpin, INPUT_PULLUP);
    rtc_gpio_deinit(gpin);
}

void ButtonHelper::prepareSleep(wakeup_action_e wakeupType) {
    if (wakeupType == WAKEUP_ACTION_BUTN) {
        gpio_hold_en(gpin);
        rtc_gpio_pullup_en(gpin);
        rtc_gpio_pulldown_dis(gpin);
    } else {
        gpio_hold_dis(gpin);
    }
}

bool ButtonHelper::isPressed() {
    return digitalRead(gpin) == LOW;
}