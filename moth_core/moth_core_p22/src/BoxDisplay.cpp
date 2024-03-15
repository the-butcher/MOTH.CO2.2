#include "BoxDisplay.h"

void BoxDisplay::begin() {
}

void BoxDisplay::flushBuffer() {
    // >> flushBuffer
    baseDisplay.writeFrameBuffers();
    // << flushBuffer
}

void BoxDisplay::hibernate() {
    baseDisplay.begin(THINKINK_GRAYSCALE4, true);
    baseDisplay.hibernate();
}

void BoxDisplay::clearBuffer() {
    baseDisplay.begin(THINKINK_GRAYSCALE4, true);
    baseDisplay.clearBuffer();
}

void BoxDisplay::renderTest(String value1, String value2, String value3) {
    BoxDisplay::clearBuffer();
    drawAntialiasedText08(value1, 20, 20, EPD_BLACK);
    drawAntialiasedText08(value2, 20, 60, EPD_BLACK);
    drawAntialiasedText08(value3, 20, 100, EPD_BLACK);
    BoxDisplay::flushBuffer();
}

void BoxDisplay::drawAntialiasedText06(String text, int xRel, int yRel, uint16_t color) {
    drawAntialiasedText(text, xRel, yRel, color, &smb06pt_l, &smb06pt_d, &smb06pt_b);
}

void BoxDisplay::drawAntialiasedText08(String text, int xRel, int yRel, uint16_t color) {
    drawAntialiasedText(text, xRel, yRel, color, &smb08pt_l, &smb08pt_d, &smb08pt_b);
}

void BoxDisplay::drawAntialiasedText18(String text, int xRel, int yRel, uint16_t color) {
    drawAntialiasedText(text, xRel, yRel, color, &smb18pt_l, &smb18pt_d, &smb18pt_b);
}

void BoxDisplay::drawAntialiasedText36(String text, int xRel, int yRel, uint16_t color) {
    drawAntialiasedText(text, xRel, yRel, color, &smb36pt_l, &smb36pt_d, &smb36pt_b);
}

/**
 * when switching fonts, setFont(...) must be called before setCursor(...), or there may be a y-offset on the very first text
 */
void BoxDisplay::drawAntialiasedText(String text, int xRel, int yRel, uint16_t color, const GFXfont *fontL, const GFXfont *fontD, const GFXfont *fontB) {
    baseDisplay.setFont(fontL);
    baseDisplay.setCursor(xRel, yRel);
    baseDisplay.setTextColor(color == EPD_BLACK ? EPD_LIGHT : EPD_DARK);
    baseDisplay.print(text);

    baseDisplay.setFont(fontD);
    baseDisplay.setCursor(xRel, yRel);
    baseDisplay.setTextColor(color == EPD_BLACK ? EPD_DARK : EPD_LIGHT);
    baseDisplay.print(text);

    baseDisplay.setFont(fontB);
    baseDisplay.setCursor(xRel, yRel);
    baseDisplay.setTextColor(color);
    baseDisplay.print(text);
}
