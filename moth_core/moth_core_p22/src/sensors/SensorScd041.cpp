#include "SensorScd041.h"

void SensorScd041::begin() {
    baseSensor.begin(Wire);
    values = {0, 0, 0};
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

MeasurementCo2 SensorScd041::readval() {
    bool isDataReady = false;
    baseSensor.getDataReadyFlag(isDataReady);
    if (isDataReady) {
        uint16_t co2;
        float temperature;
        float humidity;
        baseSensor.readMeasurement(co2, temperature, humidity);
        values = {co2, 0, 0, co2};
    }
    return values;
}
