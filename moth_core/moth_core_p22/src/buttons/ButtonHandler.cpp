#include "ButtonHandler.h"

const int64_t MICROSECONDS_MIN_DEBOUNCE = 250000;
const int64_t MICROSECONDS_MAX_FAST = 1000000;  // short press up to 1 second
const int64_t MICROSECONDS_MAX_SLOW = 5000000;

ButtonHandler::ButtonHandler(gpio_num_t gpin) {
    this->gpin = gpin;
    this->ipin = digitalPinToInterrupt(gpin);
}

void ButtonHandler::begin() {
    rtc_gpio_deinit(gpin);
    pinMode(gpin, INPUT_PULLUP);
}

void ButtonHandler::prepareSleep(bool isExt1Wakeup) {
    if (isExt1Wakeup) {
        gpio_hold_en(gpin);
        rtc_gpio_pullup_en(gpin);
        rtc_gpio_pulldown_dis(gpin);
    }
}

bool ButtonHandler::isPressed() {
    return digitalRead(gpin) == LOW;
}