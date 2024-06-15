#ifndef Network_h
#define Network_h

#include <Arduino.h>
#include "WiFi.h"

typedef struct {
    String ssid;
    String pass;
    int rssi;
    wifi_auth_mode_t encr;
} Network;

#endif