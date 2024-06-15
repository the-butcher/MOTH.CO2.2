#ifndef DataResponse_h
#define DataResponse_h
#include <Arduino.h>
#include "ESPAsyncWebServer.h"

// from https://github.com/me-no-dev/ESPAsyncWebServer/blob/master/src/WebResponseImpl.h

class DataResponse: public AsyncAbstractResponse {

  private:
    int _offsetIndex;

  public:
    DataResponse();
    ~DataResponse();
    bool _sourceValid() const { return true; }
    virtual size_t _fillBuffer(uint8_t *buf, size_t maxLen) override;
    
};

#endif