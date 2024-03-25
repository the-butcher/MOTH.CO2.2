#ifndef Config_h
#define Config_h

#include <Arduino.h>

typedef struct {
    uint16_t wHi;  // upper warn level
    uint16_t rHi;  // upper risk level
    uint16_t ref;  // co2 reference value (i.e. 425)
} thresholds_co2_t;

typedef struct {
    uint8_t rLo;  // lower risk level
    uint8_t wLo;  // lower warn level
    uint8_t wHi;  // upper warn level
    uint8_t rHi;  // upper risk level
} thresholds_deg_t;

typedef struct {
    uint8_t rLo;  // lower risk level
    uint8_t wLo;  // lower warn level
    uint8_t wHi;  // upper warn level
    uint8_t rHi;  // upper risk level
} thresholds_hum_t;

/**
 * render type of display (CHART | TABLE)
 */
typedef enum {
    DISPLAY_VAL_M_TABLE,
    DISPLAY_VAL_M_CHART
} display_val_m_e;

/**
 * current display value in table modus
 */
typedef enum {
    DISPLAY_VAL_T___CO2,
    DISPLAY_VAL_T___HPA,
    DISPLAY_VAL_T___ALT
} display_val_t_e;

/**
 * current display value in chart modus
 */
typedef enum {
    DISPLAY_VAL_C___CO2,
    DISPLAY_VAL_C___DEG,
    DISPLAY_VAL_C___HUM,
    DISPLAY_VAL_C___HPA,
    DISPLAY_VAL_C___ALT
} display_val_c_e;

typedef enum {
    DISPLAY_HRS_C____01 = 1,
    DISPLAY_HRS_C____03 = 3,
    DISPLAY_HRS_C____06 = 6,
    DISPLAY_HRS_C____12 = 12,
    DISPLAY_HRS_C____24 = 24
} display_hrs_c_e;

typedef enum {
    DISPLAY_THM___LIGHT,
    DISPLAY_THM____DARK
} display_thm___e;

typedef enum {
    DISPLAY_DEG_FAHRENH,
    DISPLAY_DEG_CELSIUS
} display_deg___e;

typedef struct {
    thresholds_co2_t thresholdsCo2;
    thresholds_deg_t thresholdsDeg;
    thresholds_hum_t thresholdsHum;
    uint8_t displayUpdateMinutes;
    display_val_m_e displayValModus;
    display_val_t_e displayValTable;
    display_val_c_e displayValChart;
    display_hrs_c_e displayHrsChart;
    display_thm___e displayValTheme;
    display_deg___e displayDegScale;
} disp____all___t;

typedef struct {
    bool powered;                  // the status that the wifi should have, by button action
    uint8_t networkExpiryMinutes;  // minute without activity before the network times out
    int8_t networkConnIndexLast;   // the index of the last network a connection was established to
} wifi____all___t;

typedef struct {
    char timezone[64];
} time____all___t;

typedef struct {
    disp____all___t disp;
    wifi____all___t wifi;
    time____all___t time;
    bool isBeep;
    float temperatureOffset;
    float pressureZerolevel;  // calculated sealevel pressure
    float altitudeBaselevel;  // the altitude that the seonsor was configured to (or set by the user)
} config_t;

class Config {
   private:
   public:
    static config_t load();
};

#endif