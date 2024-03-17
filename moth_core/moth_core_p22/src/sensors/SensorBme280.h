#ifndef SensorBme280_h
#define SensorBme280_h

#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <Wire.h>

#include "SensorBme280Base.h"
#include "types/Measurement.h"

class SensorBme280 {
   private:
    SensorBme280Base baseSensor;
    measurement_bme_t values;

   public:
    void begin();
    bool measure();
    measurement_bme_t readval();
    // static uint16_t toShortPressure(float floatValue);
    // static float toFloatPressure(uint16_t shortValue);
};

#endif