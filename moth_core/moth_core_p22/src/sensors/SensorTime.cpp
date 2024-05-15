#include "SensorTime.h"

#include <sntp.h>
#include <time.h>

#include "modules/ModuleCard.h"
#include "types/Define.h"

RTC_PCF8523 SensorTime::baseSensor;
bool SensorTime::ntpWait = false;
bool SensorTime::interrupted = false;
int32_t SensorTime::secondstimeOffsetUtc;

/**
 * called once when the device boots
 */
void SensorTime::configure(config_t& config) {
    SensorTime::baseSensor.deconfigureAllTimers();
    SensorTime::baseSensor.enableCountdownTimer(PCF8523_FrequencyMinute, 1);
}

/**
 * called every time the main setup function is called
 */
void SensorTime::begin() {
    SensorTime::baseSensor.begin();

    rtc_gpio_deinit(PIN_RTC_SQW);
    pinMode(PIN_RTC_SQW, INPUT_PULLUP);

    sntp_servermode_dhcp(1);
    sntp_set_time_sync_notification_cb(SensorTime::handleNtpUpdate);
}

/**
 * the rtc sqw pin pulls low once per minute
 */
void SensorTime::prepareSleep(wakeup_action_e wakeupType) {
    if (wakeupType == WAKEUP_ACTION_BUTN) {
        gpio_hold_en(PIN_RTC_SQW);
        rtc_gpio_pullup_en(PIN_RTC_SQW);
        rtc_gpio_pulldown_dis(PIN_RTC_SQW);
    } else {
        gpio_hold_dis(PIN_RTC_SQW);
    }
}

void SensorTime::attachWakeup(wakeup_action_e wakeupType) {
    if (wakeupType == WAKEUP_ACTION_BUTN) {
        attachInterrupt(digitalPinToInterrupt(PIN_RTC_SQW), SensorTime::handleInterrupt, FALLING);
        SensorTime::interrupted = false;
    }
}

void SensorTime::detachWakeup(wakeup_action_e wakeupType) {
    if (wakeupType == WAKEUP_ACTION_BUTN) {
        detachInterrupt(digitalPinToInterrupt(PIN_RTC_SQW));
        SensorTime::interrupted = false;
    }
}

void SensorTime::handleInterrupt() {
    SensorTime::interrupted = true;
}

bool SensorTime::isInterrupted() {
    return SensorTime::interrupted;
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
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    DateTime now(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    SensorTime::baseSensor.adjust(now);
    SensorTime::secondstimeOffsetUtc = SECONDS_FROM_1970_TO_2000 + now.secondstime() - t->tv_sec;
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
    return SensorTime::baseSensor.now().secondstime();
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

file32_def_t SensorTime::getFile32DefData(uint32_t secondstime) {
    return SensorTime::getFile32DefData(DateTime(SECONDS_FROM_1970_TO_2000 + secondstime));
}

file32_def_t SensorTime::getFile32DefData(DateTime date) {
    file32_def_t fileDefDap = SensorTime::getFile32Def(date, FILE_FORMAT_DATA_PUBLISHABLE);
    if (ModuleCard::existsPath(fileDefDap.name)) {
#ifdef USE___SERIAL
        String fileNameDap = String(fileDefDap.name);
        Serial.printf("found fileDefDap: %s\n", fileNameDap.c_str());
#endif
        return fileDefDap;
    }
    file32_def_t fileDefDar = SensorTime::getFile32Def(date, FILE_FORMAT_DATA____ARCHIVED);
    if (ModuleCard::existsPath(fileDefDar.name)) {
#ifdef USE___SERIAL
        String fileNameDar = String(fileDefDar.name);
        Serial.printf("found fileDefDar: %s\n", fileNameDar.c_str());
#endif
        return fileDefDar;
    }
#ifdef USE___SERIAL
    String fileNameDar = String(fileDefDar.name);
    Serial.printf("found neither fileDefDap nor fileDefDar\n");
#endif
    return FILE_DEF_EMPTY;
}

bool SensorTime::isPersistPath(String path) {
    String persistPath = SensorTime::getFile32Def(SensorTime::getSecondstime() - SECONDS_PER_____________HOUR, FILE_FORMAT_DATA_PUBLISHABLE).name;
#ifdef USE___SERIAL
    Serial.printf("persistPath: %s, path: %s\n", persistPath.c_str(), path.c_str());
#endif
    return persistPath.indexOf(path) >= 0;
}

String SensorTime::getDateTimeSecondsString(uint32_t secondstime) {
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + secondstime);
    char timeBuffer[32];
    sprintf(timeBuffer, "%04d.%02d.%02d %02d:%02d:%02d", date.year(), date.month(), date.day(), date.hour(), date.minute(), date.second());
    return timeBuffer;
}

String SensorTime::getDateTimeDisplayString(uint32_t secondstime) {
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + secondstime);
    char timeBuffer[32];
    sprintf(timeBuffer, "%02d.%02d.%04d,%02d:%02d", date.day(), date.month(), date.year(), date.hour(), date.minute());
    return timeBuffer;
}

String SensorTime::getDateTimeLastModString(File32 file) {
    uint16_t pdate;
    uint16_t ptime;
    file.getModifyDateTime(&pdate, &ptime);
    DateTime modifyDate(FS_YEAR(pdate), FS_MONTH(pdate), FS_DAY(pdate), FS_HOUR(ptime), FS_MINUTE(ptime), FS_SECOND(ptime));
    return SensorTime::getDateTimeLastModString(modifyDate.secondstime());
}

String SensorTime::getDateTimeLastModString(uint32_t secondstime) {
    DateTime dateUtc = DateTime(SECONDS_FROM_1970_TO_2000 + secondstime - SensorTime::secondstimeOffsetUtc);
    char timeBuffer[32];
    sprintf(timeBuffer, "%s, %02d %s %04d %02d:%02d:%02d GMT", SensorTime::getNameOfDay(dateUtc), dateUtc.day(), SensorTime::getNameOfMonth(dateUtc), dateUtc.year(), dateUtc.hour(), dateUtc.minute(), dateUtc.second());
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

/**
 * provides a datetime callback function so dat files can be written with correct timestamps
 * used in ModuleSdCard::begin(...)
 */
void SensorTime::dateTimeCallback(uint16_t* date, uint16_t* time) {
    DateTime now = SensorTime::baseSensor.now();
    *date = FAT_DATE(now.year(), now.month(), now.day());
    *time = FAT_TIME(now.hour(), now.minute(), now.second());
}
