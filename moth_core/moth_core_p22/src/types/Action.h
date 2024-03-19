#ifndef Action_h
#define Action_h

#include <Arduino.h>

#include "types/Config.h"

/**
 * ONLY CHANGE WITH CARE :: INDICES OF ACTIONS MUST REMAIN CONSTANT
 */
typedef enum {
    DEVICE_ACTION_MEASURE,  // i2c on, measure
    DEVICE_ACTION_READVAL,  // read values
    DEVICE_ACTION_DISPLAY,  // display on, render values
    DEVICE_ACTION_DEPOWER   // display off, energy sensor off, ...
} device_action_e;

typedef struct {
    device_action_e type;  // type of action to be performed
    color_t color;         // neopixel color associated with this action
    bool isExt1Wakeup;     // allow button wakeup while waiting for the next action
    uint32_t secondsWait;  // time expected for this action to complete
    uint32_t secondsNext;  // time for next execution of this action
} device_action_t;

typedef enum {
    BUTTON_ACTION_FAST,
    BUTTON_ACTION_SLOW,
    BUTTON_ACTION_NONE
} button_action_e;

typedef struct {
    String symbolFast;                                   // the symbol for a fast press
    String symbolSlow;                                   // the symbol for a slow press
    String extraLabel;                                   // extra information to be displayed for this button
    std::function<bool(config_t *config)> functionFast;  // a function to be executed on fast press
    std::function<bool(config_t *config)> functionSlow;  // a function to be executed on slow press
} button_action_t;

#endif