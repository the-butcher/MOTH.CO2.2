#include "SensorBme280.h"

Adafruit_BME280 SensorBme280::baseSensor;
values_bme_t SensorBme280::values = {0.0f};
bool SensorBme280::hasBegun = false;

void SensorBme280::begin() {
    SensorBme280::baseSensor.begin();
    SensorBme280::hasBegun = true;
}

bool SensorBme280::configure(config_t& config) {

    Serial.println("SensorBme280::configure");

    // whats currently set
    float temperatureOffsetV = SensorBme280::getTemperatureOffset();
    // whats about to be set
    float temperatureOffsetC = -config.sbme.temperatureOffset;

    Serial.printf("temperatureOffsetV: %u, temperatureOffsetC: %u\n", temperatureOffsetV, temperatureOffsetC);

    // apply, only if there is a real change
    bool temperatureApplied = false;
    if (abs(temperatureOffsetV - temperatureOffsetC) > 0.01) {
        Serial.printf("!!! applying temperature offset, %f, %f !!!\n", temperatureOffsetV, temperatureOffsetC);
        SensorBme280::baseSensor.setTemperatureCompensation(temperatureOffsetC);
        temperatureApplied = true;
    } else {
        Serial.printf("no temperature offset application needed, %f, %f\n", temperatureOffsetV, temperatureOffsetC);
    }

    return temperatureApplied;
}

bool SensorBme280::measure() {
    return true;
}

values_bme_t SensorBme280::readval() {
    SensorBme280::values = {SensorBme280::baseSensor.readTemperature(), SensorBme280::baseSensor.readHumidity(), SensorBme280::baseSensor.readPressure() / 100.0f};
    return SensorBme280::values;
}

float SensorBme280::getTemperatureOffset() {
    return SensorBme280::baseSensor.getTemperatureCompensation();
}
