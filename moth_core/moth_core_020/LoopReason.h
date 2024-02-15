#ifndef LoopReason_h
#define LoopReason_h

typedef enum {

  // button A, when table+!altitude
  LOOP_REASON______WIFI______ON,
  LOOP_REASON______WIFI_____OFF,
  LOOP_REASON______TOGGLE___PMS,

  // button B, when not table+altitude
  LOOP_REASON______TOGGLE_STATE, // table, chart
  LOOP_REASON______TOGGLE_THEME, // light, dark

  // button A, when table+altitude
  LOOP_REASON___ADD_50_ALTITUDE,
  LOOP_REASON___DEL_50_ALTITUDE,

  // button A, when table+altitude
  LOOP_REASON___TOGGLE_HOURS_FW,
  LOOP_REASON___TOGGLE_HOURS_BW,

  // button B, when table+altitude
  LOOP_REASON___ADD_10_ALTITUDE,
  LOOP_REASON___DEL_10_ALTITUDE,

  // button C
  LOOP_REASON___TOGGLE_VALUE_FW, // within state, the acual value being shown (co2, pm1.0, pm2.5, pm10.0), let a long press return to co2
  LOOP_REASON___TOGGLE_VALUE_BW,

  LOOP_REASON______RENDER_STATE, // a simple re-render
  LOOP_REASON_______MEASUREMENT, // time for a measurement or for sensor wakeup
  LOOP_REASON______CALIBRRATION, // calibrate the sensor to a given reference value
  LOOP_REASON_______HIBERNATION, // hibernate the device
  LOOP_REASON_RESET_CALIBRATION, // reset SCD41 to factory
  LOOP_REASON___________UNKNOWN

} loop_reason_t;

typedef struct {
  loop_reason_t loopReason;
  String label;
} ButtonAction;

#endif