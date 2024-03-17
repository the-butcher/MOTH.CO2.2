#ifndef SensorEnergy_h
#define SensorEnergy_h

#include <Adafruit_LC709203F.h>

#include "types/Values.h"

class SensorEnergy {
   private:
    Adafruit_LC709203F basePack;
    values_nrg_t values;

   public:
    void begin();
    bool measure();
    values_nrg_t readval();
    bool powerUp();
    bool powerDown();
    static uint16_t toShortPercent(float floatValue);
    static float toFloatPercent(uint16_t shortValue);
};

#endif