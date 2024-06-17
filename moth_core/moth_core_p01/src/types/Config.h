#ifndef Config_h
#define Config_h

#include <Arduino.h>

const String JSON_KEY_______PMS = "pms";  // co2 thresholds
const String JSON_KEY_______DEG = "deg";  // temperature thresholds
const String JSON_KEY_______HUM = "hum";  // humidity thresholds
const String JSON_KEY_______BME = "bme";  // altitude and low pass
const String JSON_KEY_______USE = "use";
const String JSON_KEY____SERVER = "srv";
const String JSON_KEY______PORT = "prt";
const String JSON_KEY_______LPA = "lpa";
const String JSON_KEY______USER = "usr";
const String JSON_KEY_______PWD = "pwd";
const String JSON_KEY____CLIENT = "cli";
const String JSON_KEY___MINUTES = "min";
const String JSON_KEY__NETWORKS = "ntw";
const String JSON_KEY_______KEY = "key";
const String JSON_KEY____OFFSET = "off";  // temperature offset
const String JSON_KEY__TIMEZONE = "tzn";
const String JSON_KEY_RISK__LOW = "rLo";  //
const String JSON_KEY_WARN__LOW = "wLo";
const String JSON_KEY_WARN_HIGH = "wHi";
const String JSON_KEY_RISK_HIGH = "rHi";
const String JSON_KEY_______C2F = "c2f";  // convert to deg F?

typedef struct {
    uint16_t wHi;  // upper warn level
    uint16_t rHi;  // upper risk level
} thresholds_pms_t;

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

typedef enum : uint8_t {
    CONFIG_STAT__DEFAULT,
    CONFIG_STAT__APPLIED
} config___stat__e;

typedef enum : uint8_t {
    DISPLAY_VAL_S__ENTRY,  // splash screen
    DISPLAY_VAL_S___NONE
} display_val_s_e;

/**
 * current display value in table modus
 */
typedef enum : uint8_t {
    DISPLAY_VAL_T____010,
    DISPLAY_VAL_T____025,
    DISPLAY_VAL_T____100,
    DISPLAY_VAL_T____HPA
} display_val_t_e;

typedef enum : uint8_t {
    DISPLAY_VAL_D______F,
    DISPLAY_VAL_D______C
} display_val_d_e;

typedef struct {
    config___stat__e configStatus;  // display config status
    thresholds_pms_t thresholdsPms;
    thresholds_deg_t thresholdsDeg;
    thresholds_hum_t thresholdsHum;
    display_val_s_e displayValSetng;
    display_val_t_e displayValTable;
    display_val_d_e displayDegScale;
} disp____all___t;

typedef enum : uint8_t {
    WIFI____VAL_P__PND_Y,  // to be turned on
    WIFI____VAL_P__CUR_Y,  // turned on
    WIFI____VAL_P__CUR_N,  // turned off
    WIFI____VAL_P__PND_N   // to be turned off
} wifi____val_p_e;

typedef struct {
    config___stat__e configStatus;  // wifi config status
    wifi____val_p_e wifiValPower;   // the status that the wifi should have, by button action
    uint8_t networkExpiryMinutes;   // minutes without activity before the network times out
    int8_t networkConnIndexLast;    // the index of the last network a connection was established to
} wifi____all___t;

typedef enum : uint8_t {
    MQTT______________OK = 10,
    // connection issues, associated with states of the mqtt client after a connection attempt
    MQTT_TIMEOUT____CONN = 20,
    MQTT_LOST_______CONN = 21,  // connection was lost
    MQTT_FAIL_______CONN = 22,  // connect failed
    MQTT_FAIL____PUBLISH = 30,  // failure during publishing
    // unrecoverable issues
    MQTT_BAD____PROTOCOL = 40,
    MQTT_BAD_________CLI = 41,
    MQTT_UNAVAIL____CONN = 42,
    MQTT_BAD_CREDENTIALS = 43,
    MQTT_NO_________AUTH = 44,
    // configuration issues, unrecoverable
    MQTT_CNF____NOT_USED = 50,
    MQTT_CNF_________SRV = 60,
    MQTT_CNF_________PRT = 61,
    MQTT_CNF_________CLI = 62,
    MQTT_FAIL________DAT = 63,
    MQTT_NO__________DAT = 64,
    // unknown issues
    MQTT_________UNKNOWN = 99,

} mqtt____stat__e;

typedef struct {
    config___stat__e configStatus;  // mqtt config status
    uint32_t mqttPublishMinutes;    // minute interval for publishing mqtt messages, 0xFFFFFF when mqtt is not configured or misconfigured
    uint8_t mqttFailureCount;
    mqtt____stat__e mqttStatus;
} mqtt____all___t;

typedef struct {
    int32_t utcOffsetSeconds;
    uint16_t ntpUpdateMinutes;
    char timezone[64];
} time____all___t;

typedef struct {
    float lpFilterRatioCurr;  // low pass filter alpha
} spms____all___t;

typedef struct {
    float temperatureOffset;
    float lpFilterRatioCurr;  // low pass filter alpha
} sbme____all___t;

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
    spms____all___t spms;
    sbme____all___t sbme;
    signal__all___t sign;
    mqtt____all___t mqtt;
} config_t;

class Config {
   private:
    static config_t* config;

   public:
    static config_t load();
    static void begin(config_t* config);
    static mqtt____stat__e getMqttStatus();
    static void setUtcOffsetSeconds(int32_t utcOffsetSeconds);
    static int32_t getUtcOffsetSeconds();
};

#endif