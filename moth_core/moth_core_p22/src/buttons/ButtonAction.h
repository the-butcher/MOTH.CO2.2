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
    static uint64_t ext1Bitmask;
    static bool toggleDisplayValTFw(config_t* config);
    static bool toggleDisplayValTBw(config_t* config);
    static std::function<bool(config_t* config)> getActionFunction(button_action_t buttonAction, button_action_e buttonActionType);
    static button_action_t buttonActionA;
    static button_action_t buttonActionB;
    static button_action_t buttonActionC;
    static gpio_num_t actionPin;
    static std::function<void(std::function<bool(config_t* config)>)> buttonActionCompleteCallback;
    static void handleInterruptA();
    static void handleInterruptB();
    static void handleInterruptC();
    static void detectButtonAction(void* parameter);
    static void handleButtonAction(button_action_e buttonActionType);

   public:
    static ButtonHelper A;
    static ButtonHelper B;
    static ButtonHelper C;
    static void begin(std::function<void(std::function<bool(config_t* config)>)> buttonActionCompleteCallback);
    static bool configure(config_t* config);
    static void prepareSleep(bool isExt1Wakeup);
    static gpio_num_t getPressedPin();
    static gpio_num_t getActionPin();
    static void attachInterrupts();
    static void detachInterrupts();
    static void createButtonAction(gpio_num_t actionPin);
};

#endif