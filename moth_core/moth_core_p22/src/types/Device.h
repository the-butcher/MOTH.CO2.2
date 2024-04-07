#ifndef Device_h
#define Device_h

#include <Arduino.h>

#include "sensors/SensorScd041.h"
#include "types/Action.h"
#include "types/Config.h"
#include "types/Values.h"

typedef enum : uint8_t {
    SETUP_BOOT,
    SETUP_MAIN
} setup_mode_t;

/**
 * ONLY CHANGE WITH CARE :: INDICES OF ACTIONS MUST REMAIN CONSTANT
 * !!! when adding an action, the deviceAction array size must be adapted
 */
typedef enum : uint8_t {
    DEVICE_ACTION_POWERUP,  // i2c on,
    DEVICE_ACTION_MEASURE,  // measure
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
    device_action_t deviceActions[6];
    device_action_e actionIndexCur;
    device_action_e actionIndexMax;
    uint64_t ext1Bitmask;
} device_t;

class Device {
   private:
    static int cmpfunc(const void* a, const void* b);
    static calibration_t calibrationResult;
    static device_action_e handleActionPowerup(config_t& config, device_action_e maxDeviceAction);
    static device_action_e handleActionMeasure(config_t& config, device_action_e maxDeviceAction);
    static device_action_e handleActionReadval(config_t& config, device_action_e maxDeviceAction);
    static device_action_e handleActionSetting(config_t& config, device_action_e maxDeviceAction);
    static device_action_e handleActionDisplay(config_t& config, device_action_e maxDeviceAction);
    static device_action_e handleActionDepower(config_t& config, device_action_e maxDeviceAction);
    static device_action_e handleActionInvalid(config_t& config, device_action_e maxDeviceAction);
    static bool isEnergyCycle();

   public:
    static uint32_t secondstimeBoot;
    static device_t load();
    static void begin(uint32_t secondstimeBoot);
    static std::function<device_action_e(config_t& config, device_action_e maxDeviceAction)> getFunctionByAction(device_action_e action);
};

#endif