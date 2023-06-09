#include "SensorScd041.h"

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
SensirionI2CScd4x SensorScd041::baseSensor;
ValuesCo2 SensorScd041::values;
float SensorScd041::temperatureOffset = 1.2; // default offset, can be overridden in /config/disp.json -> "deg/off"
bool SensorScd041::hasBegun = false;

void SensorScd041::begin() {
  SensorScd041::baseSensor.begin(Wire);
  SensorScd041::applyTemperatureOffset(); // this also starts measurements
  SensorScd041::hasBegun = true;
}

float SensorScd041::getTemperatureOffset() {
  return SensorScd041::temperatureOffset;
}

void SensorScd041::setTemperatureOffset(float temperatureOffset) {
  bool isApplOffsetRequired = temperatureOffset != SensorScd041::temperatureOffset;
  SensorScd041::temperatureOffset = temperatureOffset;
  if (SensorScd041::hasBegun && isApplOffsetRequired) {
    SensorScd041::applyTemperatureOffset();
  }
}

bool SensorScd041::tryRead() {
  bool isDataReady = false;
  SensorScd041::baseSensor.getDataReadyFlag(isDataReady); 
  if (isDataReady) {
    uint16_t co2;
    float temperature;
    float humidity;
    SensorScd041::baseSensor.readMeasurement(co2, temperature, humidity);    
    values = { co2, temperature, humidity };    
    return true;
  } else {
    // once in a while there are invalid readings and an empty valueset would screw up display and fan (if present)
  }
  return false;  
}

void SensorScd041::setPressure(float pressure) {
  SensorScd041::baseSensor.setAmbientPressure(pressure);
}

void SensorScd041::startPeriodicMeasurement() {
  SensorScd041::baseSensor.startLowPowerPeriodicMeasurement();
}

void SensorScd041::stopPeriodicMeasurement() {
  SensorScd041::baseSensor.stopPeriodicMeasurement();
}

uint16_t SensorScd041::forceCalibration(int reference) {
  uint16_t frcV = 0;
  uint16_t& frcR = frcV;
  SensorScd041::baseSensor.performForcedRecalibration(reference, frcR);
  return frcV;
}

void SensorScd041::factoryReset() {
  SensorScd041::baseSensor.stopPeriodicMeasurement();
  delay(500);
  SensorScd041::baseSensor.performFactoryReset();
  delay(400);
  SensorScd041::baseSensor.startLowPowerPeriodicMeasurement();    
}

void SensorScd041::applyTemperatureOffset() {
  SensorScd041::baseSensor.stopPeriodicMeasurement();
  delay(500);
  SensorScd041::baseSensor.setTemperatureOffset(SensorScd041::temperatureOffset);
  delay(400);
  SensorScd041::baseSensor.startLowPowerPeriodicMeasurement();    
}