#ifndef BoxClock_h
#define BoxClock_h

#include "RTClib.h"
#include "DataFileDef.h"

class BoxClock {
  
  private:
    static RTC_PCF8523 baseClock;
    static int64_t timeUpdateCounter;
    static void handleNtpUpdate(struct timeval *t);

    
  public:
    static void setTimezone(String timezone);
    static String getTimezone();
    static void begin();
    static DateTime getDate();
    static bool isUpdateable();
    static void optNtpUpdate();
    static String getTimeString(DateTime date);
    static String getDateTimeString(DateTime date);
    static DataFileDef getDataFileDef(DateTime date);
    // static String getDataFileName(DateTime date);

};

#endif