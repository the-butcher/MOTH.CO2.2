#include "ModuleClock.h"

#include "sntp.h"
#include "time.h"

RTC_PCF8523 ModuleClock::baseTime;

void ModuleClock::begin() {
    ModuleClock::baseTime.begin();
    sntp_servermode_dhcp(1);
    sntp_set_time_sync_notification_cb(ModuleClock::handleNtpUpdate);
}

bool ModuleClock::configure(config_t* config) {
    setenv("TZ", config->time.timezone, 1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
    tzset();
    configTzTime(config->time.timezone, "pool.ntp.org", "time.nist.gov");
    return true;
}

// Callback function (get's called when time adjusts via NTP)
void ModuleClock::handleNtpUpdate(struct timeval* t) {
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    DateTime now(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    ModuleClock::baseTime.adjust(now);
}

uint32_t ModuleClock::getSecondstime() {
    return ModuleClock::baseTime.now().secondstime();
}

file32_def_t ModuleClock::getFile32Def(uint32_t secondstime, String fileFormat) {
    return ModuleClock::getFile32Def(DateTime(SECONDS_FROM_1970_TO_2000 + secondstime), fileFormat);
}

file32_def_t ModuleClock::getFile32Def(DateTime date, String fileFormat) {
    char pathBuffer[10];
    sprintf(pathBuffer, "/%04d/%02d", date.year(), date.month());
    char nameBuffer[22];
    sprintf(nameBuffer, "/%04d/%02d/%04d%02d%02d.%s", date.year(), date.month(), date.year(), date.month(), date.day(), fileFormat);
    return {String(pathBuffer), String(nameBuffer)};
}

String ModuleClock::getDateTimeDisplayString(uint32_t secondstime) {
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + secondstime);
    char timeBuffer[32];
    sprintf(timeBuffer, "%02d.%02d.%04d,%02d:%02d", date.day(), date.month(), date.year(), date.hour(), date.minute());
    return timeBuffer;
}
