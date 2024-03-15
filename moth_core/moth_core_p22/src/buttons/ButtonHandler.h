#ifndef ButtonHandler_h
#define ButtonHandler_h

#include <Arduino.h>
#include <driver/rtc_io.h>

typedef enum {
    FALL_RISE_FAST,
    FALL_RISE_SLOW,
    FALL_RISE_NONE
} fallrise_t;

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