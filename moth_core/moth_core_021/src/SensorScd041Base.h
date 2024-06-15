#ifndef SensorScd041Base_h
#define SensorScd041Base_h

#include <SensirionI2CScd4x.h>

/**
 * subclass of SensirionI2CScd4x implementing a "no delay" version of
 * measureSingleShot
 */
class SensorScd041Base : public SensirionI2CScd4x {
   private:
    TwoWire* _i2cBus = nullptr;

   public:
    void begin(TwoWire& i2cBus);
    uint16_t measureSingleShotNoDelay(void);
};

#endif
