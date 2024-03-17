#ifndef BoxBeep_h
#define BoxBeep_h

#include <driver/rtc_io.h>

typedef enum {
    COLOR____WHITE = 0x333333,
    COLOR______RED = 0xFF0000,
    COLOR___YELLOW = 0x333300,
    COLOR_____BLUE = 0x0000FF,
    COLOR_____CYAN = 0x003333,
    COLOR__MAGENTA = 0x660066,
    COLOR____BLACK = 0x000000,
    COLOR_____GRAY = 0x060606
} color_t;

class BoxBeep {
   private:
    color_t pixelColor;

   public:
    BoxBeep();
    void begin();
    void prepareSleep();
    color_t getPixelColor();
    void setPixelColor(color_t pixelColor);
};

#endif