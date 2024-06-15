#include "ButtonHelper.h"

ButtonHelper::ButtonHelper(gpio_num_t gpin) {
    this->gpin = gpin;
    this->ipin = digitalPinToInterrupt(gpin);
    this->buttonAction = {"", "", nullptr, nullptr};
}

void ButtonHelper::begin() {
    pinMode(gpin, INPUT_PULLUP);
}

bool ButtonHelper::isPressed() {
    return digitalRead(gpin) == LOW;
}