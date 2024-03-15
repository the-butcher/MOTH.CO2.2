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
