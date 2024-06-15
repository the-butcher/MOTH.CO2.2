#ifndef SensorTime_h
#define SensorTime_h

#include <Arduino.h>
#include <RTClib.h>

#include "FS.h"
#include "types/Action.h"
#include "types/Config.h"
#include "types/Values.h"

const uint16_t MICROSECONDS_PER_MILLISECOND = 1000;
const uint16_t MILLISECONDS_PER______SECOND = 1000;
const uint32_t MICROSECONDS_PER______SECOND = MILLISECONDS_PER______SECOND * MICROSECONDS_PER_MILLISECOND;
const uint8_t SECONDS_PER___________MINUTE = 60;
const uint16_t SECONDS_PER_____________HOUR = SECONDS_PER___________MINUTE * 60;

const uint32_t WAITTIME________________NONE = 0;
const uint32_t WAITTIME_DISPLAY_AND_DEPOWER = 5;  // very conservative estimation, 3 or maybe even 2 could also work

const file32_def_t FILE_DEF_EMPTY = {"", "", false};

class SensorTime {
   private:
    static bool ntpWait;
    static void handleNtpUpdate(struct timeval* t);
    static String getNameOfDay(DateTime date);
    static String getNameOfMonth(DateTime date);

   public:
    static void begin();
    static void setupNtpUpdate(config_t& config);
    static bool isNtpWait();
    static uint32_t getSecondstime();
    static String getDateTimeLastModString(uint32_t secondstime);
    static String getDateTimeSecondsString(uint32_t secondstime);
    static String getDateTimeDisplayString(uint32_t secondstime);
    static file32_def_t getFile32Def(uint32_t secondstime, String fileFormat);
    static file32_def_t getFile32Def(DateTime date, String fileFormat);
    static uint32_t getSecondsUntil(uint32_t secondstime);
};

#endif