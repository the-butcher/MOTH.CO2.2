#ifndef SensorBme280_h
#define SensorBme280_h

#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "types/Config.h"
#include "types/Values.h"

const float ALTITUDE__EXP = 0.190284;
const float ALTITUDE_MULT = 44307.69396;
const float PRESSURE_ZERO = 1013.25;

class SensorBme280 {
   private:
    static Adafruit_BME280 baseSensor;
    static values_bme_t values;

   public:
    static void begin();
    static bool measure();
    static values_bme_t readval();
    static float getPressureZerolevel(float altitudeBaselevel, float pressure);
    static float getAltitude(float pressureZerolevel, float pressure);
};

#endif