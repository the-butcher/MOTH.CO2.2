#include "ButtonHandlers.h"

// gpio_num_t ButtonHandlers::wakeupPin;
// std::function<bool(button_t)> ButtonHandlers::wakeupCallback = nullptr;

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

gpio_num_t ButtonHandlers::getActionPin() {
    if (A.isPressed()) {
        return A.gpin;
    } else if (B.isPressed()) {
        return B.gpin;
    } else if (C.isPressed()) {
        return C.gpin;
    }
    return GPIO_NUM_0;
}

// bool ButtonHandlers::isTaskActive() {
//     return ButtonHandlers::wakeupCallback != nullptr;
// }

// void ButtonHandlers::handleButtonWakeup(gpio_num_t wakeupPin, std::function<bool(button_t)> wakeupCallback) {
//     ButtonHandlers::wakeupPin = wakeupPin;
//     ButtonHandlers::wakeupCallback = wakeupCallback;
//     xTaskCreate(ButtonHandlers::buttonReleaseVTask,  // Function that should be called
//                 "handle button wakeup",              // Name of the task (for debugging)
//                 1000,                                // Stack size (bytes)
//                 NULL,                                // Parameter to pass
//                 2,                                   // Task priority
//                 NULL                                 // Task handle
//     );
// }

// /**
//  * checks the given pin state repeatedly and
//  */
// void ButtonHandlers::buttonReleaseVTask(void* parameter) {
//     uint64_t millisA = millis();
//     while (millis() - millisA < 1000) {
//         if (digitalRead(wakeupPin) == HIGH) {  // already released
//             ButtonHandlers::wakeupCallback({wakeupPin, BUTTON_FAST});
//             ButtonHandlers::wakeupCallback = nullptr;
//             vTaskDelete(NULL);
//             return;
//         }
//         vTaskDelay(50 / portTICK_PERIOD_MS);
//     }
//     ButtonHandlers::wakeupCallback({wakeupPin, BUTTON_SLOW});
//     ButtonHandlers::wakeupCallback = nullptr;
//     vTaskDelete(NULL);
//     return;
// }
