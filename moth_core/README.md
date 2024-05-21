# MOTH Core

This folder contains the Arduino Sketch for the MOTH.CO2.2 sensor and a set of configuration files in [SD](SD) that need be copied to the SD card of the device.

The moth_core folder of the project is structured as follows:

---

Historic versions of the project (kept for reference):

#### [moth_core_020](moth_core_020)
#### [moth_core_021](moth_core_021)

Most recent PlatformIO project:

## [moth_core_022](moth_core_022)

---

Configuration files that define the behaviour of the device.

## [SD/config](SD/config/)

---

- ### [disp.json](SD/config/disp.json)

Configuration for everything related to displaying values.

documentation (please do not use the documented version on the device, but a clean json without the comments):

```
{
  "min": 3, <<< display update time in minutes
  "ssc": true, <<< allow shorter display update when a significant value change occured
  "alt": 153, <<< base altitude of the device
  "tzn": "CET-1CEST,M3.5.0,M10.5.0/3", <<< timezone
  "co2": {
    "wHi": 800, <<< co2 warn limit
    "rHi": 1000 <<< co2 risk limit
    "ref": 425, <<< co2 reference value for stale calculation
    "cal": 400, <<< co2 reference value for fresh air calibration
    "lpa": 0.5  <<< low pass filter alpha, low value yields stronger filtering
  },
  "deg": {
    "rLo": 14, <<< lower temperature (celsius) risk limit
    "wLo": 19, <<< lower temperature (celsius) warn limit
    "wHi": 25, <<< upper temperature (celsius) warn limit
    "rHi": 30, <<< upper temperature (celsius) risk limit
    "off": 1.5, <<< temperature offset of the scd41 sensor
    "c2f": false <<< temperature display in fahrenheit?
  },
  "hum": {
    "rLo": 25, <<< lower humidity (% RH) risk limit
    "wLo": 30, <<< lower humidity (% RH) warn limit
    "wHi": 60, <<< upper humidity (% RH) warn limit
    "rHi": 65  <<< upper humidity (% RH) risk limit
  },
  "bme": {
    "alt": 153, <<< base altitude of the sensor
    "lpa": 0.25 <<< low pass filter alpha, low value yields stronger filtering
  }
}
```
General timezone documentation can be found at:
[timezones](https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv)

Measurements are stored with local times.

Timezones that have been tested to work are (but other will work too):

|code|where|
|---|---|
|CET-1CEST,M3.5.0,M10.5.0/3|Vienna|
|GMT0BST,M3.5.0/1,M10.5.0|London|
|AST4ADT,M3.2.0,M11.1.0|Atlantic|
|EST5EDT,M3.2.0,M11.1.0|New York|
|CST6CDT,M3.2.0,M11.1.0|Chicago|
|MST7MDT,M3.2.0,M11.1.0|Denver|
|PST8PDT,M3.2.0,M11.1.0|Los Angeles|

example:

```
{
  "min": 3,
  "ssc": true,
  "tzn": "CET-1CEST,M3.5.0,M10.5.0/3",
  "co2": {
    "wHi": 800,
    "rHi": 1000,
    "ref": 425,
    "cal": 400,
    "lpa": 0.5
  },
  "deg": {
    "rLo": 14,
    "wLo": 19,
    "wHi": 25,
    "rHi": 30,
    "off": 1.5,
    "c2f": false
  },
  "hum": {
    "rLo": 25,
    "wLo": 30,
    "wHi": 60,
    "rHi": 65
  },
  "bme": {
    "alt": 153,
    "lpa": 0.25
  }
}
```

- ### [mqtt.json](SD/config/mqtt.json)

Configuration for [MQTT](https://de.wikipedia.org/wiki/MQTT).

documentation (please do not use the documented version on the device, but a clean json without the comments):

```
{
  "use": true, <<< use mqtt?
  "srv": "192.168.0.123", <<< mqtt broker host (required)
  "prt": 8883, <<< mqtt broker port (required)
  "usr": "hannes", <<< mqtt user (optional)
  "pwd": "n7NZ+SfvP68wzUgh3t4acw==", <<< mqtt password (optional)
  "cli": "moth_2", <<< the mqtt client-id that the device will use
  "min": 5 <<< publish interval in minutes
}
```

example (protected, if a certificate is present at "/config/ca.crt" secure):

```
{
  "use": true,
  "srv": "192.168.0.115",
  "prt": 8883,
  "usr": "hannes",
  "pwd": "fleischer",
  "cli": "moth__66",
  "min": 5
}
```

example (unprotected server, if the server does not require a secure connection, the certificate file "/config/ca.crt" should not exist):

```
{
  "use": true,
  "srv": "192.168.0.115",
  "prt": 1883,
  "cli": "moth__66",
  "min": 5
}
```

example (mqtt inactive):

```
{
  "use": false
}
```

for all above configurations, if there is a certificate present at "/config/ca.crt", a secure wifi-connection will be created using that certificate.

- ### [wifi.json](SD/config/wifi.json)

Configuration for the WiFi connection of the decive.

documentation (please do not use the documented version on the device, but a clean json without the comments):

```
{
  "min": 5, <<< wifi timeout in minutes, when not connected to power, wifi will auto turn off after that time
  "ntw": [ <<< an array of network connections
    {
      "key": "your-wifi-ssid", <<< the ssid of your network
      "pwd": "your-wifi-password" <<< the password of your network
    }
  ]
}
```

example:

```
{
  "min": 5,
  "ntw": [
    {
      "key": "your-wifi-ssid",
      "pwd": "your-wifi-password"
    }
  ]
}
```

example (unprotected network):

```
{
  "min": 5,
  "ntw": [
    {
      "key": "your-wifi-ssid",
      "pwd": ""
    }
  ]
}
```

## [SD/server](SD/server/)

The device provides webserver accessible through WiFi. The files in this folder provide a web-application that can be used to administer the device

The files for the webserver are built from the [moth_client_p22](../moth_client_p22/) subproject.

---
