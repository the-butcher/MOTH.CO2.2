#ifndef SensorPms003_h
#define SensorPms003_h

#include <Adafruit_PM25AQI.h>

#include "types/Values.h"

class SensorPms003 {
   private:
    static Adafruit_PM25AQI baseSensor;
    static values_pms_t values;

   public:
    static void begin();
    static bool measure();
    static values_pms_t readval();
};

#endif