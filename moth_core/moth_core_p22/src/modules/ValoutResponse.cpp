#include "ValoutResponse.h"

#include "ModuleCard.h"
#include "ModuleHttp.h"
#include "types/Define.h"

ValoutResponse::~ValoutResponse() {
    // nothing
}

ValoutResponse::ValoutResponse() : AsyncAbstractResponse() {

    ModuleCard::begin();

    lineLimit = min((uint32_t)Values::values->nextMeasureIndex, (uint32_t)MEASUREMENT_BUFFER_SIZE);

    _code = 200;
    _contentLength = lineLimit * sizeof(values_all_t);
    _contentType = "application/octet-stream";

    int maxAge = SECONDS_PER___________MINUTE + 5 - SensorTime::getSecondstime() % SECONDS_PER___________MINUTE;  // 5 seconds to reflect measurement times
    char maxAgeBuf[16];
    sprintf(maxAgeBuf, "max-age=%s", String(maxAge));
    addHeader("Cache-Control", maxAgeBuf);
}

size_t ValoutResponse::_fillBuffer(uint8_t *data, size_t maxLen) {
    uint16_t offset;
    values_all_t datValue;
    uint32_t dataIndex;
    for (uint32_t lineIndex = 0; lineIndex < lineLimit; lineIndex++) {
        dataIndex = lineIndex + Values::values->nextMeasureIndex - lineLimit;
        datValue = Values::values->measurements[dataIndex % MEASUREMENT_BUFFER_SIZE];
        offset = lineIndex * sizeof(values_all_t);
        byte *valueData = (byte *)&datValue;
        for (uint8_t dataIndex = 0; dataIndex < sizeof(values_all_t); dataIndex++) {
            data[offset + dataIndex] = valueData[dataIndex];
        }
    }
    return sizeof(values_all_t) * lineLimit;
}
