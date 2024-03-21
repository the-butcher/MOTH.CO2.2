#ifndef ModuleTicker_h
#define ModuleTicker_h

#include <RTClib.h>

#include "types/Values.h"

class ModuleTicker {
   private:
    static RTC_PCF8523 baseTime;

   public:
    static void begin();
    static uint32_t getSecondstime();
    static String getDateTimeDisplayString(uint32_t secondstime);
    static file32_def_t getFile32Def(uint32_t secondstime, String fileFormat);
    static file32_def_t getFile32Def(DateTime date, String fileFormat);
};

#endif