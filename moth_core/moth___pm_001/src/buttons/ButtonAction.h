#ifndef ButtonActions_h
#define ButtonActions_h

#include <Arduino.h>
#include <driver/rtc_io.h>

#include "ButtonHelper.h"
#include "types/Action.h"
#include "types/Config.h"
#include "types/Device.h"

class ButtonAction {
   private:
    static bool toggleDisplayValTFw(config_t& config);
    static bool toggleDisplayValTBw(config_t& config);
    static bool toggleWifi(config_t& config);
    static bool toggleBeep(config_t& config);
    static button_action_t getButtonActionFunctionWFBP(config_t& config);
    static button_action_t getButtonActionDisplayValTT(config_t& config);
    static button_action_t getButtonActionCo2Reference(config_t& config);
    static std::function<bool(config_t& config)> getActionFunction(button_action_t buttonAction, button_action_e buttonActionType);
    static gpio_num_t actionPin;
    static std::function<void(std::function<bool(config_t& config)>)> actionCompleteCallback;
    static void handleInterruptA();
    static void handleInterruptB();
    static void handleInterruptC();
    static void detectButtonActionType(void* parameter);
    static void handleButtonActionType(button_action_e buttonActionType);

   public:
    static ButtonHelper A;
    static ButtonHelper B;
    static ButtonHelper C;
    static void begin(std::function<void(std::function<bool(config_t& config)>)> actionCompleteCallback);
    static bool adapt(config_t& config);
    static void attachWakeup();
    static void detachWakeup();
    static gpio_num_t getPressedPin();
    static gpio_num_t getActionPin();
    static void createButtonAction(gpio_num_t actionPin);
};

#endif