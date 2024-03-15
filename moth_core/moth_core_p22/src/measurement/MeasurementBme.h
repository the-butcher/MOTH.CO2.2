#ifndef MeasurementBme_h
#define MeasurementBme_h

#include <Arduino.h>

typedef struct {
    uint16_t pressure;  // convention needed for fraction (65535)
} MeasurementBme;

#endif