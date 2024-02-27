#ifndef SensorScd041_h
#define SensorScd041_h

#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include "ValuesCo2.h"

class SensorScd041 {
  
  private:
    static SensirionI2CScd4x baseSensor;
    static float temperatureOffset;
    static int co2Reference;
    static bool hasBegun;
    static ValuesCo2 values;
    static void applyTemperatureOffset();
   
  public:
    static float getTemperatureOffset();
    static void setTemperatureOffset(float temperatureOffset);
    static int getCo2Reference();
    static void setCo2Reference(int co2Reference);
    static void begin();
    static void startPeriodicMeasurement();
    static void stopPeriodicMeasurement();
    static bool tryRead();
    static void setPressure(float pressure);
    static uint16_t forceCalibration(int reference);
    static void factoryReset();
    static ValuesCo2 getValues();

};

#endif