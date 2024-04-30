#ifndef ValoutResponse_h
#define ValoutResponse_h

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SdFat.h>

#include "types/Values.h"

// from https://github.com/me-no-dev/ESPAsyncWebServer/blob/master/src/WebResponseImpl.h

class ValoutResponse : public AsyncAbstractResponse {
   private:
    uint8_t lineLimit;

   public:
    ValoutResponse();
    ~ValoutResponse();
    bool _sourceValid() const {
        return true;
    }
    virtual size_t _fillBuffer(uint8_t* buf, size_t maxLen) override;
};

#endif