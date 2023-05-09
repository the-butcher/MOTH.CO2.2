# MOTH.CO2.2

This projects provides instructions for building a CO₂ sensor from commercially available electronic parts combined with 3D printed housing parts.

![CO₂-Sensor fully assembled](/images/sensor01_800.jpg?raw=true)

![CO₂-Sensor electronics](/images/sensor02_800.jpg?raw=true)

- The device uses the [Sensirion SCD-41] (https://www.adafruit.com/product/5190) Sensor, a small and high performance photoacoustic CO₂ sensor.

- The sensor measures CO₂, temperature, humidity and pressure. Measurements are rendered to the device display, and can be shown in a simple <b>chart</b> to perceive recent measurement history. The display can be switched to an inverted <b>dark mode</b>. Thresholds are configurable, just like temperature display unit and timezone. An internal <b>buzzer</b> can be turned on to give acoustic feedback when CO₂ thresholds are exceeded.

- Measurements are stored on an internal SD-Card, thus effectively providing <b>unlimited measurement history</b>.

- WiFi connectivity can be used to access current and historic measurements and to change device configuration or to calibrate. Measurements can also be be auto published over the [MQTT](https://de.wikipedia.org/wiki/MQTT) protocol.

- When operated in low power mode (no WiFi, 3 minute display update), the device lasts 4 days or longer on a single battery charge.

- The housing has wall mount openings which can be used as fixed mounts or like picture hooks when both wall mount and portability is required.

The root folder of the project is structured as follows:

---

The Arduino Sketch running on the device, configuration files and some [GFX Fonts](https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts) used:

- ## [MOTH Core](moth_core/README.md)

---

Building instructions, drawings and printable files for the device:


- ## [MOTH Parts and Assembly](moth_parts/README.md)

---

A react administration UI for the device:


- ## [MOTH Client](moth_client/README.md)

---

## License

Please be aware that all software in this project is licensed under the [MIT license](license.txt), while the drawings and 3d-printed parts are licensed under the [Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)](https://creativecommons.org/licenses/by-nc-sa/4.0/) license.

---

https://github.com/the-butcher/MOTH.CO2.2/assets/84620977/6b267401-aefe-4eec-aebe-592d79e80dc9

