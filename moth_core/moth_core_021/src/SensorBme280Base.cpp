#include "SensorBme280Base.h"

/**
 * this is essentially a copy of the bme280 takeForcedMeasurement method,
 * without the delay the caller needs to take care of waiting long enough, but
 * it can be implemented in a power saving way, i.e. sleeping rather than
 * delaying
 */
bool SensorBme280Base::takeForcedMeasurementNoDelay(void) {
    if (_measReg.mode == MODE_FORCED) {
        write8(BME280_REGISTER_CONTROL, _measReg.get());
        return true;
    } else {
        return false;
    }
}