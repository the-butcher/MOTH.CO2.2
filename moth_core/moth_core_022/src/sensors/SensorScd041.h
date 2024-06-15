#ifndef SensorScd041_h
#define SensorScd041_h

#include <Arduino.h>

#include "SensorScd041Base.h"
#include "types/Config.h"
#include "types/Values.h"

class SensorScd041 {
   private:
    static SensorScd041Base baseSensor;
    static values_co2_t values;

   public:
    static void begin();
    static bool configure(config_t& config);  // must have begun before configuration
    static bool setCompensationPressure(float compensationPressure);
    static co2cal______t forceCalibration(uint16_t requestedCo2Ref);
    static co2cal______t forceReset();
    static bool measure();
    static values_co2_t readval();
    static bool powerup(config_t& config);
    static bool depower(config_t& config);
    static uint16_t toShortDeg(float floatValue);
    static float toFloatDeg(uint16_t shortValue);
    static uint16_t toShortHum(float floatValue);
    static float toFloatHum(uint16_t shortValue);
    static float toFahrenheit(float celsius);
    static float getTemperatureOffset();
    static uint16_t getCompensationAltitude();
    static bool isAutomaticSelfCalibration();
    static bool powerupPeriodicMeasurement();
    static bool depowerPeriodicMeasurement();
};

#endif