#ifndef Measurement_h
#define Measurement_h

#include <Arduino.h>

#include "MeasurementBme.h"
#include "MeasurementCo2.h"
#include "MeasurementNrg.h"

typedef struct {
    uint32_t secondstime;
    MeasurementCo2 valuesCo2;
    MeasurementBme valuesBme;
    MeasurementNrg valuesBat;
    bool publishable;
} Measurement;

#endif