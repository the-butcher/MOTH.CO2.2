#ifndef StringPrint_h
#define StringPrint_h

#include <Print.h>

class StringPrint : public Print {

  public:
      StringPrint(String &s) : string(s) { }
      virtual size_t write(uint8_t c) { string += (char)c; return 1;};

  private:
      String &string;

};

#endif