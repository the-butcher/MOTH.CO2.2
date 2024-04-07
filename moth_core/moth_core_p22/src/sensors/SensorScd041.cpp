#include "SensorScd041.h"

#include "types/Define.h"

SensorScd041Base SensorScd041::baseSensor;
values_co2_t SensorScd041::values = {0, 0, 0};

void SensorScd041::begin() {
    SensorScd041::baseSensor.begin(Wire);
}

/**
 * called once when the device boots
 */
bool SensorScd041::configure(config_t& config) {

    // check current setting first
    float temperatureOffsetV = 0;
    float& temperatureOffsetR = temperatureOffsetV;
    SensorScd041::baseSensor.getTemperatureOffset(temperatureOffsetR);
    // whats about to be set
    float temperatureOffsetC = config.sco2.temperatureOffset;
    // apply, only if there is a real change
    bool temperatureApplied = false;
    if (abs(temperatureOffsetV - temperatureOffsetC) > 0.01) {
        SensorScd041::baseSensor.setTemperatureOffset(temperatureOffsetC);
        SensorScd041::baseSensor.setAutomaticSelfCalibration(0);
        SensorScd041::baseSensor.persistSettings();
        temperatureApplied = true;
    }
    // SensorScd041::baseSensor.startLowPowerPeriodicMeasurement();
    return temperatureApplied;
}

bool SensorScd041::setCompensationAltitude(uint16_t compensationAltitude) {
    return SensorScd041::baseSensor.setSensorAltitude(compensationAltitude);
}

calibration_t SensorScd041::forceCalibration(uint16_t requestedCo2Ref) {

    uint32_t nextMeasureIndex = Values::values->nextMeasureIndex;
    uint32_t currMeasureIndex = nextMeasureIndex - 1;
    values_all_t measurement;
    uint16_t co2Lpf = 0;
    uint16_t co2Raw = 0.0f;
    for (int i = 0; i < 3; i++) {
        measurement = Values::values->measurements[(currMeasureIndex - i + MEASUREMENT_BUFFER_SIZE) % MEASUREMENT_BUFFER_SIZE];
        if (i == 0) {
            co2Lpf = measurement.valuesCo2.co2Lpf;
        }
        co2Raw += measurement.valuesCo2.co2Raw;
    }
    uint16_t correctedCo2Ref = requestedCo2Ref - co2Lpf + (uint16_t)round(co2Raw / 3.0f);

    uint16_t frcV = 0;
    uint16_t& frcR = frcV;
    int16_t offV = 0x8000;
    SensorScd041::baseSensor.performForcedRecalibration(correctedCo2Ref, frcR);
    delay(400);

    // #ifdef USE___SERIAL
    //     Serial.printf("frcV: %d\n", frcV);
    // #endif

    if (frcV == 0xffff) {
        return {false, ACTION___CALIBRATION, requestedCo2Ref, correctedCo2Ref, 0};
    } else {
        return {true, ACTION___CALIBRATION, requestedCo2Ref, correctedCo2Ref, (int16_t)(frcV - offV)};
    }
}

calibration_t SensorScd041::forceReset() {
    uint16_t success = SensorScd041::baseSensor.performFactoryReset();
    delay(400);
    return {
        success == 0,  // zero indicates success
        ACTION_FACTORY_RESET,
        0,  // no requested value
        0,  // no corrected value
        0   // no applied offset
    };
}

bool SensorScd041::measure() {
    return SensorScd041::baseSensor.measureSingleShotNoDelay();
}

bool SensorScd041::powerup(config_t& config) {
    // if (SensorScd041::mode = SCO2___VAL_M_CYCLED) {  // currently depowered
    //     bool success = SensorScd041::baseSensor.wakeUp();
    //     if (success) {
    //         SensorScd041::mode = SCO2___VAL_M___IDLE;  // could be powered and is now IDLE
    //     }
    //     return success;
    // } else {
    //     // nothing to do (was already in IDLE)
    //     return true;
    // }
    return true;
}

bool SensorScd041::depower(config_t& config) {

    // TODO :: find out if powerup and depower are needed with the i2c power cycle

    // if (config.sco2.sensorMode == SCO2___VAL_M_CYCLED) {  // aloowed to toggle to cycled
    //     bool success = SensorScd041::baseSensor.powerDown();
    //     if (success) {
    //         SensorScd041::mode = SCO2___VAL_M_CYCLED;
    //     }
    //     return success;
    // } else {
    //     // nothing to do (the configured IDLE mode forbids depowering)
    //     return true;
    // }
    return true;
}

values_co2_t SensorScd041::readval() {
    bool isDataReady = false;
    SensorScd041::baseSensor.getDataReadyFlag(isDataReady);
    if (isDataReady) {
        uint16_t co2;
        float deg;
        float hum;
        SensorScd041::baseSensor.readMeasurement(co2, deg, hum);
        SensorScd041::values = {co2, SensorScd041::toShortDeg(deg), SensorScd041::toShortHum(hum), co2};
    }
    return SensorScd041::values;
}

// float SensorScd041::getTemperatureOffset() {
//     float degV = 0;
//     float& degR = degV;
//     SensorScd041::baseSensor.getTemperatureOffset(degR);
//     return degV;
// }

// uint16_t SensorScd041::getCompensationAltitude() {
//     uint16_t altV = 0;
//     uint16_t& altR = altV;
//     SensorScd041::baseSensor.getSensorAltitude(altR);
//     return altV;
// }

// bool SensorScd041::isAutomaticSelfCalibration() {
//     uint16_t ascV = 0;
//     uint16_t& ascR = ascV;
//     SensorScd041::baseSensor.getAutomaticSelfCalibration(ascR);
//     return ascV;
// }

uint16_t SensorScd041::toShortDeg(float floatValue) {
    return round((min(50.0f, max(-50.0f, floatValue)) + 50.0f) * 640.0f);
}

float SensorScd041::toFloatDeg(uint16_t shortValue) {
    return shortValue / 640.0f - 50.0f;
}

uint16_t SensorScd041::toShortHum(float floatValue) {
    return round(min(100.0f, max(0.0f, floatValue)) * 640.0f);
}

float SensorScd041::toFloatHum(uint16_t shortValue) {
    return shortValue / 640.0f;
}

float SensorScd041::toFahrenheit(float celsius) {
#ifdef USE___SERIAL
    Serial.printf("celsius: %d\n", celsius);
#endif
    return celsius * 9.0 / 5.0 + 32.0;
}