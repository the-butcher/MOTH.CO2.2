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
    DEVICE_ACTION_SETTING,  // any settings to be applied, i.e. after button press, or wifi request
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
    uint32_t secondstimeBoot;  // secondstime at boot time plus some buffer
    device_action_t deviceActions[5];
    uint8_t actionIndexCur;
    uint8_t actionIndexMax;
} device_t;

const uint8_t LOWPASS_BUFFER_SIZE = 16;  // arbitrary size
const float LOWPASS_ALPHA = 0.33;        // higher: faster reaction

class Device {
   private:
    static void handleActionMeasure(config_t* config);
    static void handleActionReadval(config_t* config);
    static void handleActionSetting(config_t* config);
    static void handleActionDisplay(config_t* config);
    static void handleActionDepower(config_t* config);
    static void handleActionInvalid(config_t* config);

   public:
    static device_t load();
    static std::function<void(config_t* config)> getFunctionByAction(device_action_e action);
};

#endif