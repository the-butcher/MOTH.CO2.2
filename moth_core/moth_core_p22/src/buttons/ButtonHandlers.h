#ifndef ButtonHandlers_h
#define ButtonHandlers_h

#include <Arduino.h>
#include <driver/rtc_io.h>

#include "BoxBeep.h"
#include "ButtonHandler.h"
#include "types/Action.h"

class ButtonHandlers {
   private:
    uint64_t ext1Bitmask;
    // static std::function<bool(button_t)> wakeupCallback;
    // static gpio_num_t wakeupPin;
    // static void buttonReleaseVTask(void* pvParameters);

   public:
    ButtonHandlers() : A(GPIO_NUM_11), B(GPIO_NUM_12), C(GPIO_NUM_6){};
    ButtonHandler A;
    ButtonHandler B;
    ButtonHandler C;
    void begin();
    void prepareSleep(bool isExt1Wakeup);
    gpio_num_t getActionPin();
    // bool isTaskActive();
    // void handleButtonWakeup(gpio_num_t wakeupPin, std::function<bool(button_t)> wakeupCallback);
};

#endif