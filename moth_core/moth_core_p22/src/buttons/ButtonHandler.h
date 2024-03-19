#ifndef ButtonHandler_h
#define ButtonHandler_h

#include <Arduino.h>
#include <driver/rtc_io.h>

class ButtonHandler {
   private:
   public:
    ButtonHandler(gpio_num_t gpin);
    void begin();
    void prepareSleep(bool isExt1Wakeup);
    gpio_num_t gpin;  // gpio pin
    uint8_t ipin;     // interrupt pin
    bool isPressed();
};

#endif