#ifndef ModuleTicker_h
#define ModuleTicker_h

#include <RTClib.h>

#include "types/Values.h"

class ModuleTicker {
   private:
    RTC_PCF8523 baseTime;

   public:
    void begin();
    DateTime getDate();
    static String getDateTimeDisplayString(uint32_t secondstime);
    static file32_def_t getFile32Def(uint32_t secondstime);
    static uint32_t WAITTIME________________NONE;
    static uint32_t WAITTIME_DISPLAY_AND_DEPOWER;
    static uint32_t MILLISECONDS_PER______SECOND;
    static uint32_t MICROSECONDS_PER______SECOND;
    static uint32_t MICROSECONDS_PER_MILLISECOND;
};

#endif