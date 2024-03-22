#ifndef ButtonActions_h
#define ButtonActions_h

#include <Arduino.h>
#include <driver/rtc_io.h>

#include "ButtonHelper.h"
#include "modules/ModuleSignal.h"
#include "modules/ModuleTicker.h"
#include "sensors/SensorBme280.h"
#include "types/Action.h"
#include "types/Config.h"

class ButtonAction {
   private:
    static uint64_t ext1Bitmask;
    static bool toggleDisplayValTFw(config_t* config);
    static bool toggleDisplayValTBw(config_t* config);
    static bool toggleDisplayValCFw(config_t* config);
    static bool toggleDisplayValCBw(config_t* config);
    static bool toggleDisplayValMod(config_t* config);
    static bool toggleDisplayValThm(config_t* config);
    static bool toggleDisplayValHBw(config_t* config);
    static bool toggleDisplayValHFw(config_t* config);
    static bool incrementAltitude10(config_t* config);
    static bool decrementAltitude10(config_t* config);
    static bool incrementAltitude50(config_t* config);
    static bool decrementAltitude50(config_t* config);
    static bool toggleWifi(config_t* config);
    static bool toggleBeep(config_t* config);
    static button_action_t getButtonActionFunctionWFBP(config_t* config);
    static button_action_t getButtonActionAltitude1010(config_t* config);
    static button_action_t getButtonActionAltitude5050(config_t* config);
    static button_action_t getButtonActionDisplayValHR(config_t* config);
    static button_action_t getButtonActionDisplayValTT(config_t* config);
    static button_action_t getButtonActionDisplayValCC(config_t* config);
    static button_action_t getButtonActionDisplayValMT(config_t* config);
    static std::function<bool(config_t* config)> getActionFunction(button_action_t buttonAction, button_action_e buttonActionType);
    static gpio_num_t actionPin;
    static std::function<void(std::function<bool(config_t* config)>)> buttonActionCompleteCallback;
    static void handleInterruptA();
    static void handleInterruptB();
    static void handleInterruptC();
    static void detectButtonActionType(void* parameter);
    static void handleButtonActionType(button_action_e buttonActionType);

   public:
    static ButtonHelper A;
    static ButtonHelper B;
    static ButtonHelper C;
    static void begin(std::function<void(std::function<bool(config_t* config)>)> buttonActionCompleteCallback);
    static bool configure(config_t* config);
    static void prepareSleep(wakeup_e wakeupType);
    static void attachWakeup(wakeup_e wakeupType);
    static void detachWakeup(wakeup_e wakeupType);
    static gpio_num_t getPressedPin();
    static gpio_num_t getActionPin();
    // static bool accepts(gpio_num_t actionPin);
    static void createButtonAction(gpio_num_t actionPin);
};

#endif