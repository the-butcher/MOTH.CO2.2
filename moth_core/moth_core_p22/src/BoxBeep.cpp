#define USE_NEOPIXEL = 1;
#include <Arduino.h>

#ifdef USE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
#endif

#include "BoxBeep.h"

#ifdef USE_NEOPIXEL
Adafruit_NeoPixel pixels(1, GPIO_NUM_33, NEO_GRB + NEO_KHZ800);
#endif

BoxBeep::BoxBeep() {
    this->pixelColor = COLOR____BLACK;
}

void BoxBeep::begin() {
#ifdef USE_NEOPIXEL
    pixels.begin();
#else
    // de-power the neopixel
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, LOW);
    rtc_gpio_deinit((gpio_num_t)NEOPIXEL_POWER);
#endif
}

void BoxBeep::prepareSleep() {
#ifdef USE_NEOPIXEL
    gpio_hold_en((gpio_num_t)NEOPIXEL_POWER);
#else
    gpio_hold_dis((gpio_num_t)NEOPIXEL_POWER);
#endif
}

color_t BoxBeep::getPixelColor() {
    return BoxBeep::pixelColor;
}

void BoxBeep::setPixelColor(color_t pixelColor) {
    BoxBeep::pixelColor = pixelColor;
#ifdef USE_NEOPIXEL
    pixels.setPixelColor(0, BoxBeep::pixelColor);  // red for measuring
    pixels.show();
    delay(100);
#endif
}
