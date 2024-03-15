#ifndef SensorEnergy_h
#define SensorEnergy_h

#include <Adafruit_LC709203F.h>

#include "measurement/MeasurementNrg.h"

class SensorEnergy {
   private:
    Adafruit_LC709203F basePack;
    MeasurementNrg values;

   public:
    void begin();
    MeasurementNrg readval();
    static uint16_t toShortPercent(float floatValue);
    static float toFloatPercent(uint16_t shortValue);
    bool powerUp();
    bool powerDown();
};

#endif