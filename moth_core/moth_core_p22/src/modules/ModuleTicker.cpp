#include "ModuleTicker.h"

RTC_PCF8523 ModuleTicker::baseTime;

void ModuleTicker::begin() {
    ModuleTicker::baseTime.begin();
}

uint32_t ModuleTicker::getSecondstime() {
    return ModuleTicker::baseTime.now().secondstime();
}

file32_def_t ModuleTicker::getFile32Def(uint32_t secondstime, String fileFormat) {
    return ModuleTicker::getFile32Def(DateTime(SECONDS_FROM_1970_TO_2000 + secondstime), fileFormat);
}

file32_def_t ModuleTicker::getFile32Def(DateTime date, String fileFormat) {
    char pathBuffer[10];
    sprintf(pathBuffer, "/%04d/%02d", date.year(), date.month());
    char nameBuffer[22];
    sprintf(nameBuffer, "/%04d/%02d/%04d%02d%02d.%s", date.year(), date.month(), date.year(), date.month(), date.day(), fileFormat);
    return {String(pathBuffer), String(nameBuffer)};
}

String ModuleTicker::getDateTimeDisplayString(uint32_t secondstime) {
    DateTime date = DateTime(SECONDS_FROM_1970_TO_2000 + secondstime);
    char timeBuffer[32];
    sprintf(timeBuffer, "%02d.%02d.%04d,%02d:%02d", date.day(), date.month(), date.year(), date.hour(), date.minute());
    return timeBuffer;
}
