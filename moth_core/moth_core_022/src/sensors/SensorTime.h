#ifndef SensorTime_h
#define SensorTime_h

#include <Arduino.h>
#include <RTClib.h>
#include <SdFat.h>

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

/**
 * GPIO_NUM_8  :: A5
 */
const gpio_num_t PIN_RTC_SQW = GPIO_NUM_8;
const file32_def_t FILE_DEF_EMPTY = {"", "", false};

class SensorTime {
   private:
    static RTC_PCF8523 baseSensor;
    static bool ntpWait;
    static bool interrupted;
    static void handleInterrupt();
    static void handleNtpUpdate(struct timeval* t);
    static String getNameOfDay(DateTime date);
    static String getNameOfMonth(DateTime date);

   public:
    static void begin();
    static void configure(config_t& config);
    static void setupNtpUpdate(config_t& config);
    static void prepareSleep(wakeup_action_e wakeupType);
    static void attachWakeup(wakeup_action_e wakeupType);
    static void detachWakeup(wakeup_action_e wakeupType);
    static bool isNtpWait();
    static uint32_t getSecondstime();
    static String getDateTimeLastModString(uint32_t secondstime);
    static String getDateTimeLastModString(File32 file);
    static String getDateTimeSecondsString(uint32_t secondstime);
    static String getDateTimeDisplayString(uint32_t secondstime);
    static file32_def_t getFile32Def(uint32_t secondstime, String fileFormat);
    static file32_def_t getFile32Def(DateTime date, String fileFormat);
    static file32_def_t getFile32DefData(uint32_t secondstime);
    static file32_def_t getFile32DefData(DateTime date);
    static bool isPersistPath(String path);
    static uint32_t getSecondsUntil(uint32_t secondstime);
    static bool isInterrupted();
    static void dateTimeCallback(uint16_t* date, uint16_t* time);
};

#endif