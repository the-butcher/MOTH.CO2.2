#ifndef File32Response_h
#define File32Response_h

#include <Arduino.h>
#include <SdFat.h>

#include "ESPAsyncWebServer.h"

// from https://github.com/me-no-dev/ESPAsyncWebServer/blob/master/src/WebResponseImpl.h

class File32Response : public AsyncAbstractResponse {
   private:
    File32 _content;
    String _path;

   public:
    File32Response(String path, String contentType);
    ~File32Response();
    bool _sourceValid() const {
        return !!(_content);
    }
    virtual size_t _fillBuffer(uint8_t *buf, size_t maxLen) override;
};

#endif