#include <sys/select.h>
#include "BoxClock.h"

#include "time.h"
#include "sntp.h"

// https://randomnerdtutorials.com/esp32-ntp-timezones-daylight-saving/
#define MILLISECONDS_PER_HOUR UINT32_C(60 * 60 * 1000) // 1 HOUR

/**
 * ################################################
 * ## mutable variables
 * ################################################
 */
int64_t timeUpdateCounter = 0;

// https://sites.google.com/a/usapiens.com/opnode/time-zones
// default CET/CEST, can be overridden in /config/disp.json -> "tzn"
String timezone = "CET-1CEST,M3.5.0,M10.5.0/3"; 

/**
 * ################################################
 * ## static class variabales                     
 * ################################################
 */
RTC_PCF8523 BoxClock::baseClock;


void BoxClock::begin() {

  // from examples -> ESP... -> SimpleTime.ino
  BoxClock::baseClock.begin();

  /**
   * NTP server address could be aquired via DHCP,
   *
   * NOTE: This call should be made BEFORE esp32 aquires IP address via DHCP,
   * otherwise SNTP option 42 would be rejected by default.
   * NOTE: updateFromNtp() function call if made AFTER DHCP-client run
   * will OVERRIDE aquired NTP server address
   */
  sntp_servermode_dhcp(1);    // (optional)

  // BoxClock::setTimezone(timezone);
  // TODO set localtime from BoxClock

  // set notification call-back function 
  sntp_set_time_sync_notification_cb(BoxClock::handleUpdateFromNtp);


}

String BoxClock::getTimezone() {
  return timezone;
}

void BoxClock::setTimezone(String timezone1) {
  timezone = timezone1;
  setenv("TZ", timezone.c_str(), 1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset(); // when the sketch is uploaded, the ESP's time is kept, but the timezone is lost, this is to restore and have valid time, i.e. after a reset
  // BoxClock::updateReverse();
}

bool BoxClock::isUpdateable() {
  return millis() > (timeUpdateCounter * MILLISECONDS_PER_HOUR);
}

void BoxClock::updateFromNtp() {
  if (BoxClock::isUpdateable()) { // every MILLISECONDS_PER_HOUR milliseconds
    configTzTime(timezone.c_str(), "pool.ntp.org", "time.nist.gov");
    timeUpdateCounter++;
  }
}

// void BoxClock::updateReverse() {
//   struct tm timeinfo;
//   getLocalTime(&timeinfo);
//   DateTime now(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
//   BoxClock::baseClock.adjust(now);
// }

// Callback function (get's called when time adjusts via NTP)
void BoxClock::handleUpdateFromNtp(struct timeval *t) {
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  DateTime now(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  BoxClock::baseClock.adjust(now);  
  // BoxClock::updateReverse(); // localtime should already be set at that time
  // DateTime now(t->tv_sec + 60 * 60 * 2);
  // BoxClock::baseClock.adjust(now);
}

DateTime BoxClock::getDate() {
  return BoxClock::baseClock.now();
}

String BoxClock::getDateTimeString(DateTime date) {
  char timeBuffer[32];
  sprintf(timeBuffer, "%04d-%02d-%02d %02d:%02d:%02d", date.year(), date.month(), date.day(), date.hour(), date.minute(), date.second());
  return timeBuffer;
}

DataFileDef BoxClock::getDataFileDef(DateTime date) {
  char pathBuffer[10];
  sprintf(pathBuffer, "/%04d/%02d", date.year(), date.month());
  char nameBuffer[22];
  sprintf(nameBuffer, "/%04d/%02d/%04d%02d%02d.csv", date.year(), date.month(), date.year(), date.month(), date.day());
  return {
    String(pathBuffer),
    String(nameBuffer)
  };
}

String BoxClock::getTimeString(DateTime date) {
  char timeBuffer[16];
  sprintf(timeBuffer, "%02d:%02d", date.hour(), date.minute());
  return timeBuffer;
}
