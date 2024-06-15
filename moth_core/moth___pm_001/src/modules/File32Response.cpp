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
File32Response::File32Response(String path, String mimeType) : AsyncAbstractResponse() {

    ModuleCard::begin();

    _code = 200;
    _path = path;
    _content = LittleFS.open(_path.c_str(), FILE_READ);
    _contentLength = _content.size();
    _contentType = mimeType;

    lastModified = SensorTime::getDateTimeLastModString(_content);

    addHeader("Cache-Control", "no-cache");
    addHeader("Last-Modified", lastModified);

    if (mimeType == "text/html" || mimeType == "application/javascript") {
        addHeader("Content-Encoding", "gzip");
    }
}

bool File32Response::wasModifiedSince(String ifModifiedSince) {
    return ifModifiedSince != lastModified;
}

size_t File32Response::_fillBuffer(uint8_t *data, size_t len) {
    return _content.read(data, len);
}
