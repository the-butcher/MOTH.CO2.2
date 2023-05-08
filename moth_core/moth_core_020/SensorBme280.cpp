#include "SensorBme280.h"

/**
 * ################################################
 * ## static class variabales                     
 * ################################################
 */
Adafruit_BME280 SensorBme280::baseSensor;
ValuesBme SensorBme280::values;

void SensorBme280::begin() {
  baseSensor.begin();
}

bool SensorBme280::tryRead() {
  values = {
    SensorBme280::baseSensor.readTemperature(),
    SensorBme280::baseSensor.readHumidity(),
    SensorBme280::baseSensor.readPressure()
  };
  return true;
}
