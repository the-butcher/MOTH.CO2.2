#include "BoxFiles.h"
#include "File32Response.h"

File32Response::~File32Response(){
  if(_content) {
    _content.close();
  }
}

/**
 * response wrapper around the content of a File32
 */
File32Response::File32Response(String path, String contentType): AsyncAbstractResponse(){
  _code = 200;
  _path = path;
  _content.open(_path.c_str(), O_RDONLY);
  _contentLength = _content.size();
  _contentType = contentType;
}

size_t File32Response::_fillBuffer(uint8_t *data, size_t len) {
  return _content.read(data, len);
}

