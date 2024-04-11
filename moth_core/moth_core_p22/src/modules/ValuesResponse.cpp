#include "ValuesResponse.h"

#include "ModuleCard.h"
#include "ModuleHttp.h"
#include "types/Define.h"

ValuesResponse::~ValuesResponse() {
    // nothing
}

ValuesResponse::ValuesResponse() : AsyncAbstractResponse() {

    ModuleCard::begin();

    lineLimit = min((uint32_t)Values::values->nextMeasureIndex, (uint32_t)MEASUREMENT_BUFFER_SIZE);
    firstFill = true;

    _code = 200;
    _contentLength = CSV_HEAD.length() + lineLimit * CSV_LINE_LENGTH;
    _contentType = "text/csv";

    int maxAge = SECONDS_PER___________MINUTE + 5 - SensorTime::getSecondstime() % SECONDS_PER___________MINUTE;  // 5 seconds to reflect the csv times
    char maxAgeBuf[16];
    sprintf(maxAgeBuf, "max-age=%s", String(maxAge));
    addHeader("Cache-Control", maxAgeBuf);
}

size_t ValuesResponse::_fillBuffer(uint8_t *data, size_t maxLen) {
    if (firstFill) {
        for (uint8_t charIndex = 0; charIndex < CSV_HEAD.length(); charIndex++) {
            data[charIndex] = CSV_HEAD[charIndex];
        }
        firstFill = false;
        return CSV_HEAD.length();
    } else {
        uint16_t offset;
        values_all_t datValue;
        uint32_t dataIndex;
        for (uint32_t lineIndex = 0; lineIndex < lineLimit; lineIndex++) {
            dataIndex = lineIndex + Values::values->nextMeasureIndex - lineLimit;
            // #ifdef USE___SERIAL
            //             Serial.printf("lineIndex: %d, next: %d, lineLimit: %d, dataIndex: %d\n", lineIndex, Values::values->nextMeasureIndex, lineLimit, dataIndex % MEASUREMENT_BUFFER_SIZE);
            // #endif
            datValue = Values::values->measurements[dataIndex % MEASUREMENT_BUFFER_SIZE];
            offset = lineIndex * CSV_LINE_LENGTH;
            ModuleHttp::fillBufferWithCsv(&datValue, data, offset);
        }
        return CSV_LINE_LENGTH * lineLimit;
    }
}
