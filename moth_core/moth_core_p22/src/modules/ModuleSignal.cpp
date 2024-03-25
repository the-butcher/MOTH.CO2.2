#include <Arduino.h>

#include "types/Action.h"

#ifdef USE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
#endif

#include "modules/ModuleSignal.h"

#ifdef USE_NEOPIXEL
Adafruit_NeoPixel pixels(1, GPIO_NUM_33, NEO_GRB + NEO_KHZ800);
#endif

color_t ModuleSignal::pixelColor = COLOR____BLACK;

void ModuleSignal::begin() {
#ifdef USE_NEOPIXEL
    pixels.begin();
#else
    // de-power the neopixel
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, LOW);
    rtc_gpio_deinit((gpio_num_t)NEOPIXEL_POWER);
#endif
}

void ModuleSignal::prepareSleep() {
#ifdef USE_NEOPIXEL
    gpio_hold_en((gpio_num_t)NEOPIXEL_POWER);
#else
    gpio_hold_dis((gpio_num_t)NEOPIXEL_POWER);
#endif
}

color_t ModuleSignal::getPixelColor() {
    return ModuleSignal::pixelColor;
}

void ModuleSignal::setPixelColor(color_t pixelColor) {
    ModuleSignal::pixelColor = pixelColor;
#ifdef USE_NEOPIXEL
    pixels.setPixelColor(0, ModuleSignal::pixelColor);  // red for measuring
    pixels.show();
#endif
}
