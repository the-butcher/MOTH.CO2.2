# MOTH Core

This folder contains the Arduino Sketch for the MOTH.CO2.2 sensor and a set of configuration- and system files in [SD](SD) that need be placed on the SD card of the device.

The root folder of the project is structured as follows:

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

## [SD](SD)

- ### [encr.json](SD/config/encr.json)

Passwords stored in the device are encrypted. This file defines key and initialization vector for the encryption. You can use the default configuration, but it is recommended to define your own keys for extra safety. Once you know which keys you want to use you can use this [Online AES Encryption and Decryption Tool](https://www.javainuse.com/aesgenerator) to encrypt passwords needed in the configuration.

Using the key "ielxdb1yd4xlcco1" and initialization vector "4mtxg8yroia48rwf", the plaintext value of "toomuchcoffee" should encryt to "pM43tKZYyemPQVyHdezUog==", an encrypted value of "G8H9B97V6jSl4W7A7/KxdQ==" should decrypt to "inthemorning".

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
	"off": 1.2, <<< temperature sensor offset (higher value gives lower temperature reading)
	"c2f": false <<< temperature display in fahrenheit?
  },
  "hum": {
    "rLo": 25, <<< lower humidity (% RH) risk limit
    "wLo": 30, <<< lower humidity (% RH) warn limit
    "wHi": 60, <<< upper humidity (% RH) warn limit
    "rHi": 65  <<< upper humidity (% RH) risk limit
  }
}
```

example:

```
{
  "min": 3,
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
	"off": 1.2,
	"c2f": false
  },
  "hum": {
    "rLo": 25,
    "wLo": 30,
    "wHi": 60,
    "rHi": 65
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
---

The GFX Fonts used by the sketch. These need to be copied to your ...\Arduino\libraries\Adafruit_GFX_Library\Fonts directory once the [Adafruit_GFX_Library](https://github.com/adafruit/Adafruit-GFX-Library) has been installed.

## [fonts](fonts)

---