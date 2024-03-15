#ifndef SensorBme280_h
#define SensorBme280_h

#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <Wire.h>

#include "SensorBme280Base.h"
#include "measurement/MeasurementBme.h"

class SensorBme280 {
   private:
    SensorBme280Base baseSensor;
    MeasurementBme values;

   public:
    void begin();
    bool measure();
    MeasurementBme readval();
    static uint16_t toShortPressure(float floatValue);
    static float toFloatPressure(uint16_t shortValue);
};

#endif