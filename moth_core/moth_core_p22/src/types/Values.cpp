#include "types/Values.h"

values_t Values::load() {
    return {
        0,  // nextMeasureIndex
        0,  // nextDisplayIndex
        0   // nextConnectIndex
    };
}
