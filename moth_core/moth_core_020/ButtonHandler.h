#ifndef ButtonHandler_h
#define ButtonHandler_h

#include "Arduino.h"

typedef enum {
  FALL_RISE_FAST,
  FALL_RISE_SLOW,
  FALL_RISE_NONE
} fallrise_t;

class ButtonHandler {
  
  private:
    int64_t microsecondsLastFall = -1;
    int64_t microsecondsLastRise = -1;
    int curLevel;
    int refLevel;
  
  public:
    ButtonHandler(gpio_num_t gpin);
    void begin();
    gpio_num_t gpin;
    uint8_t ipin;
    gpio_int_type_t getWakeupLevel();
    fallrise_t getFallRise();

};

#endif