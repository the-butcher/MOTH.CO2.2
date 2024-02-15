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
    static float altitudeOffset;
    static float pressureOffset; // calculated pressure at sea level
    static bool hasBegun;
    static ValuesBme values;
    static void applyTemperatureOffset();
    static void applyAltitudeOffset(float pressure);
    static float getAltitude(float pressure);

  public:
    static float getTemperatureOffset();
    static void setTemperatureOffset(float temperatureOffset);
    static void setAltitudeOffset(float altitudeOffset);
    static void updateAltitude(float altitudeUpdate);
    static void begin();
    static bool tryRead();
    static ValuesBme getValues();

};

#endif