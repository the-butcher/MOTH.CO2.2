# MOTH.CO2.2

This projects provides instructions for building a CO₂ sensor from commercially available electronic parts combined with 3D printed housing parts.

![CO₂-Sensor fully assembled](/images/sensor01_800.jpg?raw=true)

- The device uses the [Sensirion SCD-41](https://www.adafruit.com/product/5190) Sensor, a small and high performance photoacoustic CO₂ sensor.

- The sensor measures CO₂, temperature, humidity and pressure. Measurements are rendered to the device display, and can also be shown in a simple <ins>chart</ins> to perceive recent measurement history. The display can be switched to an inverted <ins>dark mode</ins>. Thresholds are configurable, just like temperature display unit and timezone. An internal <ins>buzzer</ins> can be turned on to give acoustic feedback when CO₂ thresholds are exceeded.

- Measurements are stored on an internal SD-Card, thus effectively providing <ins>unlimited measurement history</ins>.

- WiFi connectivity can be used to access current and historic measurements and to change device configuration or to calibrate. Measurements can also be be auto published over the [MQTT](https://de.wikipedia.org/wiki/MQTT) protocol.

- When operated in low power mode (no WiFi, 3 minute display update), the device lasts around <ins>1 week</ins> on a single battery charge.

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

  - client.html

    This page resembles the device's tabular view of the latest measurement.

    ![web-app client](/images/client_800.png?raw=true)

  - chart.html

    This page gives access to the full history of measurements. Given the size of the SD-Card, there will be no limit regarding storage, effectively the device will keep an indefinite list of measurements.

    ![web-app client](/images/chart_800.png?raw=true)

  - server.html

    This page gives access to the device's api. View status, configuration, data, upload file, calibrate, reset, ...

    ![web-app client](/images/server_800.png?raw=true)


- ### MQTT

  TODO :: describe

- ### TODO :: here or somewhere (!) have a set of basic introductions

---

## License

Please be aware that all software in this project is licensed under the [MIT license](license.txt), while the drawings and 3d-printed parts are licensed under the [Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)](https://creativecommons.org/licenses/by-nc-sa/4.0/) license.


https://github.com/the-butcher/MOTH.CO2.2/assets/84620977/6b267401-aefe-4eec-aebe-592d79e80dc9

