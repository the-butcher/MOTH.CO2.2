#include "BoxTime.h"

uint32_t BoxTime::MICROSECONDS_PER______SECOND = 1000000;
uint32_t BoxTime::MICROSECONDS_PER_MILLISECOND = 1000;
uint32_t BoxTime::MILLISECONDS_PER______SECOND = 1000;
uint32_t BoxTime::WAITTIME________________NONE = 0;
uint32_t BoxTime::WAITTIME_DISPLAY_AND_DEPOWER = 5;  // very conservative estimation, 3 or maybe even 2 could also work

void BoxTime::begin() {
    this->baseTime.begin();
}

DateTime BoxTime::getDate() {
    return baseTime.now();
}

file32_def_t BoxTime::getFile32Def(uint32_t secondstime) {
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + secondstime);
    char pathBuffer[10];
    sprintf(pathBuffer, "/%04d/%02d", date.year(), date.month());
    char nameBuffer[22];
    sprintf(nameBuffer, "/%04d/%02d/%04d%02d%02d.csv", date.year(), date.month(), date.year(), date.month(), date.day());
    return {String(pathBuffer), String(nameBuffer)};
}

String BoxTime::getDateTimeDisplayString(uint32_t secondstime) {
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + secondstime);
    char timeBuffer[32];
    sprintf(timeBuffer, "%02d.%02d.%04d,%02d:%02d", date.day(), date.month(), date.year(), date.hour(), date.minute());
    return timeBuffer;
}
