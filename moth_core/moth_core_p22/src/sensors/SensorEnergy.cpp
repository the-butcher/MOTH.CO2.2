#include "SensorEnergy.h"

void SensorEnergy::begin() {
    basePack.begin();
    basePack.setPackSize(LC709203F_APA_3000MAH);
    basePack.setAlarmVoltage(3.8);
}

bool SensorEnergy::powerUp() {
    return basePack.setPowerMode(LC709203F_POWER_OPERATE);
}

bool SensorEnergy::powerDown() {
    return basePack.setPowerMode(LC709203F_POWER_SLEEP);
}

MeasurementNrg SensorEnergy::readval() {
    values = {toShortPercent(basePack.cellPercent())};
    return values;
}

uint16_t SensorEnergy::toShortPercent(float floatValue) {
    return round(min(1300.0f, max(300.0f, floatValue)) * 640.0f);
}

float SensorEnergy::toFloatPercent(uint16_t shortValue) {
    return shortValue / 640.0f;
}