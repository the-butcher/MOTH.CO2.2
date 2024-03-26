#include "SensorScd041.h"

#include "types/Define.h"

SensorScd041Base SensorScd041::baseSensor;
values_co2_t SensorScd041::values = {0, 0, 0};

void SensorScd041::begin() {
    SensorScd041::baseSensor.begin(Wire);
}

bool SensorScd041::configure(config_t* config) {
    // check current setting first
    float temperatureOffsetV = 0;
    float& temperatureOffsetR = temperatureOffsetV;
    SensorScd041::baseSensor.getTemperatureOffset(temperatureOffsetR);
    // whats about to be set
    float temperatureOffsetC = config->sco2.temperatureOffset;
    // apply, only if there is a real change
    if (abs(temperatureOffsetV - temperatureOffsetC) > 0.01) {
        SensorScd041::baseSensor.setTemperatureOffset(temperatureOffsetC);
        SensorScd041::baseSensor.setAutomaticSelfCalibration(0);
        SensorScd041::baseSensor.persistSettings();
        return true;
    } else {
        return false;
    }
}

uint16_t SensorScd041::forceCalibration(uint16_t calibrationReference) {
#ifdef USE___SERIAL
    Serial.print("calibrationReference: ");
    Serial.println(calibrationReference);
#endif
    // TODO :: return a struct instance, so the display can render appropriate information
    return 0;
    // uint16_t frcV = 0;
    // uint16_t& frcR = frcV;
    // SensorScd041::baseSensor.performForcedRecalibration(reference, frcR);
    // delay(400);
    // return frcV;
}

bool SensorScd041::measure() {
    return SensorScd041::baseSensor.measureSingleShotNoDelay();
}

bool SensorScd041::powerup() {
    return SensorScd041::baseSensor.wakeUp();
}

bool SensorScd041::depower() {
    return SensorScd041::baseSensor.powerDown();
}

values_co2_t SensorScd041::readval() {
    bool isDataReady = false;
    SensorScd041::baseSensor.getDataReadyFlag(isDataReady);
    if (isDataReady) {
        uint16_t co2;
        float deg;
        float hum;
        SensorScd041::baseSensor.readMeasurement(co2, deg, hum);
        SensorScd041::values = {co2, SensorScd041::toShortDeg(deg), SensorScd041::toShortHum(hum), co2};
    }
    return SensorScd041::values;
}

float SensorScd041::getTemperatureOffset() {
    float degV = 0;
    float& degR = degV;
    SensorScd041::baseSensor.getTemperatureOffset(degR);
    return degV;
}

bool SensorScd041::isAutomaticSelfCalibration() {
    uint16_t ascV = 0;
    uint16_t& ascR = ascV;
    SensorScd041::baseSensor.getAutomaticSelfCalibration(ascR);
    return ascV;
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