#ifndef SensorBme280Base_h
#define SensorBme280Base_h

#include <Adafruit_BME280.h>

/**
 * subclass of Adafruit_BME280 implementing a "no delay" version of
 * takeForcedMeasurement
 */
class SensorBme280Base : public Adafruit_BME280 {
   private:
   public:
    bool takeForcedMeasurementNoDelay(void);
};

#endif
