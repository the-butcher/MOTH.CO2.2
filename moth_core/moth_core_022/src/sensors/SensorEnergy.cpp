#include "SensorEnergy.h"

#include "modules/ModuleSignal.h"

Adafruit_LC709203F SensorEnergy::basePack;
bool SensorEnergy::hasBegun = false;  // only valid for a single awake cycle

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
    return true;
}

values_nrg_t SensorEnergy::readval() {
    if (Values::isEnergyCycle()) {
        return {SensorEnergy::toShortPercent(SensorEnergy::basePack.cellPercent())};
    } else {
        return Values::latest().valuesNrg;
    }
}

uint16_t SensorEnergy::toShortPercent(float floatValue) {
    return round(min(100.0f, max(0.0f, floatValue)) * 640.0f);
}

float SensorEnergy::toFloatPercent(uint16_t shortValue) {
    return shortValue / 640.0f;
}