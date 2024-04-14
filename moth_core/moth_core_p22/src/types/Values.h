#ifndef Measurement_h
#define Measurement_h

#include <Arduino.h>

const String FIELD_NAME____TIME = "time";
const String FIELD_NAME_CO2_LPF = "co2_lpf";
const String FIELD_NAME_CO2_RAW = "co2_raw";
const String FIELD_NAME_____DEG = "deg";
const String FIELD_NAME_____HUM = "hum";
const String FIELD_NAME_____HPA = "hpa";
const String FIELD_NAME_____BAT = "bat";

const float VALUE_SCALE_CO2LPF = 8.0f;

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
    uint16_t co2Lpf;  // filtered value (multiplies by 8, since there is room in uint16_t and the low pass filter will benefit from it)
    uint16_t deg;     // convention needed for fraction
    uint16_t hum;     // convention needed for fraction
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
    uint32_t nextMeasureIndex;
    uint32_t nextDisplayIndex;  // next index for a regular display refresh
    uint32_t lastDisplayIndex;  // the last index rendered in either table or chart
    uint32_t nextAutoNtpIndex;  // next index at which an ntp should be updated
    uint32_t nextAutoPubIndex;  // next index at which mqtt should be published
    values_all_t measurements[MEASUREMENT_BUFFER_SIZE];
} values_t;

/**
 * helper type for describing file name and path
 */
typedef struct {
    String path;
    String name;
} file32_def_t;

class Values {
   private:
   public:
    static values_all_t emptyMeasurement(uint32_t secondstime);
    static values_t* values;
    static values_t load();
    static void begin(values_t* values);
    static bool isSignificantChange(float last, float curr);
    static bool isEnergyCycle();
    static values_all_t latest();
};

#endif