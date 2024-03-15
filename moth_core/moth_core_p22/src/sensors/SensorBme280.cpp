#include "SensorBme280.h"

const float ALTITUDE__EXP = 0.190284;
const float ALTITUDE_MULT = 44307.69396;

const Adafruit_BME280::sensor_mode MODE_FORCED = (Adafruit_BME280::sensor_mode)0b01;
const Adafruit_BME280::sensor_sampling SAMPLING_X1 = (Adafruit_BME280::sensor_sampling)0b001;
const Adafruit_BME280::sensor_filter FILTER_OFF = (Adafruit_BME280::sensor_filter)0b000;
const Adafruit_BME280::standby_duration STANDBY_MS_1000 = (Adafruit_BME280::standby_duration)0b101;

void SensorBme280::begin() {
    baseSensor.begin();
    // https://www.mouser.com/datasheet/2/783/BST-BME280-DS002-1509607.pdf for weather measurements
    baseSensor.setSampling(MODE_FORCED, SAMPLING_X1, SAMPLING_X1, SAMPLING_X1, FILTER_OFF, STANDBY_MS_1000);
    values = {0};
}

bool SensorBme280::measure() {
    return baseSensor.takeForcedMeasurementNoDelay();
}

MeasurementBme SensorBme280::readval() {
    return SensorBme280::values = {toShortPressure(baseSensor.readPressure() / 100.0f)};
}

uint16_t SensorBme280::toShortPressure(float floatValue) {
    return round((min(1300.0f, max(300.0f, floatValue)) - 300.0f) * 64.0f);
}

float SensorBme280::toFloatPressure(uint16_t shortValue) {
    return shortValue / 64.0f + 300.0f;
}
