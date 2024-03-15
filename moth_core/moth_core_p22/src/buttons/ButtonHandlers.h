#ifndef ButtonHandlers_h
#define ButtonHandlers_h

#include <Arduino.h>
#include <driver/rtc_io.h>

#include "ButtonHandler.h"

class ButtonHandlers {
   private:
    uint64_t ext1Bitmask;

   public:
    ButtonHandlers() : A(GPIO_NUM_11), B(GPIO_NUM_12), C(GPIO_NUM_6){};
    ButtonHandler A;
    ButtonHandler B;
    ButtonHandler C;
    void begin();
    void prepareSleep(bool isExt1Wakeup);
    bool isAnyPressed();
    bool handleWakeupPin(gpio_num_t pin);
};

#endif