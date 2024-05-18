# MOTH.CO2.2

This projects provides instructions for building a CO₂ sensor from commercially available electronic parts combined with 3D printed housing parts.

![CO₂-Sensor fully assembled](/images/sensor01_800.jpg?raw=true)

- CO₂, temperature, humidity, pressure, altitude.

- 2 months on one battery charge, when operated in low power mode (no WiFi, 3 minute display update).

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

---
## The root folder of the project is structured as follows:
### [MOTH Core](moth_core/README.md)

This project is built with [PlatformIO](https://platformio.org/). Please refer to the instructions provided on the PlatformIO website. PlatformIO makes development with the ESP32 microcontroller easier and faster compared to the Arduino IDE. If you have not switched yet i advise you to do so, it will save you time after a very short period of time.
The PlatformIO project, and configuration files can be found here.

---
### [MOTH Parts and Assembly](moth_parts/README.md)

Building instructions, drawings and printable files for the device.

---
### [MOTH Client](moth_client/README.md)

A react administration UI for the device.

![web-app client](/images/moth_client.gif)

---

### Data access

- #### On display

The device display supports two display modes, "table" and "chart". In table mode the latest CO₂, temperature and  humidity values are shown numerically

TODO :: image here

In chart mode one of CO₂, temperature, humidity or pressure are shown in a simple chart.

TODO :: image here

- #### Over Wifi

  The device provides various possibilities to access current and historic data.

- #### MQTT

  It is possible to configure a mqtt connection and upload a certificate. The device will make connections in configurable intervals und publish it's measurements.
  Even though no POC has been made so far, it should be straightforward to integrate the sensor into i.e. Home Assistant.

- #### TODO :: here or somewhere (!) have a set of basic introductions

---

### License

Please be aware that all software in this project is licensed under the [MIT license](license.txt), while the drawings and 3d-printed parts are licensed under the [Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)](https://creativecommons.org/licenses/by-nc-sa/4.0/) license.

https://github.com/the-butcher/MOTH.CO2.2/assets/84620977/23303f60-04a6-4f3b-a14d-cb0d6dc5e2be


