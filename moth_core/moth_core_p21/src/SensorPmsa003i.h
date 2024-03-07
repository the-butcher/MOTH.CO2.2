#ifndef SensorPmsa003i_h
#define SensorPmsa003i_h

#include <Arduino.h>
#include "Adafruit_PM25AQI.h"
#include "ValuesPms.h"
#include "RunningAverage.h"

typedef enum {
    PMS____ON_M, // on in measurement interval
    PMS____ON_D, // on in display interval
    PMS_____OFF,
    PMS_PAUSE_M, // paused in measurement interval
    PMS_PAUSE_D, // paused in display interval
} pms_mode_t;

class SensorPmsa003i {
  
  private:
    static Adafruit_PM25AQI baseSensor;
    static pms_mode_t mode;

  public:
    static bool ACTIVE;
    static int WARMUP_SECONDS;
    static gpio_num_t PMS_ENABLE___GPIO;
    static void setMode(pms_mode_t mode);
    static pms_mode_t getMode();
    static void begin();
    static bool tryRead();
    static ValuesPms values;
    
};

#endif