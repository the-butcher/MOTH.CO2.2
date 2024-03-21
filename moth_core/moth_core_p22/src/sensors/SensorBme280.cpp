#include "SensorBme280.h"

float SensorBme280::ALTITUDE__EXP = 0.190284;
float SensorBme280::ALTITUDE_MULT = 44307.69396;

const Adafruit_BME280::sensor_mode MODE_FORCED = (Adafruit_BME280::sensor_mode)0b01;
const Adafruit_BME280::sensor_sampling SAMPLING_X1 = (Adafruit_BME280::sensor_sampling)0b001;
const Adafruit_BME280::sensor_filter FILTER_OFF = (Adafruit_BME280::sensor_filter)0b000;
const Adafruit_BME280::standby_duration STANDBY_MS_1000 = (Adafruit_BME280::standby_duration)0b101;

SensorBme280Base SensorBme280::baseSensor;
values_bme_t SensorBme280::values = {0.0f};

void SensorBme280::begin() {
    SensorBme280::baseSensor.begin();
    // https://www.mouser.com/datasheet/2/783/BST-BME280-DS002-1509607.pdf for weather measurements
    SensorBme280::baseSensor.setSampling(MODE_FORCED, SAMPLING_X1, SAMPLING_X1, SAMPLING_X1, FILTER_OFF, STANDBY_MS_1000);
}

bool SensorBme280::measure() {
    return SensorBme280::baseSensor.takeForcedMeasurementNoDelay();
}

values_bme_t SensorBme280::readval() {
    return SensorBme280::values = {SensorBme280::baseSensor.readPressure() / 100.0f};
}

float SensorBme280::getPressureZerolevel(float altitudeBaselevel, float pressure) {
    return pressure / pow(1 - altitudeBaselevel / SensorBme280::ALTITUDE_MULT, 1 / SensorBme280::ALTITUDE__EXP);
}

float SensorBme280::getAltitude(float pressureZerolevel, float pressure) {
    return (1 - pow(pressure / pressureZerolevel, SensorBme280::ALTITUDE__EXP)) * SensorBme280::ALTITUDE_MULT;
}
