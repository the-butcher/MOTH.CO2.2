#include "types/Values.h"

#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"

values_t *Values::values = nullptr;

values_t Values::load() {
    values_t values = {
        0,  // nextMeasureIndex
        0,  // nextDisplayIndex
        0,  // lastDisplayIndex
        0   // nextAutoConIndex
        // measurement buffer
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

void Values::begin(values_t *values) {
    Values::values = values;
}

bool Values::isSignificantChange(float last, float curr) {
    return abs(last - curr) / last > 0.1;  // 10 percent change considered significant, however this will not work good with values close to zero
}