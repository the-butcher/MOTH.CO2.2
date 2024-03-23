#ifndef Measurement_h
#define Measurement_h

#include <Arduino.h>

/**
 * size of the measurement buffer
 * with a measurement interval currently hardcoded to one minute it represents the minute interval in which measurements are saved
 */
const uint8_t MEASUREMENT_BUFFER_SIZE = 60;

/**
 * HISTORY_BUFFER_SIZE is identical to MEASUREMENT_BUFFER_SIZE by accident not by design, therefore redefined
 */
const uint8_t HISTORY_____BUFFER_SIZE = 60;

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

typedef struct {
    values_all_t measurements[MEASUREMENT_BUFFER_SIZE];
    uint32_t nextMeasureIndex;
    uint32_t nextDisplayIndex;
} values_t;

/**
 * helper type for describing file name and path
 */
typedef struct {
    String path;
    String name;
} file32_def_t;

#endif