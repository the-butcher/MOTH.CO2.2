#include "types/Values.h"

#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"

values_t *Values::values = nullptr;

values_t Values::load() {
    values_t values = {
        0,  // nextMeasureIndex
        0,  // nextDisplayIndex
        0,  // lastDisplayIndex
        0,  // nextAutoNtpIndex
        0   // nextAutoPubIndex (there will be at least one call to mttt, which in case on non- or misconfiguration can set this value to 0xFFFFFF)
        // <-- implicit measurement buffer
    };
    for (uint8_t i = 0; i < MEASUREMENT_BUFFER_SIZE; i++) {
        values.measurements[i] = Values::emptyMeasurement(0);
    }
    return values;
}

values_all_t Values::emptyMeasurement(uint32_t secondstime) {
    return {
        secondstime,                                                           // secondstime of history measurement
        {0, SensorScd041::toShortDeg(0.0), SensorScd041::toShortHum(0.0), 0},  // co2 measurement
        {0.0f},                                                                // bme measurement
        {SensorEnergy::toShortPercent(0.0)}                                    // nrg measurement
    };
}

values_all_t Values::latest() {
    return Values::values->measurements[(Values::values->nextMeasureIndex - 1) % MEASUREMENT_BUFFER_SIZE];
}

void Values::begin(values_t *values) {
    Values::values = values;
}

bool Values::isSignificantChange(float last, float curr) {
    return abs(last - curr) / last > 0.1;  // 10 percent change considered significant, however this will not work good with values close to zero
}

/**
 * check if this cycle should be used to take a battery measurement
 */
bool Values::isEnergyCycle() {
    return (Values::values->nextMeasureIndex) % 5 == 0;
}