# MOTH Core

This folder contains the Arduino Sketch for the MOTH.CO2.2 sensor and a set of configuration- and system files in [SD](SD) that need be placed on the SD card of the device.

The moth_core folder of the project is structured as follows:

---

Arduino Sketch

## [moth_core_020](moth_core_020)

Required packages are:

- RTClib at version 2.1.1
- Adafruit BusIO at version 1.14.1
- ESP Async WebServer at version 1.2.3
- AsyncTCP at version 1.1.1
- PubSubClient at version 2.8
- ArduinoJson at version 5.13.5
- Adafruit LC709203F at version 1.3.2
- SdFat - Adafruit Fork at version 2.2.1
- Adafruit EPD at version 4.5.1
- Adafruit GFX Library at version 1.11.5
- QRCode at version 0.0.1
- Adafruit Unified Sensor at version 1.1.9
- Adafruit BME280 Library at version 2.2.2
- Sensirion I2C SCD4x at version 0.4.0
- Sensirion Core at version 0.6.0
- AESLib at version 2.3.6

---

Configuration files that define the behaviour of the sensor and a single html page which is required for Cross-Site (CORS) concerns when administering the device.

## [SD/config](SD/config/)

- ### [encr.json](SD/config/encr.json)

Passwords stored in the device are encrypted. This file defines key and initialization vector for the encryption. You can use the default configuration, but it is recommended to define your own keys for extra safety. Once you know which keys you want to use you can use this [Online AES Encryption and Decryption Tool](https://www.javainuse.com/aesgenerator) to encrypt passwords needed in the configuration.

Using the key "ielxdb1yd4xlcco1" and initialization vector "4mtxg8yroia48rwf", the plaintext value of "toomuchcoffee" should encryt to "pM43tKZYyemPQVyHdezUog==", an encrypted value of "G8H9B97V6jSl4W7A7/KxdQ==" should decrypt to "inthemorning".

The device api provides an "api/encrypt" operation for convenience.

documentation (please do not use the documented version on the device, but a clean json without the comments):

```
{
  "key": "moth_aes128__key", <<< 16-character (!) encryption key
  "inv": "moth_aes128__inv"  <<< 16-character (!) initialization vector
}
```

example:

```
{
  "key": "ielxdb1yd4xlcco1",
  "inv": "4mtxg8yroia48rwf"
}
```

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
  },
  "deg": {
    "rLo": 14, <<< lower temperature (celsius) risk limit
    "wLo": 19, <<< lower temperature (celsius) warn limit
    "wHi": 25, <<< upper temperature (celsius) warn limit
    "rHi": 30, <<< upper temperature (celsius) risk limit
    "off": [ <<< temperature offsets
      0.805, <<< primary temperature offset of the scd41 sensor
      0.785  <<< secondary temperature offset of the bme280 sensor (this value will change internally at runtime)
    ],
    "c2f": false, <<< temperature display in fahrenheit?
    "cor": 0.42 <<< temperature correction (see explanation below)
  },
  "hum": {
    "rLo": 25, <<< lower humidity (% RH) risk limit
    "wLo": 30, <<< lower humidity (% RH) warn limit
    "wHi": 60, <<< upper humidity (% RH) warn limit
    "rHi": 65  <<< upper humidity (% RH) risk limit
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

When the "cor" property holds a value larger than zero, the device attempts to correct temperature changes from charging or WiFi operation.
This correction is based on the internal temperature difference between the on-controller bme280 sensor and the scd41's primary temperature sensor.
Check the moth_core_020/BoxDisplay::getDisplayValues() method for more detail.

example:

```
{
  "min": 3,
  "ssc": true,
  "alt": 153,
  "tzn": "CET-1CEST,M3.5.0,M10.5.0/3",
  "co2": {
    "wHi": 800,
    "rHi": 1000
  },
  "deg": {
    "rLo": 14,
    "wLo": 19,
    "wHi": 25,
    "rHi": 30,
    "off": [
      0.805,
      0.785
    ],
    "c2f": false,
    "cor": 0.42
  },
  "hum": {
    "rLo": 25,
    "wLo": 30,
    "wHi": 60,
    "rHi": 657
  }
}
```

- ### [mqtt.json](SD/config/mqtt.json)

Configuration for [MQTT](https://de.wikipedia.org/wiki/MQTT).

documentation (please do not use the documented version on the device, but a clean json without the comments):

```
{
  "srv": "192.168.0.123", <<< mqtt broker host (required)
  "prt": 8883, <<< mqtt broker port (required)
  "crt": "/config/ca.crt", <<< mqtt certificate (optional), if specified, the certificate must be present at the location specified
  "usr": "hannes", <<< mqtt user (optional)
  "pwd": "n7NZ+SfvP68wzUgh3t4acw==", <<< mqtt password (optional)
  "cli": "moth", <<< the mqtt client-id that the device will use
  "top": "moth/co2" <<< the mqtt topic that the device will use when publishing values
}
```

example (protected server with certificate):

```
{
  "srv": "192.168.0.116",
  "prt": 8883,
  "crt": "/config/ca.crt",
  "usr": "hannes",
  "pwd": "n7NZ+SfvP68wzUgh3t4acw==",
  "cli": "moth",
  "top": "moth/co2"
}
```

example (protected server, no certificate):

```
{
  "srv": "192.168.0.116",
  "prt": 8883,
  "usr": "hannes",
  "pwd": "n7NZ+SfvP68wzUgh3t4acw==",
  "cli": "moth",
  "top": "moth/co2"
}
```

example (unprotected server, no certificate):

```
{
  "srv": "192.168.0.116",
  "prt": 1883,
  "cli": "moth",
  "top": "moth/co2"
}
```

- ### [wifi.json](SD/config/wifi.json)

Configuration for the WiFi connection of the decive.

documentation (please do not use the documented version on the device, but a clean json without the comments):

```
{
  "min": 5, <<< wifi timeout in minutes, when not connected to power, wifi will auto turn off after that time
  "ntw": [ <<< an array of network connections
    {
      "key": "your-wifi-ssid", <<< the ssid of your network
      "pwd": "06xj5MI/EJoS/3PliM5nzA==" <<< encrypted password for your networt (see encr.json above)
    }
  ]
}
```

example (the password decrypts to "your-wifi-pass"):

```
{
  "min": 5,
  "ntw": [
    {
      "key": "your-wifi-ssid",
      "pwd": "06xj5MI/EJoS/3PliM5nzA=="
    }
  ]
}
```

## [SD/server](SD/server/)

The device contains a small webserver. The files in this folder server for convenient access to the device api.

- ### [SD/server/client.html](SD/server/chart.html)

Provides a simple UI showing the latest measurements.

- ### [SD/server/chart.html](SD/server/chart.html)

Provides a simple UI to access historic data stored in the device.

- ### [SD/server/server.html](SD/server/server.html)

Provides a simple UI to access the api operations of the device.

The files for the webserver are built from the [moth_client](../moth_client/) subproject.

---

The GFX Fonts used by the sketch. These need to be copied to your ...\Arduino\libraries\Adafruit_GFX_Library\Fonts directory once the [Adafruit_GFX_Library](https://github.com/adafruit/Adafruit-GFX-Library) has been installed.

## [fonts](fonts)

---