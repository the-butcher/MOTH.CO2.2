#include "types/Config.h"

#include <Arduino.h>

#include "modules/ModuleClock.h"

config_t Config::load() {
    config_t config = {
        {
            {
                800,   // co2 wHi
                1000,  // co2 rHi
                425    // co2 ref
            },
            {
                14,  // deg rLo
                19,  // deg wLo
                25,  // deg wHi
                30   // deg rHi
            },
            {
                25,  // hum rLo
                30,  // hum wLo
                60,  // hum wHi
                65   // hum rHi
            },
            3,                    // display update minutes
            DISPLAY_VAL_M_TABLE,  // chart | table
            DISPLAY_VAL_T___CO2,  // value shown when rendering a measurement (table)
            DISPLAY_VAL_C___CO2,  // value shown when rendering the chart
            DISPLAY_HRS_C____24,  // hours to be shown in chart display
            DISPLAY_THM___LIGHT,  // light theme
            DISPLAY_DEG_CELSIUS,  // temperature scale
        },
        {

        },
        {
            false,
            5,  // network expiry minutes
            0   // last successful connection index
        },
        false,  // beep
        1.5,    // temperature offset
        0.0,    // calculated sealevel pressure, 0.0 = needs recalculation
        153     // the altitude that the seonsor was configured to (or set by the user)
    };
    String timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
    timezone.toCharArray(config.time.timezone, 64);
    return config;
}
