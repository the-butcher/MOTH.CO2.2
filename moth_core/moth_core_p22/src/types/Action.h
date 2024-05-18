#ifndef Action_h
#define Action_h

#include <Arduino.h>

#include "modules/ModuleSignal.h"
#include "types/Config.h"

typedef enum : uint8_t {
    WAKEUP_ACTION_BUTN,  // ext1 wakeup from button pins or RTC_SQW pin
    WAKEUP_ACTION_BUSY   // ext0 wakeup from busy pin HIGH
} wakeup_action_e;

typedef enum : uint8_t {
    BUTTON_ACTION_FAST,
    BUTTON_ACTION_SLOW,
    BUTTON_ACTION_NONE
} button_action_e;

typedef struct {
    String symbolFast;                                   // the symbol for a fast press
    String symbolSlow;                                   // the symbol for a slow press
    String extraLabel;                                   // extra information to be displayed for this button
    std::function<bool(config_t& config)> functionFast;  // a function to be executed on fast press
    std::function<bool(config_t& config)> functionSlow;  // a function to be executed on slow press
} button_action_t;

#endif