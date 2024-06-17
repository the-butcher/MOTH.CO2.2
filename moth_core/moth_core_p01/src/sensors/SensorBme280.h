#ifndef SensorBme280_h
#define SensorBme280_h

#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "types/Config.h"
#include "types/Values.h"

class SensorBme280 {
   private:
    static Adafruit_BME280 baseSensor;
    static values_bme_t values;
    static bool hasBegun;

   public:
    static void begin();
    static bool configure(config_t& config);  // must have begun before configuration
    static bool measure();
    static values_bme_t readval();
    static float getTemperatureOffset();
};

#endif