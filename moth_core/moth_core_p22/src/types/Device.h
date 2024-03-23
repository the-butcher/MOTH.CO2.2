#ifndef Device_h
#define Device_h

#include "types/Action.h"
#include "types/Config.h"
#include "types/Values.h"

typedef enum {
    SETUP_BOOT,
    SETUP_MAIN
} setup_mode_t;

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
    device_action_e type;        // type of action to be performed
    color_t color;               // neopixel color associated with this action
    wakeup_action_e wakeupType;  // wakeup type after this action
    uint32_t secondsWait;        // wait time after this action
    uint32_t secondsNext;        // time for next execution of this action
} device_action_t;

typedef struct {
    uint32_t secondsSetupBase;  // secondstime at boot time plus some buffer
    device_action_t deviceActions[4];
    uint8_t actionIndexCur;
    uint8_t actionIndexMax;
} device_t;

class Device {
   private:
    static void handleActionMeasure(values_t* values, config_t* config);
    static void handleActionReadval(values_t* values, config_t* config);
    static void handleActionDisplay(values_t* values, config_t* config);
    static void handleActionDepower(values_t* values, config_t* config);
    static void handleActionInvalid(values_t* values, config_t* config);

   public:
    static device_t load();
    static std::function<void(values_t* values, config_t* config)> getFunctionByAction(device_action_e action);
};

#endif