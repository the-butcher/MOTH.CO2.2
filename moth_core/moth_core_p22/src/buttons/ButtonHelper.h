#ifndef ButtonHelper_h
#define ButtonHelper_h

#include <Arduino.h>
#include <driver/rtc_io.h>

class ButtonHelper {
   private:
   public:
    ButtonHelper(gpio_num_t gpin);
    void begin();
    void prepareSleep(bool isExt1Wakeup);
    gpio_num_t gpin;  // gpio pin
    uint8_t ipin;     // interrupt pin
    bool isPressed();
};

#endif