#ifndef BoxTime_h
#define BoxTime_h

#include <RTClib.h>

class BoxTime {
   private:
    RTC_PCF8523 baseTime;

   public:
    void begin();
    DateTime getDate();
    static uint32_t WAITTIME________________NONE;
    static uint32_t WAITTIME_DISPLAY_AND_DEPOWER;
    static uint32_t MILLISECONDS_PER______SECOND;
    static uint32_t MICROSECONDS_PER______SECOND;
    static uint32_t MICROSECONDS_PER_MILLISECOND;
};

#endif