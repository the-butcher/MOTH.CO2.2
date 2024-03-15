#ifndef SensorScd041_h
#define SensorScd041_h

#include <Arduino.h>

#include "SensorScd041Base.h"
#include "measurement/MeasurementCo2.h"

class SensorScd041 {
   private:
    SensorScd041Base baseSensor;
    MeasurementCo2 values;

   public:
    void begin();
    bool measure();
    MeasurementCo2 readval();
    bool powerUp();
    bool powerDown();
};

#endif