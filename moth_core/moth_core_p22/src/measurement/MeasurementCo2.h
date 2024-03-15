#ifndef MeasurementCo2_h
#define MeasurementCo2_h

#include <Arduino.h>

typedef struct {
    uint16_t co2;
    int16_t temperature;  // convention needed for fraction
    uint16_t humidity;    // convention needed for fraction
    uint16_t co2Raw;
} MeasurementCo2;

#endif