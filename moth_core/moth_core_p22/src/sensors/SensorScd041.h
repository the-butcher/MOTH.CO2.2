#ifndef SensorScd041_h
#define SensorScd041_h

#include <Arduino.h>

#include "SensorScd041Base.h"
#include "types/Config.h"
#include "types/Values.h"

typedef enum : uint8_t {
    ACTION___CALIBRATION,
    ACTION_FACTORY_RESET
} calibration_e;

typedef struct {
    bool success;
    calibration_e action;
    uint16_t requestedCo2Ref;   // calibrationReference requested through wifi
    uint16_t correctedCo2Ref;   // calibrationReference adapted with recent measurements
    int16_t calibrationResult;  // the offset effectively applied through calibration
} calibration_t;

class SensorScd041 {
   private:
    static SensorScd041Base baseSensor;
    static values_co2_t values;

   public:
    static void begin();
    static bool configure(config_t& config);  // must have begun before configuration
    static bool setCompensationAltitude(uint16_t compensationAltitude);
    static calibration_t forceCalibration(uint16_t requestedCo2Ref);
    static calibration_t forceReset();
    static bool measure();
    static values_co2_t readval();
    static bool powerup(config_t& config);
    static bool depower(config_t& config);
    static uint16_t toShortDeg(float floatValue);
    static float toFloatDeg(uint16_t shortValue);
    static uint16_t toShortHum(float floatValue);
    static float toFloatHum(uint16_t shortValue);
    static float toFahrenheit(float celsius);
    // static float getTemperatureOffset();
    // static uint16_t getCompensationAltitude();
    // static bool isAutomaticSelfCalibration();
};

#endif