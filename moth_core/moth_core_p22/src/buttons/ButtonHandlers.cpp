#include "ButtonHandlers.h"

void ButtonHandlers::begin() {
    ext1Bitmask = 1ULL << A.gpin | 1ULL << B.gpin | 1ULL << C.gpin;
    A.begin();
    B.begin();
    C.begin();
}

void ButtonHandlers::prepareSleep(bool isExt1Wakeup) {
    A.prepareSleep(isExt1Wakeup);
    B.prepareSleep(isExt1Wakeup);
    C.prepareSleep(isExt1Wakeup);
    if (isExt1Wakeup) {
        esp_sleep_enable_ext1_wakeup(ext1Bitmask, ESP_EXT1_WAKEUP_ANY_LOW);
    }
}

bool ButtonHandlers::isAnyPressed() {
    return A.isPressed() || B.isPressed() || C.isPressed();
}

bool ButtonHandlers::handleWakeupPin(gpio_num_t pin) {
    if (pin == A.gpin) {
        return true;
    } else if (pin == B.gpin) {
        return true;
    } else if (pin == C.gpin) {
        return true;
    } else {
        return false;
    }
}
