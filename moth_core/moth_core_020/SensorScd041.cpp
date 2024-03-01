#include "SensorScd041.h"


/**
 * ################################################
 * ## static class variabales                     
 * ################################################
 */
SensirionI2CScd4x SensorScd041::baseSensor;
ValuesCo2 SensorScd041::values;
float SensorScd041::temperatureOffset = 1.2; // default offset, can be overridden in /config/disp.json -> "deg/off"
int SensorScd041::co2Reference = 425;
bool SensorScd041::hasBegun = false;

void SensorScd041::begin() {

  SensorScd041::baseSensor.begin(Wire);
  SensorScd041::baseSensor.setAutomaticSelfCalibration(0); // no automatic self calibration desired

  SensorScd041::values = { 
    -1, 
    -1, 
    -1
  }; 
      
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

int SensorScd041::getCo2Reference() {
  return SensorScd041::co2Reference;
}

void SensorScd041::setCo2Reference(int co2Reference) {
  SensorScd041::co2Reference = co2Reference;
}

bool SensorScd041::tryRead() {
  // when measuring periodically, there is nothing to be done here
  // TODO :: power usage once the nordic device is here, if single shot is much better, a solution needs to be found for co2 value noise
  return SensorScd041::baseSensor.measureSingleShotNoDelay(); // delay must be taken care of in this sketch's code
  // return true;
}

ValuesCo2 SensorScd041::getValues() {
  bool isDataReady = false;
  SensorScd041::baseSensor.getDataReadyFlag(isDataReady); 
  if (isDataReady) {
    uint16_t co2;
    float temperature;
    float humidity;
    SensorScd041::baseSensor.readMeasurement(co2, temperature, humidity);    
    SensorScd041::values = { 
      co2, 
      temperature, 
      humidity,
      co2 
    };    
  }
  return SensorScd041::values;  
}

void SensorScd041::setPressure(float pressure) {
  SensorScd041::baseSensor.setAmbientPressure(pressure);
}

bool SensorScd041::isAutomaticSelfCalibration() {
  uint16_t ascV = 0;
  uint16_t& ascR = ascV;
  SensorScd041::baseSensor.getAutomaticSelfCalibration(ascR);
  return ascV;
}

void SensorScd041::startPeriodicMeasurement() {
  // SensorScd041::baseSensor.startLowPowerPeriodicMeasurement();
}

void SensorScd041::stopPeriodicMeasurement() {
  // SensorScd041::baseSensor.stopPeriodicMeasurement();
  // delay(500);
}

uint16_t SensorScd041::forceCalibration(int reference) {
  uint16_t frcV = 0;
  uint16_t& frcR = frcV;
  SensorScd041::baseSensor.performForcedRecalibration(reference, frcR);
  delay(400);
  return frcV;
}

void SensorScd041::factoryReset() {
  SensorScd041::stopPeriodicMeasurement();
  SensorScd041::baseSensor.performFactoryReset();
  delay(400);
  SensorScd041::startPeriodicMeasurement();    
}

void SensorScd041::applyTemperatureOffset() {
  SensorScd041::stopPeriodicMeasurement();
  SensorScd041::baseSensor.setTemperatureOffset(SensorScd041::temperatureOffset);
  delay(400);
  SensorScd041::startPeriodicMeasurement();    
}