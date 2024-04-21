#include "SensorScd041.h"

#include "sensors/SensorBme280.h"
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

    // whats currently set
    float temperatureOffsetV = SensorScd041::getTemperatureOffset();
    // whats about to be set
    float temperatureOffsetC = config.sco2.temperatureOffset;
    // apply, only if there is a real change
    bool temperatureApplied = false;
    if (temperatureOffsetC != 0.0f && abs(temperatureOffsetV - temperatureOffsetC) > 0.01) {
#ifdef USE___SERIAL
        Serial.printf("!!! applying temperature offset, %f, %f !!!\n", temperatureOffsetV, temperatureOffsetC);
#endif
        SensorScd041::baseSensor.setTemperatureOffset(temperatureOffsetC);
        SensorScd041::baseSensor.setAutomaticSelfCalibration(0);
        SensorScd041::baseSensor.persistSettings();
        temperatureApplied = true;
    } else {
#ifdef USE___SERIAL
        Serial.printf("no temperature offset application needed, %f, %f\n", temperatureOffsetV, temperatureOffsetC);
#endif
    }

    SensorScd041::powerupPeriodicMeasurement();

    // SensorScd041::baseSensor.startLowPowerPeriodicMeasurement();
    return temperatureApplied;
}

bool SensorScd041::setCompensationPressure(float compensationPressure) {
    uint16_t compensationAltitude = (uint16_t)max(0.0f, round(SensorBme280::getAltitude(PRESSURE_ZERO, compensationPressure)));  // negative unsigned values jumps to 0xFFFF -> invalid co2 levels
    return SensorScd041::baseSensor.setSensorAltitude(compensationAltitude);
    // lowpowerperiodic
    // return SensorScd041::baseSensor.setAmbientPressure(compensationPressure);
}

bool SensorScd041::powerupPeriodicMeasurement() {
    // lowpowerperiodic
    // uint16_t success = SensorScd041::baseSensor.startLowPowerPeriodicMeasurement();
    // return success == 0;
    return true;
}

bool SensorScd041::depowerPeriodicMeasurement() {
    // lowpowerperiodic
    // uint16_t success = SensorScd041::baseSensor.stopPeriodicMeasurement();
    // delay(500);
    // return success == 0;
    return true;
}

co2cal______t SensorScd041::forceCalibration(uint16_t requestedCo2Ref) {

    co2cal______t result = Values::getCo2Cal();  // get the initial set of values
    result.refValue = requestedCo2Ref;

    SensorScd041::depowerPeriodicMeasurement();  // stop periodic if set to be periodic

    // the correction that is expected from the calibration
    int16_t corExpct = requestedCo2Ref - result.avgValue;

    uint16_t frcV = 0;
    uint16_t& frcR = frcV;
    SensorScd041::baseSensor.performForcedRecalibration(requestedCo2Ref, frcR);
    delay(400);

    if (frcV != 0xffff) {  // other than error

        int16_t corValue = (int16_t)(frcV - 0x8000);

        // assume avgValue was 450 and requestedCo2Ref was 420 -> corExpct would be -30
        // if corValue came out at -20, an additional amount of -10 would have to be applied

        requestedCo2Ref += corExpct;  // -30 -> 390
        requestedCo2Ref -= corValue;  // -20 -> 410

        int16_t corExtra = corValue - corExpct;

#ifdef USE___SERIAL
        Serial.printf("corExpct: %d, corValue: %d, requestedCo2Ref: %d\n", corExpct, corValue, requestedCo2Ref);
#endif

        // TODO :: second forced calibration with the corrected value
        // add second correction to first, return as final correction, treat error if present

        result.corValue = corValue;
        result.type = CO2CAL_SUCCESS;

    } else {  // error

        result.corValue = 0;
        result.type = CO2CAL_FAILURE;
    }

    SensorScd041::powerupPeriodicMeasurement();  // start periodic if set to be periodic

    return result;
}

co2cal______t SensorScd041::forceReset() {

    co2cal______t result = Values::getCo2Cal();  // get the initial set of values

    SensorScd041::depowerPeriodicMeasurement();  // stop periodic if set to be periodic

    uint16_t success = SensorScd041::baseSensor.performFactoryReset();
    delay(400);

    SensorScd041::powerupPeriodicMeasurement();  // start periodic if set to be periodic

    result.refValue = 0;
    result.corValue = 0;
    result.type = success == 0 ? CO2CAL_SUCCESS : CO2CAL_FAILURE;  // zero indicates success
}

bool SensorScd041::measure() {
    return SensorScd041::baseSensor.measureSingleShotNoDelay();
    // lowpowerperiodic
    // return true;
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
        SensorScd041::values = {(uint16_t)round(co2 * VALUE_SCALE_CO2LPF), SensorScd041::toShortDeg(deg), SensorScd041::toShortHum(hum), co2};
    }
    return SensorScd041::values;
}

float SensorScd041::getTemperatureOffset() {
    float degV = 0;
    float& degR = degV;
    SensorScd041::baseSensor.getTemperatureOffset(degR);
    delay(1);
    return degV;
}

uint16_t SensorScd041::getCompensationAltitude() {
    uint16_t altV = 0;
    uint16_t& altR = altV;
    SensorScd041::baseSensor.getSensorAltitude(altR);
    delay(1);
    return altV;
}

bool SensorScd041::isAutomaticSelfCalibration() {
    uint16_t ascV = 0;
    uint16_t& ascR = ascV;
    SensorScd041::baseSensor.getAutomaticSelfCalibration(ascR);
    delay(1);
    return ascV;
}

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
    return celsius * 9.0 / 5.0 + 32.0;
}