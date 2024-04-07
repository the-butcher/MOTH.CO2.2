#ifndef SensorEnergy_h
#define SensorEnergy_h

#include <Adafruit_LC709203F.h>
#include <Arduino.h>

#include "types/Define.h"
#include "types/Values.h"

class SensorEnergy {
   private:
    static Adafruit_LC709203F basePack;
    static values_nrg_t values;
    static bool isReadRequired;
    static bool hasBegun;

   public:
    static void begin();  // will set the power mode to operational
    static bool measure();
    static values_nrg_t readval();
    static bool powerup();
    static bool depower();
    static uint16_t toShortPercent(float floatValue);
    static float toFloatPercent(uint16_t shortValue);
};

#endif