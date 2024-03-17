#ifndef SensorScd041_h
#define SensorScd041_h

#include <Arduino.h>

#include "SensorScd041Base.h"
#include "types/Config.h"
#include "types/Values.h"

class SensorScd041 {
   private:
    SensorScd041Base baseSensor;
    values_co2_t values;

   public:
    void begin();
    bool measure();
    bool configure(config_t config);
    values_co2_t readval();
    bool powerUp();
    bool powerDown();
    static uint16_t toShortDeg(float floatValue);
    static float toFloatDeg(uint16_t shortValue);
    static uint16_t toShortHum(float floatValue);
    static float toFloatHum(uint16_t shortValue);
};

#endif