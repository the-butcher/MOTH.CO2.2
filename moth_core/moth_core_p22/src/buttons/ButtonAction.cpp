#include "ButtonAction.h"

void ButtonAction::begin() {
    ext1Bitmask = 1ULL << A.gpin | 1ULL << B.gpin | 1ULL << C.gpin;
    A.begin();
    B.begin();
    C.begin();
}

void ButtonAction::prepareSleep(bool isExt1Wakeup) {
    A.prepareSleep(isExt1Wakeup);
    B.prepareSleep(isExt1Wakeup);
    C.prepareSleep(isExt1Wakeup);
    if (isExt1Wakeup) {
        esp_sleep_enable_ext1_wakeup(ext1Bitmask, ESP_EXT1_WAKEUP_ANY_LOW);
    }
}

gpio_num_t ButtonAction::getPressedPin() {
    if (A.isPressed()) {
        return A.gpin;
    } else if (B.isPressed()) {
        return B.gpin;
    } else if (C.isPressed()) {
        return C.gpin;
    }
    return GPIO_NUM_0;
}

bool ButtonAction::toggleDisplayValTFw(config_t *config) {
    // TODO ::
}
