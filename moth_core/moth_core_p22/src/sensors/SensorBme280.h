#ifndef SensorBme280_h
#define SensorBme280_h

#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <Wire.h>

#include "SensorBme280Base.h"
#include "types/Values.h"

class SensorBme280 {
   private:
    SensorBme280Base baseSensor;
    values_bme_t values;

   public:
    void begin();
    bool measure();
    values_bme_t readval();
};

#endif