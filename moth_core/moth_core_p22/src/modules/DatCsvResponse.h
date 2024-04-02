#ifndef DatCsvResponse_h
#define DatCsvResponse_h

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SdFat.h>

#include "types/Values.h"

// from https://github.com/me-no-dev/ESPAsyncWebServer/blob/master/src/WebResponseImpl.h

class DatCsvResponse : public AsyncAbstractResponse {
   private:
    File32 _content;
    String _path;
    bool firstFill;

   public:
    DatCsvResponse(String path);
    ~DatCsvResponse();
    bool _sourceValid() const {
        return !!(_content);
    }
    virtual size_t _fillBuffer(uint8_t *buf, size_t maxLen) override;
};

#endif