#ifndef Measurement_h
#define Measurement_h

#include <Arduino.h>

typedef struct {
    uint16_t co2;
    uint16_t deg;  // convention needed for fraction
    uint16_t hum;  // convention needed for fraction
    uint16_t co2Raw;
} values_co2_t;

typedef struct {
    float pressure;  // precision required for altitude conversion
} values_bme_t;

typedef struct {
    uint16_t percent;  // convention needed for fraction
} values_nrg_t;

typedef struct {
    uint32_t secondstime;
    values_co2_t valuesCo2;
    values_bme_t valuesBme;
    values_nrg_t valuesNrg;
    bool publishable;
} values_all_t;

/**
 * helper type for describing file name and path
 */
typedef struct {
    String path;
    String name;
} file32_def_t;

#endif