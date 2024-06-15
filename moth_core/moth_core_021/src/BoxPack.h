#ifndef BoxPack_h
#define BoxPack_h

#include "ValuesBat.h"
#include "Adafruit_LC709203F.h"

class BoxPack {
  
  private:
    static Adafruit_LC709203F basePack;
    static bool isPowered();
    
  public:
    static void begin();
    static bool tryRead();
    static ValuesBat values;

};

#endif