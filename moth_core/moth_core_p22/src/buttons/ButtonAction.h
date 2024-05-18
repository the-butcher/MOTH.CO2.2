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
    static uint64_t ext1Bitmask;
    static bool toggleDisplayValTFw(config_t& config);
    static bool toggleDisplayValTBw(config_t& config);
    static bool toggleDisplayValCFw(config_t& config);
    static bool toggleDisplayValCBw(config_t& config);
    static bool toggleDisplayValMod(config_t& config);
    static bool toggleDisplayValThm(config_t& config);
    static bool toggleDisplayValHBw(config_t& config);
    static bool toggleDisplayValHFw(config_t& config);
    static bool incrementAltitude10(config_t& config);
    static bool decrementAltitude10(config_t& config);
    static bool incrementAltitude50(config_t& config);
    static bool decrementAltitude50(config_t& config);
    static bool calibrateToCo2Refer(config_t& config);
    static bool toggleWifi(config_t& config);
    static bool toggleBeep(config_t& config);
    static button_action_t getButtonActionFunctionWFBP(config_t& config);
    static button_action_t getButtonActionAltitude1010(config_t& config);
    static button_action_t getButtonActionAltitude5050(config_t& config);
    static button_action_t getButtonActionDisplayValHR(config_t& config);
    static button_action_t getButtonActionDisplayValTT(config_t& config);
    static button_action_t getButtonActionDisplayValCC(config_t& config);
    static button_action_t getButtonActionDisplayValMT(config_t& config);
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
    static void prepareSleep(wakeup_action_e wakeupType);
    static void attachWakeup(wakeup_action_e wakeupType);
    static void detachWakeup(wakeup_action_e wakeupType);
    static gpio_num_t getPressedPin();
    static gpio_num_t getActionPin();
    static void createButtonAction(gpio_num_t actionPin);
};

#endif