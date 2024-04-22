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
    static void toggleDisplayValTFw(config_t& config);
    static void toggleDisplayValTBw(config_t& config);
    static void toggleDisplayValCFw(config_t& config);
    static void toggleDisplayValCBw(config_t& config);
    static void toggleDisplayValMod(config_t& config);
    static void toggleDisplayValThm(config_t& config);
    static void toggleDisplayValHBw(config_t& config);
    static void toggleDisplayValHFw(config_t& config);
    static void incrementAltitude10(config_t& config);
    static void decrementAltitude10(config_t& config);
    static void incrementAltitude50(config_t& config);
    static void decrementAltitude50(config_t& config);
    static void calibrateToCo2Refer(config_t& config);
    static void toggleWifi(config_t& config);
    static void toggleBeep(config_t& config);
    static button_action_t getButtonActionFunctionWFBP(config_t& config);
    static button_action_t getButtonActionAltitude1010(config_t& config);
    static button_action_t getButtonActionAltitude5050(config_t& config);
    static button_action_t getButtonActionDisplayValHR(config_t& config);
    static button_action_t getButtonActionDisplayValTT(config_t& config);
    static button_action_t getButtonActionDisplayValCC(config_t& config);
    static button_action_t getButtonActionDisplayValMT(config_t& config);
    static button_action_t getButtonActionCo2Reference(config_t& config);
    static std::function<void(config_t& config)> getActionFunction(button_action_t buttonAction, button_action_e buttonActionType);
    static gpio_num_t actionPin;
    static std::function<void(std::function<void(config_t& config)>)> buttonActionCompleteCallback;
    static void handleInterruptA();
    static void handleInterruptB();
    static void handleInterruptC();
    static void detectButtonActionType(void* parameter);
    static void handleButtonActionType(button_action_e buttonActionType);

   public:
    static ButtonHelper A;
    static ButtonHelper B;
    static ButtonHelper C;
    static void begin(std::function<void(std::function<void(config_t& config)>)> buttonActionCompleteCallback);
    static bool adapt(config_t& config);
    static void prepareSleep(wakeup_action_e wakeupType);
    static void attachWakeup(wakeup_action_e wakeupType);
    static void detachWakeup(wakeup_action_e wakeupType);
    static gpio_num_t getPressedPin();
    static gpio_num_t getActionPin();
    static void createButtonAction(gpio_num_t actionPin);
};

#endif