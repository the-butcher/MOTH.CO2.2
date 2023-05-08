#ifndef BoxClock_h
#define BoxClock_h

#include "RTClib.h"

class BoxClock {
  
  private:
    // static void handleTimeAvailable(struct timeval *t);

    
  public:
    static void setTimezone(String timezone);
    static String getTimezone();
    static void begin();
    static DateTime getDate();
    static void updateTime();
    static String getTimeString(DateTime date);
    static String getDateTimeString(DateTime date);
    static String getDataFileName(DateTime date);

};

#endif