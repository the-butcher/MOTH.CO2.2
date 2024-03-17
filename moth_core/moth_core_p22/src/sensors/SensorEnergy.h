#ifndef SensorEnergy_h
#define SensorEnergy_h

#include <Adafruit_LC709203F.h>

#include "types/Measurement.h"

class SensorEnergy {
   private:
    Adafruit_LC709203F basePack;
    measurement_nrg_t values;

   public:
    void begin();
    bool measure();
    measurement_nrg_t readval();
    bool powerUp();
    bool powerDown();
    static uint16_t toShortPercent(float floatValue);
    static float toFloatPercent(uint16_t shortValue);
};

#endif