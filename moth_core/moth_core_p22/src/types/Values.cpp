#include "types/Values.h"

#include "sensors/SensorEnergy.h"
#include "sensors/SensorScd041.h"

values_t *Values::values = nullptr;

values_t Values::load() {
    values_t values = {
        0,  // nextMeasureIndex
        0,  // nextDisplayIndex
        0,  // lastDisplayIndex
        0,  // nextAutoNtpIndex
        0   // nextAutoPubIndex (there will be at least one call to mttt, which in case on non- or misconfiguration can set this value to 0xFFFFFF)
        // <-- implicit measurement buffer
    };
    for (uint8_t i = 0; i < MEASUREMENT_BUFFER_SIZE; i++) {
        values.measurements[i] = Values::emptyMeasurement(0);
    }
    return values;
}

values_all_t Values::emptyMeasurement(uint32_t secondstime) {
    return {
        secondstime,                                                           // secondstime of history measurement
        {0, SensorScd041::toShortDeg(0.0), SensorScd041::toShortHum(0.0), 0},  // co2 measurement
        {0.0f},                                                                // bme measurement
        {SensorEnergy::toShortPercent(0.0)}                                    // nrg measurement
    };
}

values_all_t Values::latest() {
    return Values::values->measurements[(Values::values->nextMeasureIndex - 1) % MEASUREMENT_BUFFER_SIZE];
}

void Values::begin(values_t *values) {
    Values::values = values;
}

bool Values::isSignificantChange(float last, float curr) {
    return abs(last - curr) / last > 0.1;  // 10 percent change considered significant, however this will not work good with values close to zero
}

/**
 * check if this cycle should be used to take a battery measurement
 */
bool Values::isEnergyCycle() {
    return (Values::values->nextMeasureIndex) % 5 == 0;
}

co2cal______t Values::getCo2Cal() {
    co2cal______t co2cal = {
        10000,           // minValue
        0,               // maxValue
        0,               // avgValue
        0,               // devValue
        0,               // refValue, zero = undefined for the beginning
        0,               // corValue, zero = no correction for the beginning
        CO2CAL_DISPLAY,  // type
        // values
    };
    uint32_t valueIndex;
    for (uint8_t index = 0; index < CALIBRATION_BUFFER_SIZE; index++) {
        valueIndex = index - CALIBRATION_BUFFER_SIZE + Values::values->nextMeasureIndex;
        co2cal.values[index] = Values::values->measurements[valueIndex % MEASUREMENT_BUFFER_SIZE].valuesCo2.co2Raw;
        if (co2cal.values[index] > 0) {
            co2cal.minValue = min(co2cal.minValue, co2cal.values[index]);
            co2cal.maxValue = max(co2cal.maxValue, co2cal.values[index]);
        }
#ifdef USE___SERIAL
        Serial.printf("co2cal.values[%d/%d]: %d\n", index, valueIndex, co2cal.values[index]);
#endif
    }
    uint32_t sumValue = 0;
    uint8_t numValue = 0;
    for (uint8_t index = 0; index < CALIBRATION_BUFFER_SIZE; index++) {
        if (co2cal.values[index] > 0) {
            sumValue += co2cal.values[index];
            numValue++;
        }
    }
    co2cal.avgValue = round(sumValue * 1.0f / numValue);
    uint32_t sumDeviation = 0;
    for (uint8_t index = 0; index < CALIBRATION_BUFFER_SIZE; index++) {
        if (co2cal.values[index] > 0) {
            sumDeviation += pow(co2cal.values[index] - co2cal.avgValue, 2);
        }
    }
    co2cal.devValue = round(sqrt(sumDeviation * 1.0f / numValue));

#ifdef USE___SERIAL
    Serial.printf("co2cal.minValue: %d\n", co2cal.minValue);
    Serial.printf("co2cal.maxValue: %d\n", co2cal.maxValue);
    Serial.printf("co2cal.avgValue: %d\n", co2cal.avgValue);
    Serial.printf("co2cal.devValue: %d\n", co2cal.devValue);
#endif
    return co2cal;
}