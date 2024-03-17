#ifndef Measurement_h
#define Measurement_h

#include <Arduino.h>

typedef struct {
    uint16_t co2;
    uint16_t deg;  // convention needed for fraction
    uint16_t hum;  // convention needed for fraction
    uint16_t co2Raw;
} measurement_co2_t;

typedef struct {
    float pressure;  // precision required for altitude conversion
} measurement_bme_t;

typedef struct {
    uint16_t percent;  // convention needed for fraction
} measurement_nrg_t;

typedef struct {
    uint32_t secondstime;
    measurement_co2_t valuesCo2;
    measurement_bme_t valuesBme;
    measurement_nrg_t valuesNrg;
    bool publishable;
} measurement_t;

#endif