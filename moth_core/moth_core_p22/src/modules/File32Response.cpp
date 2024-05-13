#include "File32Response.h"

#include "ModuleCard.h"

File32Response::~File32Response() {
    if (_content) {
        _content.close();
    }
}

/**
 * response wrapper around the content of a File32
 */
File32Response::File32Response(String path, String contentType) : AsyncAbstractResponse() {

    ModuleCard::begin();

    _code = 200;
    _path = path;
    _content.open(_path.c_str(), O_RDONLY);
    _contentLength = _content.size();
    _contentType = contentType;

    lastModified = SensorTime::getDateTimeLastModString(_content);

    if (path.indexOf(".dat") > 0) {
        String dayPath = SensorTime::getFile32Def(SensorTime::getSecondstime(), "dat").name;  // slash in first char pos
        if (dayPath.indexOf(path) < 0) {
            addHeader("Cache-Control", "max-age=31536000");
        } else {
            uint8_t measurementsUntilUpdate = MEASUREMENT_BUFFER_SIZE - Values::values->nextMeasureIndex % MEASUREMENT_BUFFER_SIZE;
            char maxAgeBuf[16];
            sprintf(maxAgeBuf, "max-age=%s", String(measurementsUntilUpdate * SECONDS_PER___________MINUTE));
            addHeader("Cache-Control", maxAgeBuf);
        }
    } else {
        addHeader("Cache-Control", "no-cache");
    }
    addHeader("Last-Modified", lastModified);
}

bool File32Response::wasModifiedSince(String ifModifiedSince) {
    return ifModifiedSince != lastModified;
}

size_t File32Response::_fillBuffer(uint8_t *data, size_t len) {
    return _content.read(data, len);
}
