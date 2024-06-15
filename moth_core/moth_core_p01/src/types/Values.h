#ifndef Measurement_h
#define Measurement_h

#include <Arduino.h>

const String FIELD_NAME____TIME = "time";
const String FIELD_NAME___PM010 = "pm010";
const String FIELD_NAME___PM025 = "pm025";
const String FIELD_NAME___PM100 = "pm100";
const String FIELD_NAME_____DEG = "deg";
const String FIELD_NAME_____HUM = "hum";
const String FIELD_NAME_____HPA = "hpa";

/**
 * helper type for describing file name and path
 */
typedef struct {
    String path;
    String name;
    bool exists;
} file32_def_t;

/**
 * size of the measurement buffer
 * with a measurement interval currently hardcoded to one minute it represents the minute interval in which measurements are saved
 */
const uint8_t MEASUREMENT_BUFFER_SIZE = 60;

typedef struct {
    uint16_t pm010;
    uint16_t pm025;
    uint16_t pm100;
} values_pms_t;

typedef struct {
    float deg;
    float hum;
    float pressure;
} values_bme_t;

typedef struct {
    uint32_t secondstime;
    values_pms_t valuesPms;
    values_bme_t valuesBme;
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

class Values {
   private:
   public:
    static values_all_t emptyMeasurement(uint32_t secondstime);
    static values_t* values;
    static values_t load();
    static void begin(values_t* values);
    static values_all_t latest();
};

#endif