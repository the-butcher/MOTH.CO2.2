#include "types/Values.h"

values_t *Values::values = nullptr;

values_t Values::load() {
    return {
        0,  // nextMeasureIndex
        0,  // nextDisplayIndex
        0   // nextAutoConIndex
        // measurement buffer
    };
}

void Values::begin(values_t *values) {
    Values::values = values;
}
