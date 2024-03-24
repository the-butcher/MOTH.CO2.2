#ifndef ModuleClock_h
#define ModuleClock_h

#include <RTClib.h>

#include "types/Config.h"
#include "types/Define.h"
#include "types/Values.h"

class ModuleClock {
   private:
    static RTC_PCF8523 baseTime;
    static void handleNtpUpdate(struct timeval* t);

   public:
    static void begin();
    static bool configure(config_t* config);
    static uint32_t getSecondstime();
    static String getDateTimeDisplayString(uint32_t secondstime);
    static file32_def_t getFile32Def(uint32_t secondstime, String fileFormat);
    static file32_def_t getFile32Def(DateTime date, String fileFormat);
};

#endif