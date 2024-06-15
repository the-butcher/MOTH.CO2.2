#ifndef ButtonHandlers_h
#define ButtonHandlers_h

#include "Arduino.h"
#include "LoopReason.h"
#include "ButtonHandler.h"

class ButtonHandlers {
  
  private:
  
  public:
    static ButtonHandler A;
    static ButtonHandler B;
    static ButtonHandler C;
    static void begin();
    static void assignChartHours();
    static void assignWifiAndPms();
    static void assignAltitudeModifiers();
    static void assignThemeAndState();

};

#endif