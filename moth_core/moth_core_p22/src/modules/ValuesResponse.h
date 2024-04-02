#ifndef ValuesResponse_h
#define ValuesResponse_h

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SdFat.h>

#include "types/Values.h"

// from https://github.com/me-no-dev/ESPAsyncWebServer/blob/master/src/WebResponseImpl.h

class ValuesResponse : public AsyncAbstractResponse {
   private:
    bool firstFill;
    uint8_t lineLimit;

   public:
    ValuesResponse();
    ~ValuesResponse();
    bool _sourceValid() const {
        return true;
    }
    virtual size_t _fillBuffer(uint8_t* buf, size_t maxLen) override;
};

#endif