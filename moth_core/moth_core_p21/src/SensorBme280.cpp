#include "SensorBme280.h"

/**
 * ################################################
 * ## constants
 * ################################################
 */
const float ALTITUDE__EXP = 0.190284;
const float ALTITUDE_MULT = 44307.69396;

const Adafruit_BME280::sensor_mode MODE_FORCED = (Adafruit_BME280::sensor_mode)0b01;
const Adafruit_BME280::sensor_sampling SAMPLING_X1 = (Adafruit_BME280::sensor_sampling)0b001;
const Adafruit_BME280::sensor_filter FILTER_OFF = (Adafruit_BME280::sensor_filter)0b000;
const Adafruit_BME280::standby_duration STANDBY_MS_1000 = (Adafruit_BME280::standby_duration)0b101;

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
SensorBme280Base SensorBme280::baseSensor;
ValuesBme SensorBme280::values;
float SensorBme280::temperatureOffset = 2.0;  // will be autocalibrated (?)
float SensorBme280::altitudeOffset = 0.0;
float SensorBme280::pressureOffset = 0.0;
bool SensorBme280::hasBegun = false;

void SensorBme280::begin() {
    SensorBme280::baseSensor.begin();
    SensorBme280::baseSensor.setSampling(MODE_FORCED, SAMPLING_X1, SAMPLING_X1, SAMPLING_X1, FILTER_OFF,
                                         STANDBY_MS_1000);  // as of
                                                            // https://www.mouser.com/datasheet/2/783/BST-BME280-DS002-1509607.pdf
                                                            // for weather measurements
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
    SensorBme280::pressureOffset = 0;  // triggers a recalculation upon next measurement
}

bool SensorBme280::tryRead() {
    SensorBme280::baseSensor.takeForcedMeasurementNoDelay();
    return true;
}

ValuesBme SensorBme280::getValues() {
    float pressure = SensorBme280::baseSensor.readPressure();
    if (SensorBme280::pressureOffset == 0) {
        SensorBme280::applyAltitudeOffset(pressure);
    }
    SensorBme280::values = {SensorBme280::baseSensor.readTemperature(), SensorBme280::baseSensor.readHumidity(), pressure, SensorBme280::getAltitude(pressure)};
    return SensorBme280::values;
}

void SensorBme280::updateAltitude(float altitudeUpdate) {
    SensorBme280::setAltitudeOffset(SensorBme280::values.altitude + altitudeUpdate);
    ValuesBme valuesBme = SensorBme280::getValues();
    Measurements::putValuesBme(valuesBme);
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
