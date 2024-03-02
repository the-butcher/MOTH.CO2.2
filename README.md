# MOTH.CO2.2

This projects provides instructions for building a CO₂ sensor from commercially available electronic parts combined with 3D printed housing parts.

![CO₂-Sensor fully assembled](/images/sensor01_800.jpg?raw=true)

- CO₂, temperature, humidity, pressure, altitude.

- 3 weeks on one battery charge, when operated in low power mode (no WiFi, 3 minute display update).

- [Sensirion SCD-41](https://www.adafruit.com/product/5190) Sensor used, a small and high performance photoacoustic CO₂ sensor.

- Chart with 1h, 3h, 6h, 12h or 24h data history.

![CO₂-Sensor fully assembled](/images/sensor03_800.jpg?raw=true)

- Dark mode (inverted) available.

- Configurable Thresholds for CO₂, temperature und humidity.

- Configurable temperature unit °C and °F and timezone.

- Unlimited measurement history through internal SD-Card storage.

- WiFi for access to current and historic measurements and to change device configuration, calibration, update firmware.

![CO₂-Sensor fully assembled](/images/sensor04_800.jpg?raw=true)

- MQTT can be configured to auto publish Measurements ([MQTT](https://de.wikipedia.org/wiki/MQTT)).

- The housing can be wall mounted, with or without theft protection.

The root folder of the project is structured as follows:

---

The Arduino Sketch running on the device, configuration files and some [GFX Fonts](https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts) used:

## [MOTH Core](moth_core/README.md)

---

Building instructions, drawings and printable files for the device:


## [MOTH Parts and Assembly](moth_parts/README.md)

---

A react administration UI for the device:


## [MOTH Client](moth_client/README.md)

---

## Data

- ### On display

The device display supports two display modes, "table" and "chart". In table mode the latest CO₂, temperature and  humidity values are shown numerically

TODO :: image here

In chart mode one of CO₂, temperature, humidity or pressure are shown in a simple chart.

TODO :: image here

- ### Over Wifi

  The device hosts a small react-web-application.

  - chart.html

    This page gives access to the full history of measurements. Given the size of the SD-Card, there will be no limit regarding storage, effectively the device will keep an infinite list of measurements.

    ![web-app client](/images/chart_800.png?raw=true)

  - server.html

    This page gives access to the device's api. View status, configuration, data, upload file, calibrate, reset, update firmware, ...

    ![web-app client](/images/server_800.png?raw=true)


- ### MQTT

  It is possible to configure a mqtt connection and upload a certificate. The device will make connections in configurable intervals und publish it's measurements.
  Even though no POC has been made so far, it should be straightforward to integrate the sensor into i.e. Home Assistant.

- ### TODO :: here or somewhere (!) have a set of basic introductions

---

## License

Please be aware that all software in this project is licensed under the [MIT license](license.txt), while the drawings and 3d-printed parts are licensed under the [Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)](https://creativecommons.org/licenses/by-nc-sa/4.0/) license.

https://github.com/the-butcher/MOTH.CO2.2/assets/84620977/23303f60-04a6-4f3b-a14d-cb0d6dc5e2be


