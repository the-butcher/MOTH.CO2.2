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
typedef enum : uint8_t {
    DISPLAY_VAL_M__TABLE,
    DISPLAY_VAL_M__CHART
} display_val_m_e;

typedef enum : uint8_t {
    DISPLAY_VAL_S__ENTRY,  // splash screen
    DISPLAY_VAL_S___NONE,  // ?
    DISPLAY_VAL_S_____QR,  // wifi qr codes
    DISPLAY_VAL_S____CO2   // calibration or reset
} display_val_s_e;

/**
 * current display value in table modus
 */
typedef enum : uint8_t {
    DISPLAY_VAL_T____CO2,
    DISPLAY_VAL_T____HPA,
    DISPLAY_VAL_T____ALT
} display_val_t_e;

/**
 * current display value in chart modus
 */
typedef enum : uint8_t {
    DISPLAY_VAL_C____CO2,
    DISPLAY_VAL_C____DEG,
    DISPLAY_VAL_C____HUM,
    DISPLAY_VAL_C____HPA,
    DISPLAY_VAL_C____ALT,
    DISPLAY_VAL_C____NRG
} display_val_c_e;

typedef enum : uint8_t {
    DISPLAY_HRS_C_____01 = 1,
    DISPLAY_HRS_C_____03 = 3,
    DISPLAY_HRS_C_____06 = 6,
    DISPLAY_HRS_C_____12 = 12,
    DISPLAY_HRS_C_____24 = 24
} display_val_h_e;

typedef enum : uint8_t {
    DISPLAY_THM____LIGHT,
    DISPLAY_THM_____DARK
} display_val_e_e;

typedef enum : uint8_t {
    DISPLAY_DEG__FAHRENH,
    DISPLAY_DEG__CELSIUS
} display_val_d_e;

typedef enum : uint8_t {
    DISPLAY_VAL_Y____FIX,  // update with fixed interval
    DISPLAY_VAL_Y____SIG   // update upon significant change
} display_val_y_e;

typedef struct {
    thresholds_co2_t thresholdsCo2;
    thresholds_deg_t thresholdsDeg;
    thresholds_hum_t thresholdsHum;
    display_val_s_e displayValSetng;
    display_val_m_e displayValModus;
    display_val_t_e displayValTable;
    display_val_c_e displayValChart;
    display_val_h_e displayHrsChart;
    display_val_e_e displayValTheme;
    display_val_d_e displayDegScale;
    display_val_y_e displayValCycle;
    uint8_t displayUpdateMinutes;
} disp____all___t;

// uint32_t st = sizeof(display_val_s_e);

typedef enum : uint8_t {
    WIFI____VAL_P_PND_Y,  // to be turned on
    WIFI____VAL_P_CUR_Y,  // turned on
    WIFI____VAL_P_CUR_N,  // turned off
    WIFI____VAL_P_PND_N   // to be turned off
} wifi____val_p_e;

typedef struct {
    wifi____val_p_e wifiValPower;  // the status that the wifi should have, by button action
    uint8_t networkExpiryMinutes;  // minutes without activity before the network times out
    int8_t networkConnIndexLast;   // the index of the last network a connection was established to
} wifi____all___t;

typedef struct {
    uint16_t ntpUpdateMinutes;
    char timezone[64];
} time____all___t;

typedef struct {
    float temperatureOffset;
} sco2____all___t;

typedef enum : uint8_t {
    SIGNAL__VAL______ON,
    SIGNAL__VAL_____OFF
} signal__val___e;

typedef struct {
    signal__val___e signalValSound;
} signal__all___t;

typedef struct {
    disp____all___t disp;
    wifi____all___t wifi;
    time____all___t time;
    sco2____all___t sco2;
    signal__all___t sign;
    float pressureZerolevel;  // calculated sealevel pressure
    float altitudeBaselevel;  // the altitude that the seonsor was configured to (or set by the user)
} config_t;

class Config {
   private:
   public:
    static config_t load();
};

#endif