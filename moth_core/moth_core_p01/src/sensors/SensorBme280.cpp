#include "SensorBme280.h"

Adafruit_BME280 SensorBme280::baseSensor;
values_bme_t SensorBme280::values = {0.0f};

void SensorBme280::begin() {
    SensorBme280::baseSensor.begin();
}

bool SensorBme280::measure() {
    return true;
}

values_bme_t SensorBme280::readval() {
    SensorBme280::values = {SensorBme280::baseSensor.readTemperature(), SensorBme280::baseSensor.readHumidity(), SensorBme280::baseSensor.readPressure() / 100.0f};
    return SensorBme280::values;
}

float SensorBme280::getPressureZerolevel(float altitudeBaselevel, float pressure) {
    return pressure / pow(1 - altitudeBaselevel / ALTITUDE_MULT, 1 / ALTITUDE__EXP);
}

float SensorBme280::getAltitude(float pressureZerolevel, float pressure) {
    return (1 - pow(pressure / pressureZerolevel, ALTITUDE__EXP)) * ALTITUDE_MULT;
}
