// #define USE_NEOPIXEL = 1;

#include "BoxBeep.h"

#include "SensorPmsa003i.h"

#ifdef USE_NEOPIXEL
#include <Adafruit_NeoPixel.h>
#endif

/**
 * ################################################
 * ## constants
 * ################################################
 */
const int BUZZER____FREQ_LO = 1000;  // 3755
const int BUZZER____CHANNEL = 0;
const int BUZZER_RESOLUTION = 8;  // 0 - 255
const int BUZZER_______GPIO = SensorPmsa003i::ACTIVE ? GPIO_NUM_8 : GPIO_NUM_17;

color_t BoxBeep::pixelColor = COLOR___BLACK;
sound_t BoxBeep::sound = SOUND_OFF;

#ifdef USE_NEOPIXEL
Adafruit_NeoPixel pixels(1, GPIO_NUM_33, NEO_GRB + NEO_KHZ800);
#endif

void BoxBeep::begin() {
    ledcSetup(BUZZER____CHANNEL, BUZZER____FREQ_LO, BUZZER_RESOLUTION);
    ledcAttachPin(BUZZER_______GPIO, BUZZER____CHANNEL);
#ifdef USE_NEOPIXEL
    pixels.begin();
#else
    // de-power the neopixel
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, LOW);
#endif
}

void BoxBeep::beep() {
    ledcWrite(BUZZER____CHANNEL, 180);
    ledcWriteTone(BUZZER____CHANNEL, BUZZER____FREQ_LO);
    delay(50);
    ledcWrite(BUZZER____CHANNEL, 0);
}

color_t BoxBeep::getPixelColor() { return BoxBeep::pixelColor; }

void BoxBeep::setPixelColor(color_t pixelColor) {
    BoxBeep::pixelColor = pixelColor;
#ifdef USE_NEOPIXEL
    pixels.setPixelColor(0, BoxBeep::pixelColor);  // red for measuring
    pixels.show();
#endif
}

sound_t BoxBeep::getSound() { return BoxBeep::sound; }

void BoxBeep::toggleSound() {
    if (BoxBeep::sound == SOUND__ON) {
        BoxBeep::sound = SOUND_OFF;
    } else {
        BoxBeep::sound = SOUND__ON;
    }
}
