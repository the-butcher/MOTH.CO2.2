# MOTH.CO2.2

This projects provides instructions for building a CO₂ sensor from commercially available electronic parts combined with 3D printed housing parts.

![CO₂-Sensor fully assembled](/images/sensor01_800.jpg?raw=true)

- The device uses the [Sensirion SCD-41](https://www.adafruit.com/product/5190) Sensor, a small and high performance photoacoustic CO₂ sensor.

- The sensor measures CO₂, temperature, humidity and pressure. Measurements are rendered to the device display, and can also be shown in a simple <ins>chart</ins> to perceive recent measurement history. The display can be switched to an inverted <ins>dark mode</ins>. Thresholds are configurable, just like temperature display unit and timezone. An internal <ins>buzzer</ins> can be turned on to give acoustic feedback when CO₂ thresholds are exceeded.

- Measurements are stored on an internal SD-Card, thus effectively providing <ins>unlimited measurement history</ins>.

- WiFi connectivity can be used to access current and historic measurements and to change device configuration or to calibrate. Measurements can also be be auto published over the [MQTT](https://de.wikipedia.org/wiki/MQTT) protocol.

- When operated in low power mode (no WiFi, 3 minute display update), the device lasts <ins>4 days or longer</ins> on a single battery charge.

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

## License

Please be aware that all software in this project is licensed under the [MIT license](license.txt), while the drawings and 3d-printed parts are licensed under the [Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)](https://creativecommons.org/licenses/by-nc-sa/4.0/) license.

---

https://github.com/the-butcher/MOTH.CO2.2/assets/84620977/6b267401-aefe-4eec-aebe-592d79e80dc9

