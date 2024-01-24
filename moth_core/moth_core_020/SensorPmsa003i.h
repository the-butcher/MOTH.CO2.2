#ifndef SensorPmsa003i_h
#define SensorPmsa003i_h

#include <Arduino.h>
#include "Adafruit_PM25AQI.h"
#include "ValuesPms.h"
#include "RunningAverage.h"

typedef enum {
    PMS_____ON,
    PMS____OFF,
    PMS_PAUSED
} pms_mode_t;

class SensorPmsa003i {
  
  private:
    static Adafruit_PM25AQI baseSensor;
    static pms_mode_t mode;
    static RunningAverage avgPm010;
    static RunningAverage avgPm025;
    static RunningAverage avgPm100;
    static RunningAverage avgPc003;
    static RunningAverage avgPc005;
    static RunningAverage avgPc010;
    static RunningAverage avgPc025;
    static RunningAverage avgPc050;
    static RunningAverage avgPc100;

  public:
    static bool ACTIVE;
    static int WARMUP_SECONDS;
    static void setMode(pms_mode_t mode);
    static pms_mode_t getMode();
    static void begin();
    static bool tryRead();
    static ValuesPms getValues();
    
};

#endif