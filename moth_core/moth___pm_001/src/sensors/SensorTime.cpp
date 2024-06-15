#include "SensorTime.h"

#include <sntp.h>
#include <time.h>

#include "modules/ModuleCard.h"
#include "types/Define.h"

bool SensorTime::ntpWait = false;

/**
 * called every time the main setup function is called
 */
void SensorTime::begin() {
    sntp_servermode_dhcp(1);
    sntp_set_time_sync_notification_cb(SensorTime::handleNtpUpdate);
}

void SensorTime::setupNtpUpdate(config_t& config) {
    setenv("TZ", config.time.timezone, 1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
    tzset();
    configTzTime(config.time.timezone, "pool.ntp.org", "time.nist.gov");  // will trigger actual ntp update call
    SensorTime::ntpWait = true;
}

/**
 *  Callback function (get's called when time adjusts via NTP)
 */
void SensorTime::handleNtpUpdate(struct timeval* t) {
    Serial.println("handling ntp update");
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    DateTime now(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Config::setUtcOffsetSeconds(SECONDS_FROM_1970_TO_2000 + now.secondstime() - t->tv_sec);
    SensorTime::ntpWait = false;
}

bool SensorTime::isNtpWait() {
    return SensorTime::ntpWait;
}

uint32_t SensorTime::getSecondsUntil(uint32_t secondsdest) {
    uint32_t secondstime = SensorTime::getSecondstime();
    return secondsdest > secondstime ? secondsdest - secondstime : WAITTIME________________NONE;
}

uint32_t SensorTime::getSecondstime() {

    time_t timeNow;
    time(&timeNow);

    struct tm* timeinfo;
    timeinfo = localtime(&timeNow);

    DateTime now(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    return now.secondstime();
}

file32_def_t SensorTime::getFile32Def(uint32_t secondstime, String fileFormat) {
    return SensorTime::getFile32Def(DateTime(SECONDS_FROM_1970_TO_2000 + secondstime), fileFormat);
}

file32_def_t SensorTime::getFile32Def(DateTime date, String fileFormat) {
    char pathBuffer[10];
    sprintf(pathBuffer, "/%04d/%02d", date.year(), date.month());
    char nameBuffer[22];
    sprintf(nameBuffer, "/%04d/%02d/%04d%02d%02d.%s", date.year(), date.month(), date.year(), date.month(), date.day(), fileFormat);
    String name = String(nameBuffer);
    return {String(pathBuffer), name, ModuleCard::existsPath(name)};
}

String SensorTime::getDateTimeSecondsString(uint32_t secondstime) {
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + secondstime);
    char timeBuffer[32];
    sprintf(timeBuffer, "%04d-%02d-%02dT%02d:%02d:%02d", date.year(), date.month(), date.day(), date.hour(), date.minute(), date.second());
    return timeBuffer;
}

String SensorTime::getDateTimeLastModString(uint32_t secondstime) {
    DateTime dateUtc = DateTime(SECONDS_FROM_1970_TO_2000 + secondstime - Config::getUtcOffsetSeconds());
    char timeBuffer[32];
    sprintf(timeBuffer, "%s, %02d %s %04d %02d:%02d:%02d GMT", SensorTime::getNameOfDay(dateUtc), dateUtc.day(), SensorTime::getNameOfMonth(dateUtc), dateUtc.year(), dateUtc.hour(), dateUtc.minute(), dateUtc.second());
    return timeBuffer;
}

String SensorTime::getDateTimeDisplayString(uint32_t secondstime) {
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + secondstime);
    char timeBuffer[32];
    sprintf(timeBuffer, "%02d.%02d.%04d,%02d:%02d", date.day(), date.month(), date.year(), date.hour(), date.minute());
    return timeBuffer;
}

String SensorTime::getNameOfDay(DateTime date) {
    uint8_t indexOfDay = date.dayOfTheWeek();
    if (indexOfDay == 0) {
        return "Sun";
    } else if (indexOfDay == 1) {
        return "Mon";
    } else if (indexOfDay == 2) {
        return "Tue";
    } else if (indexOfDay == 3) {
        return "Wed";
    } else if (indexOfDay == 4) {
        return "Thu";
    } else if (indexOfDay == 5) {
        return "Fri";
    } else if (indexOfDay == 6) {
        return "Sat";
    } else {
        return "NA";
    }
}

String SensorTime::getNameOfMonth(DateTime date) {
    uint8_t indexOfMonth = date.month();
    if (indexOfMonth == 1) {
        return "Jan";
    } else if (indexOfMonth == 2) {
        return "Feb";
    } else if (indexOfMonth == 3) {
        return "Mar";
    } else if (indexOfMonth == 4) {
        return "Apr";
    } else if (indexOfMonth == 5) {
        return "May";
    } else if (indexOfMonth == 6) {
        return "Jun";
    } else if (indexOfMonth == 7) {
        return "Jul";
    } else if (indexOfMonth == 8) {
        return "Aug";
    } else if (indexOfMonth == 9) {
        return "Sep";
    } else if (indexOfMonth == 10) {
        return "Oct";
    } else if (indexOfMonth == 11) {
        return "Nov";
    } else if (indexOfMonth == 12) {
        return "Dec";
    } else {
        return "NA";
    }
}
