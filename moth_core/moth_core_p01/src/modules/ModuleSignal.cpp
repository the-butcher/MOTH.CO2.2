#include <Arduino.h>

#ifdef USE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
#endif

#include "modules/ModuleSignal.h"

#ifdef USE_NEOPIXEL
Adafruit_NeoPixel pixels(1, GPIO_NUM_33, NEO_GRB + NEO_KHZ800);
#endif

color_t ModuleSignal::pixelColor = COLOR____BLACK;

void ModuleSignal::begin() {
    ledcSetup(BUZZER____CHANNEL, BUZZER____FREQ_LO, BUZZER_RESOLUTION);
    ledcAttachPin(BUZZER_______GPIO, BUZZER____CHANNEL);
#ifdef USE_NEOPIXEL
    pixels.begin();
#else
    // de-power the neopixel
    rtc_gpio_deinit((gpio_num_t)NEOPIXEL_POWER);
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, LOW);
#endif
}

void ModuleSignal::beep(uint16_t frequency) {
    ledcWrite(BUZZER____CHANNEL, 180);
    ledcWriteTone(BUZZER____CHANNEL, frequency);
    delay(50);
    ledcWrite(BUZZER____CHANNEL, 0);
}

void ModuleSignal::beep() {
    ModuleSignal::beep(BUZZER____FREQ_LO);
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
