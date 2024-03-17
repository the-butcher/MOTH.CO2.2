#include "SensorScd041Base.h"

#define SCD4X_I2C_ADDRESS 0x62

/**
 * duplicate the _i2cBus variable
 */
void SensorScd041Base::begin(TwoWire& i2cBus) {
    SensirionI2CScd4x::begin(i2cBus);
    _i2cBus = &i2cBus;
}

/**
 * this is essentially a copy of the scd41 singleShot method, without the delay
 * the caller needs to take care of waiting 5 seconds, but it can be implemented
 * in a power saving way, i.e. sleeping rather than delaying
 */
uint16_t SensorScd041Base::measureSingleShotNoDelay() {
    uint16_t error;
    uint8_t buffer[2];
    SensirionI2CTxFrame txFrame(buffer, 2);

    error = txFrame.addCommand(0x219D);
    if (error) {
        return error;
    }

    error = SensirionI2CCommunication::sendFrame(SCD4X_I2C_ADDRESS, txFrame, *_i2cBus);
    delay(1);  // TODO :: experimental, trying to find the cause for 10s@1mA after readval
    return error;
}