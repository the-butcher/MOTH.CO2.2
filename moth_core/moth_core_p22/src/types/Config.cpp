#include "types/Config.h"

#include <Arduino.h>

#include "sensors/SensorTime.h"

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
            DISPLAY_VAL_S__ENTRY,  // entry screen first
            DISPLAY_VAL_M__TABLE,  // entry -> chart | table
            DISPLAY_VAL_T____CO2,  // value shown when rendering a measurement (table)
            DISPLAY_VAL_C____ALT,  // value shown when rendering the chart
            DISPLAY_HRS_C_____01,  // hours to be shown in chart display
            DISPLAY_THM____LIGHT,  // light theme
            DISPLAY_VAL_D______C,  // temperature scale
            DISPLAY_VAL_Y____SIG,
            3  // display update minutes
        },
        {
            WIFI____VAL_P_CUR_N,  // initial wifi state off
            10,                   // network expiry minutes
            0                     // last successful connection index
        },
        {
            360  // ntp update interval, 6 hours
            // [static timezone char array [64]]
        },
        {
            1.5,                  // temperature offset
            SCO2___VAL_M___IDLE,  // sensor mode (power cycled, against recommendations, idle, when turning off i2c which may be a hidden power cylce, the sensor calibrates, which it does not when power cycled)
            0,                    // no calibration
            false                 // no reset
        },
        {
            SIGNAL__VAL_____OFF  // warn signal
        },
        0.0,  // calculated sealevel pressure, 0.0 = needs recalculation
        153   // the altitude that the sensor was configured to (or set to by the user)
    };
    String timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
    timezone.toCharArray(config.time.timezone, 64);
    return config;
}
