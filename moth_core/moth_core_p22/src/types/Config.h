#ifndef Config_h
#define Config_h

#include <Arduino.h>

typedef struct {
    uint16_t warnHi;     // upper warn level
    uint16_t riskHi;     // upper risk level
    uint16_t reference;  // co2 reference value (i.e. 425)
} thresholds_co2_t;

typedef struct {
    uint8_t riskLo;  // lower risk level
    uint8_t wanrLo;  // lower warn level
    uint8_t warnHi;  // upper warn level
    uint8_t riskHi;  // upper risk level
} thresholds_lh_t;

typedef enum {
    DISPLAY_VAL_T___CO2,
    DISPLAY_VAL_T___HPA
} display_val_t_e;

typedef struct {
    thresholds_co2_t thresholdsCo2;
    thresholds_lh_t thresholdsDeg;
    thresholds_lh_t thresholdsHum;
    display_val_t_e displayValTable;
    bool isFahrenheit;
    bool isBeep;
    float temperatureOffset;
} config_t;

#endif