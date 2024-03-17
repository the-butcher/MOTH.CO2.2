#include "SensorScd041.h"

void SensorScd041::begin() {
    baseSensor.begin(Wire);
    values = {0, 0, 0};
}

bool SensorScd041::configure(config_t config) {
    // check current setting first
    float temperatureOffsetV = 0;
    float& temperatureOffsetR = temperatureOffsetV;
    baseSensor.getTemperatureOffset(temperatureOffsetR);
    // whats about to be set
    float temperatureOffsetC = config.temperatureOffset;
    // apply, only if there is a real change
    if (abs(temperatureOffsetV - temperatureOffsetC) > 0.01) {
        baseSensor.setTemperatureOffset(temperatureOffsetC);
        baseSensor.setAutomaticSelfCalibration(0);
        baseSensor.persistSettings();
        return true;
    } else {
        return false;
    }
}

bool SensorScd041::measure() {
    return baseSensor.measureSingleShotNoDelay();
}

bool SensorScd041::powerUp() {
    return baseSensor.wakeUp();
}

bool SensorScd041::powerDown() {
    return baseSensor.powerDown();
}

values_co2_t SensorScd041::readval() {
    bool isDataReady = false;
    baseSensor.getDataReadyFlag(isDataReady);
    if (isDataReady) {
        uint16_t co2;
        float deg;
        float hum;
        baseSensor.readMeasurement(co2, deg, hum);
        values = {co2, SensorScd041::toShortDeg(deg), SensorScd041::toShortHum(hum), co2};
    }
    return values;
}

uint16_t SensorScd041::toShortDeg(float floatValue) {
    return round((min(50.0f, max(-50.0f, floatValue)) + 50.0f) * 640.0f);
}

float SensorScd041::toFloatDeg(uint16_t shortValue) {
    return shortValue / 640.0f - 50.0f;
}

uint16_t SensorScd041::toShortHum(float floatValue) {
    return round(min(100.0f, max(0.0f, floatValue)) * 640.0f);
}

float SensorScd041::toFloatHum(uint16_t shortValue) {
    return shortValue / 640.0f;
}