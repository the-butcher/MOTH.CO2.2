#include "BoxTime.h"

uint32_t BoxTime::MICROSECONDS_PER______SECOND = 1000000;
uint32_t BoxTime::MICROSECONDS_PER_MILLISECOND = 1000;
uint32_t BoxTime::MILLISECONDS_PER______SECOND = 1000;
uint32_t BoxTime::WAITTIME________________NONE = 0;
uint32_t BoxTime::WAITTIME_DISPLAY_AND_DEPOWER = 5;  // very conservative

void BoxTime::begin() {
    this->baseTime.begin();
}

DateTime BoxTime::getDate() {
    return baseTime.now();
}

String BoxTime::getDateTimeDisplayString(uint32_t secondstime) {
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + secondstime);
    char timeBuffer[32];
    sprintf(timeBuffer, "%02d.%02d.%04d,%02d:%02d", date.day(), date.month(), date.year(), date.hour(), date.minute());
    return timeBuffer;
}
