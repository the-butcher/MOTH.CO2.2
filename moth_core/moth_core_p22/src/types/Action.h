#ifndef Action_h
#define Action_h

#include <Arduino.h>

#include "modules/ModuleSignal.h"
#include "types/Config.h"

typedef enum {
    WAKEUP_ACTION_BUTN,
    WAKEUP_ACTION_BUSY
} wakeup_action_e;

typedef enum {
    BUTTON_ACTION_FAST,
    BUTTON_ACTION_SLOW,
    BUTTON_ACTION_NONE
} button_action_e;

typedef struct {
    String symbolFast;                                   // the symbol for a fast press
    String symbolSlow;                                   // the symbol for a slow press
    String extraLabel;                                   // extra information to be displayed for this button
    std::function<void(config_t *config)> functionFast;  // a function to be executed on fast press
    std::function<void(config_t *config)> functionSlow;  // a function to be executed on slow press
} button_action_t;

const uint32_t WAITTIME________________NONE = 0;
// const uint32_t WAITTIME_____________FOREVER = SECONDS_PER_____________HOUR;
const uint32_t WAITTIME_DISPLAY_AND_DEPOWER = 5;  // very conservative estimation, 3 or maybe even 2 could also work

#endif