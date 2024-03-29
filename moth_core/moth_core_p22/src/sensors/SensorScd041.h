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
    static bool configure(config_t* config);  // must have begun before configuration
    static uint16_t forceCalibration(uint16_t calibrationReference);
    static bool measure();
    static values_co2_t readval();
    static bool powerup();
    static bool depower();
    static uint16_t toShortDeg(float floatValue);
    static float toFloatDeg(uint16_t shortValue);
    static uint16_t toShortHum(float floatValue);
    static float toFloatHum(uint16_t shortValue);
    static float getTemperatureOffset();
    static bool isAutomaticSelfCalibration();
};

#endif