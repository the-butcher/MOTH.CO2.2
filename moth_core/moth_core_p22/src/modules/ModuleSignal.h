#ifndef ModuleSignal_h
#define ModuleSignal_h

#include <driver/rtc_io.h>

#include "types/Define.h"

typedef enum : uint32_t {
    COLOR____WHITE = 0x333333,
    COLOR______RED = 0xFF0000,
    COLOR____GREEN = 0x00FF00,
    COLOR___YELLOW = 0x333300,
    COLOR_____BLUE = 0x0000FF,
    COLOR_____CYAN = 0x003333,
    COLOR__MAGENTA = 0x660066,
    COLOR____BLACK = 0x000000,
    COLOR_____GRAY = 0x060606
} color_t;

const int BUZZER____FREQ_LO = 1000;  // 3755
const int BUZZER____CHANNEL = 0;
const int BUZZER_RESOLUTION = 8;  // 0 - 255
const int BUZZER_______GPIO = GPIO_NUM_17;

class ModuleSignal {
   private:
    static color_t pixelColor;

   public:
    static void begin();
    static void beep();
    static void prepareSleep();
    static color_t getPixelColor();
    static void setPixelColor(color_t pixelColor);
};

#endif