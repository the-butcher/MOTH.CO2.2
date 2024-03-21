#ifndef SensorBme280_h
#define SensorBme280_h

#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <Wire.h>

#include "SensorBme280Base.h"
#include "types/Config.h"
#include "types/Values.h"

class SensorBme280 {
   private:
    static SensorBme280Base baseSensor;
    static values_bme_t values;
    static float ALTITUDE__EXP;
    static float ALTITUDE_MULT;

   public:
    static void begin();
    static bool measure();
    static values_bme_t readval();
    static float getPressureZerolevel(float altitudeBaselevel, float pressure);
    static float getAltitude(float pressureZerolevel, float pressure);
};

#endif