#ifndef Action_h
#define Action_h

#include <Arduino.h>

/**
 * ONLY CHANGE WITH CARE :: INDICES OF ACTIONS MUST REMAIN CONSTANT
 */
typedef enum {
    ACTION_MEASURE,  // i2c on, measure
    ACTION_READVAL,  // read values, i2c off
    ACTION_DISPLAY,  // display on, render values
    ACTION_DEPOWER   // display off
} action_t;

typedef struct {
    action_t type;         // type of action to be performed
    color_t color;         // neopixel color associated with this action
    bool isExt1Wakeup;     // allow button wakeup while waiting for the next action
    uint32_t secondsWait;  // time expected for this action to complete
    uint32_t secondsNext;  // time for next execution of this action
} action_s;

#endif