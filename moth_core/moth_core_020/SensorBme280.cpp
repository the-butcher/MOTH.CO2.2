#include "SensorBme280.h"

/**
 * ################################################
 * ## mutable variables
 * ################################################
 */

/**
 * ################################################
 * ## static class variabales                     
 * ################################################
 */
Adafruit_BME280 SensorBme280::baseSensor;
ValuesBme SensorBme280::values;
float SensorBme280::temperatureOffset = 2.0; // will be autocalibrated (?)
bool SensorBme280::hasBegun = false;

void SensorBme280::begin() {
  SensorBme280::baseSensor.begin();
  SensorBme280::applyTemperatureOffset();
  SensorBme280::hasBegun = true;
}

float SensorBme280::getTemperatureOffset() {
  return SensorBme280::temperatureOffset;
}

void SensorBme280::setTemperatureOffset(float temperatureOffset) {
  bool isApplOffsetRequired = temperatureOffset != SensorBme280::temperatureOffset;
  SensorBme280::temperatureOffset = temperatureOffset;
  if (SensorBme280::hasBegun && isApplOffsetRequired) {
    SensorBme280::applyTemperatureOffset();
  }
}

bool SensorBme280::tryRead() {
  values = {
    SensorBme280::baseSensor.readTemperature(),
    SensorBme280::baseSensor.readHumidity(),
    SensorBme280::baseSensor.readPressure()
  };
  return true;
}

void SensorBme280::applyTemperatureOffset() {
  SensorBme280::baseSensor.setTemperatureCompensation(-SensorBme280::temperatureOffset);
}
