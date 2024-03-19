#ifndef ButtonActions_h
#define ButtonActions_h

#include <Arduino.h>
#include <driver/rtc_io.h>

#include "ButtonHelper.h"
#include "modules/ModuleSignal.h"
#include "types/Action.h"
#include "types/Config.h"

class ButtonAction {
   private:
    uint64_t ext1Bitmask;
    // static std::function<bool(button_t)> wakeupCallback;
    // static gpio_num_t wakeupPin;
    // static void buttonReleaseVTask(void* pvParameters);

   public:
    ButtonAction() : A(GPIO_NUM_11), B(GPIO_NUM_12), C(GPIO_NUM_6){};
    ButtonHelper A;
    ButtonHelper B;
    ButtonHelper C;
    button_action_t buttonActionA;
    button_action_t buttonActionB;
    button_action_t buttonActionC;
    void begin();
    void prepareSleep(bool isExt1Wakeup);
    gpio_num_t getPressedPin();
    bool toggleDisplayValTFw(config_t *config);
};

#endif