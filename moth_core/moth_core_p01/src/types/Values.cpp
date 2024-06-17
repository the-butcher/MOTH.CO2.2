#include "types/Values.h"

values_t *Values::values = nullptr;

values_t Values::load() {
    values_t values = {
        0,  // nextMeasureIndex
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
        secondstime,        // secondstime of history measurement
        {0, 0, 0},          // pms measurement
        {0.0f, 0.0f, 0.0f}  // bme measurement
    };
}

values_all_t Values::latest() {
    return Values::values->measurements[(Values::values->nextMeasureIndex - 1) % MEASUREMENT_BUFFER_SIZE];
}

void Values::begin(values_t *values) {
    Values::values = values;
}
