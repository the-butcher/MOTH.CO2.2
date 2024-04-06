#include "File32Response.h"

#include "ModuleSdcard.h"

File32Response::~File32Response() {
#ifdef USE___SERIAL
    Serial.println("closing File32Response ...");
#endif
    if (_content) {
        _content.close();
    }
}

/**
 * response wrapper around the content of a File32
 */
File32Response::File32Response(String path, String contentType) : AsyncAbstractResponse() {
#ifdef USE___SERIAL
    Serial.println("opening File32Response ...");
#endif
    ModuleSdcard::begin();
    _code = 200;
    _path = path;
    _content.open(_path.c_str(), O_RDONLY);
    _contentLength = _content.size();
    _contentType = contentType;

    lastModified = SensorTime::getDateTimeLastModString(_content);
    addHeader("Last-Modified", SensorTime::getDateTimeLastModString(_content));
}

bool File32Response::wasModifiedSince(String ifModifiedSince) {
    return ifModifiedSince != lastModified;
}

size_t File32Response::_fillBuffer(uint8_t *data, size_t len) {
    return _content.read(data, len);
}
