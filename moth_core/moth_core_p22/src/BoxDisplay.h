#ifndef BoxDisplay_h
#define BoxDisplay_h

#include <Adafruit_EPD.h>
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <Fonts/smb06pt_b.h>
#include <Fonts/smb06pt_d.h>
#include <Fonts/smb06pt_l.h>
#include <Fonts/smb08pt_b.h>
#include <Fonts/smb08pt_d.h>
#include <Fonts/smb08pt_l.h>
#include <Fonts/smb18pt_b.h>
#include <Fonts/smb18pt_d.h>
#include <Fonts/smb18pt_l.h>
#include <Fonts/smb36pt_b.h>
#include <Fonts/smb36pt_d.h>
#include <Fonts/smb36pt_l.h>

#include "BoxDisplayBase.h"

class BoxDisplay {
   private:
    BoxDisplayBase baseDisplay;
    void drawAntialiasedText06(String text, int xRel, int yRel, uint16_t color);
    void drawAntialiasedText08(String text, int xRel, int yRel, uint16_t color);
    void drawAntialiasedText18(String text, int xRel, int yRel, uint16_t color);
    void drawAntialiasedText36(String text, int xRel, int yRel, uint16_t color);
    void drawAntialiasedText(String text, int xRel, int yRel, uint16_t color, const GFXfont *fontL, const GFXfont *fontD, const GFXfont *fontB);
    void clearBuffer();
    void flushBuffer();

   public:
    void begin();
    void renderTest(String value1, String value2, String value3);
    void hibernate();
};

#endif