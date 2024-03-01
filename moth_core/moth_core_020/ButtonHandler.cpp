#include "esp32-hal-gpio.h"
#include "pins_arduino.h"
#include "ButtonHandler.h"

/**
 * ################################################
 * ## constants
 * ################################################
 */
const int64_t MICROSECONDS_MIN_DEBOUNCE = 250000;
const int64_t MICROSECONDS_MAX_FAST = 1000000;
const int64_t MICROSECONDS_MAX_SLOW = 5000000;

ButtonHandler::ButtonHandler(gpio_num_t gpin) {
  this->gpin = gpin;
  this->ipin = digitalPinToInterrupt(gpin);
  this->microsecondsLastFall = -1;
  this->microsecondsLastRise = -1;
  this->curLevel = HIGH;
  this->refLevel = HIGH;
  this->buttonActionFast = {
    LOOP_REASON___________UNKNOWN,
    ""
  };
  this->buttonActionSlow = {
    LOOP_REASON___________UNKNOWN,
    ""
  };
  this->extraLabel = "";
}

void ButtonHandler::begin() {
  pinMode(gpin, INPUT_PULLUP); 
}

gpio_int_type_t ButtonHandler::getWakeupLevel() {
  return digitalRead(gpin) == HIGH ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL;
}

fallrise_t ButtonHandler::getFallRise() {

  curLevel = digitalRead(gpin);
  fallrise_t fallRise = FALL_RISE_NONE;

  if (curLevel != refLevel) { // an actual change
    if (curLevel == LOW) { // changed to being pressed
      if (esp_timer_get_time() - microsecondsLastFall > MICROSECONDS_MIN_DEBOUNCE) {
        microsecondsLastFall = esp_timer_get_time();
      }
    } else { // changed to being released
      if (esp_timer_get_time() - microsecondsLastRise > MICROSECONDS_MIN_DEBOUNCE) {
        microsecondsLastRise = esp_timer_get_time();
        if (microsecondsLastRise - microsecondsLastFall <= MICROSECONDS_MAX_FAST) {
          fallRise = FALL_RISE_FAST;
        }
      }
    }
  } else if (curLevel == LOW && esp_timer_get_time() - microsecondsLastFall > MICROSECONDS_MAX_FAST) {
    fallRise = FALL_RISE_SLOW;
  }

  refLevel = curLevel;
  return fallRise;

}
  


