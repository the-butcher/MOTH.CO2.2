#ifndef SensorTime_h
#define SensorTime_h

#include <RTClib.h>

#include "types/Action.h"
#include "types/Config.h"
#include "types/Define.h"
#include "types/Values.h"

const uint32_t MICROSECONDS_PER______SECOND = 1000000;
const uint16_t MICROSECONDS_PER_MILLISECOND = 1000;
const uint16_t MILLISECONDS_PER______SECOND = 1000;
const uint8_t SECONDS_PER___________MINUTE = 60;
const uint16_t SECONDS_PER_____________HOUR = SECONDS_PER___________MINUTE * 60;

class SensorTime {
   private:
    static RTC_PCF8523 baseSensor;
    static bool ntpWait;
    static void handleNtpUpdate(struct timeval* t);

   public:
    static void begin();
    static bool configure(config_t* config);
    static bool isNtpWait();
    static uint32_t getSecondstime();
    static String getDateTimeDisplayString(uint32_t secondstime);
    static file32_def_t getFile32Def(uint32_t secondstime, String fileFormat);
    static file32_def_t getFile32Def(DateTime date, String fileFormat);
    static uint32_t getSecondsUntil(uint32_t secondstime);
};

#endif