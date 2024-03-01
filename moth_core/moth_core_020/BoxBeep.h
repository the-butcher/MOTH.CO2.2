#ifndef BoxBeep_h
#define BoxBeep_h

typedef enum {
  COLOR___WHITE = 0x666666, 
  COLOR_____RED = 0xFF0000,
  COLOR__YELLOW = 0x666600,
  COLOR____BLUE = 0x0000FF, 
  COLOR____CYAN = 0x006666, 
  COLOR_MAGENTA = 0x660066,
  COLOR___BLACK = 0x000000
} color_t;

class BoxBeep {
  
  private:
    static color_t pixelColor;
    
  public:
    static void begin();
    static void beep();
    static color_t getPixelColor();
    static void setPixelColor(color_t pixelColor);

};

#endif