#include "SensorBme280.h"

/**
 * ################################################
 * ## constants
 * ################################################
 */
const float ALTITUDE__EXP = 0.190284;
const float ALTITUDE_MULT = 44307.69396;

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
float SensorBme280::altitudeOffset = 0.0;
float SensorBme280::pressureOffset = 0.0;
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

void SensorBme280::setAltitudeOffset(float altitudeOffset) {
  SensorBme280::altitudeOffset = altitudeOffset;
  SensorBme280::pressureOffset = 0; // triggers a recalculation upon next measurement
}

void SensorBme280::resetPressureOffset() {
  SensorBme280::pressureOffset = 0; // triggers a recalculation upon next measurement
}

bool SensorBme280::tryRead() {
  float pressure = SensorBme280::baseSensor.readPressure();
  if (SensorBme280::pressureOffset == 0) {
    SensorBme280::applyAltitudeOffset(pressure);
  }
  SensorBme280::values = {
    SensorBme280::baseSensor.readTemperature(),
    SensorBme280::baseSensor.readHumidity(),
    pressure,
    SensorBme280::getAltitude(pressure)
  };
  return true;
}

void SensorBme280::applyTemperatureOffset() {
  SensorBme280::baseSensor.setTemperatureCompensation(-SensorBme280::temperatureOffset);
}

float SensorBme280::getAltitude(float pressure) {
  return (1 - pow(pressure * 0.01 / SensorBme280::pressureOffset, ALTITUDE__EXP)) * ALTITUDE_MULT;
}

void SensorBme280::applyAltitudeOffset(float pressure) {
  SensorBme280::pressureOffset = pressure * 0.01 / pow(1 - SensorBme280::altitudeOffset / ALTITUDE_MULT, 1 / ALTITUDE__EXP);
}
  

