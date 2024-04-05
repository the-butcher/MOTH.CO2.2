#include "SensorEnergy.h"

Adafruit_LC709203F SensorEnergy::basePack;
values_nrg_t SensorEnergy::values = {0};
bool SensorEnergy::hasBegun = false;
bool SensorEnergy::isReadRequired = true;

void SensorEnergy::begin() {
    if (!SensorEnergy::hasBegun) {
        SensorEnergy::basePack.begin();
        SensorEnergy::basePack.setPackSize(LC709203F_APA_3000MAH);
        SensorEnergy::basePack.setAlarmVoltage(3.8);
        SensorEnergy::hasBegun = true;
    }
}

bool SensorEnergy::powerup() {
    return SensorEnergy::basePack.setPowerMode(LC709203F_POWER_OPERATE);
}

bool SensorEnergy::depower() {
    return SensorEnergy::basePack.setPowerMode(LC709203F_POWER_SLEEP);
}

bool SensorEnergy::measure() {
    isReadRequired = true;
    return true;
}

values_nrg_t SensorEnergy::readval() {
    if (SensorEnergy::isReadRequired) {
        SensorEnergy::values = {toShortPercent(SensorEnergy::basePack.cellPercent())};
        SensorEnergy::isReadRequired = false;
    }
    return SensorEnergy::values;
}

uint16_t SensorEnergy::toShortPercent(float floatValue) {
    return round(min(100.0f, max(0.0f, floatValue)) * 640.0f);
}

float SensorEnergy::toFloatPercent(uint16_t shortValue) {
    return shortValue / 640.0f;
}