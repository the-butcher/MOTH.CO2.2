#ifndef Action_h
#define Action_h

#include <Arduino.h>

#include "modules/ModuleSignal.h"
#include "types/Config.h"

typedef enum : uint8_t {
    BUTTON_ACTION_FAST,
    BUTTON_ACTION_SLOW,
    BUTTON_ACTION_NONE
} button_action_e;

typedef struct {
    String symbolFast;                                   // the symbol for a fast press
    String symbolSlow;                                   // the symbol for a slow press
    std::function<bool(config_t& config)> functionFast;  // a function to be executed on fast press
    std::function<bool(config_t& config)> functionSlow;  // a function to be executed on slow press
} button_action_t;

#endif