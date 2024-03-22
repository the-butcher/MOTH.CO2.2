#include "ButtonHelper.h"

const int64_t MICROSECONDS_MIN_DEBOUNCE = 250000;
const int64_t MICROSECONDS_MAX_FAST = 1000000;  // short press up to 1 second
const int64_t MICROSECONDS_MAX_SLOW = 5000000;

ButtonHelper::ButtonHelper(gpio_num_t gpin) {
    this->gpin = gpin;
    this->ipin = digitalPinToInterrupt(gpin);
    this->buttonAction = {"", "", "", nullptr, nullptr};
}

void ButtonHelper::begin() {
    pinMode(gpin, INPUT_PULLUP);
    rtc_gpio_deinit(gpin);
}

void ButtonHelper::prepareSleep(wakeup_e wakeupType) {
    if (wakeupType == WAKEUP_BUTTONS) {
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