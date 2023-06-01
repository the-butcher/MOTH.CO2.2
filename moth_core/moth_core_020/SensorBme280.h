#ifndef SensorBme280_h
#define SensorBme280_h

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "ValuesBme.h"

class SensorBme280 {
  
  private:
    static Adafruit_BME280 baseSensor;
    static float temperatureOffset;
    static bool hasBegun;
    static void applyTemperatureOffset();

  public:
    static float getTemperatureOffset();
    static void setTemperatureOffset(float temperatureOffset);
    static void begin();
    static bool tryRead();
    static ValuesBme values;

};

#endif