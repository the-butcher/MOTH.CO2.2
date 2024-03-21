#include "SensorEnergy.h"

Adafruit_LC709203F SensorEnergy::basePack;
values_nrg_t SensorEnergy::values = {0};

void SensorEnergy::begin() {
    SensorEnergy::basePack.begin();
    SensorEnergy::basePack.setPackSize(LC709203F_APA_3000MAH);
    SensorEnergy::basePack.setAlarmVoltage(3.8);
}

bool SensorEnergy::powerUp() {
    return SensorEnergy::basePack.setPowerMode(LC709203F_POWER_OPERATE);
}

bool SensorEnergy::powerDown() {
    return SensorEnergy::basePack.setPowerMode(LC709203F_POWER_SLEEP);
}

bool SensorEnergy::measure() {
    SensorEnergy::values = {toShortPercent(SensorEnergy::basePack.cellPercent())};
    return true;
}

values_nrg_t SensorEnergy::readval() {
    return SensorEnergy::values;
}

uint16_t SensorEnergy::toShortPercent(float floatValue) {
    return round(min(100.0f, max(0.0f, floatValue)) * 640.0f);
}

float SensorEnergy::toFloatPercent(uint16_t shortValue) {
    return shortValue / 640.0f;
}