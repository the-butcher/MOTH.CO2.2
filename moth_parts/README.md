[MOTH Parts](#moth_parts)

[MOTH Assembly](#moth_assembly)

---

# <a name="moth_parts">MOTH Parts</a>

Everyone can build this sensor, well almost everyone. All you need is access to a 3d-printer, your own or a may a friends printer. The rest is electronic parts available in retail and some hardware.

Electronic parts:

- [Adafruit ESP32-S2 Feather with BME280](https://www.adafruit.com/product/5303) The microcontroller. Not an S3, but it has an BME-280 Sensor onboard, thus saving an extra oart to be fit into the housing. The processors power is far beyond necessary and the pressure sensor delivers extra precision to the CO₂ sensor.

![Adafruit ESP32-S2 Feather with BME280](https://cdn-shop.adafruit.com/970x728/5303-14.jpg)

- [Adafruit SCD-41 - True CO2 Temperature and Humidity Sensor - STEMMA QT / Qwiic](https://www.adafruit.com/product/5190) The actual CO₂ sensor. Easy to connect due STEMMA QT connection with the microcontroller.

![Adafruit SCD-41 - True CO2 Temperature and Humidity Sensor - STEMMA QT / Qwiic](https://cdn-shop.adafruit.com/970x728/5190-06.jpg)

- [STEMMA QT / Qwiic JST SH 4-Pin Cable - 50mm Long](https://www.adafruit.com/product/4399) Cable connecting microcontroller and CO₂ sensor.

![STEMMA QT / Qwiic JST SH 4-Pin Cable - 50mm Long](https://cdn-shop.adafruit.com/970x728/4399-00.jpg)

- [Adafruit 2.9" Grayscale eInk / ePaper Display FeatherWing - 4 Level Grayscale](https://www.adafruit.com/product/4777) Cable connecting microcontroller and CO₂ sensor.

![Adafruit 2.9" Grayscale eInk / ePaper Display FeatherWing - 4 Level Grayscale](https://cdn-shop.adafruit.com/970x728/4777-00.jpg)

- [3,7 V 3000mAh 18650 Lithium Li-Ion Battery Pack](https://batteryzone.de/products/3-7-v-3000mah-18650-lithium-li-ion-batterien-pack-wiederaufladbare-mit-xh-2-54mm-2pin-stecker-fur-rc-boot-diy-power-bank?variant=40528117825744&currency=EUR&utm_medium=product_sync&utm_source=google&utm_content=sag_organic&utm_campaign=sag_organic&gclid=CjwKCAiA-8SdBhBGEiwAWdgtcNf_XDsI0CIc7C1LFinTcrsDmiB5t58YchcTnI23JrJWoPHF5A4VPxoC0mYQAvD_BwE) 18650 Battery Pack with PH2.0 plug.

![3,7 V 3000mAh 18650 Lithium Li-Ion Battery Pack](../images/batterypack_970.jpg)

Hardware:

- 6 screws M2 x 5mm
- 4 screws M2 x 8mm
- 6 nuts M2
- 4 [threaded inserts M2 x 4mm](https://www.reichelt.at/at/de/3d-druck-gewindeeinsaetze-m2x4-70-stueck-rx-m2x4-p332211.html?PROVID=2807&gclid=Cj0KCQjwu-KiBhCsARIsAPztUF0J6DnrXp0YX_Ajaskqt8CrePC7NPw2n9GdOXkIdW2tjeDlc9kSpYEaAjDGEALw_wcB)

![threaded inserts M2 x 4mm](../images/inserts_970.jpg)

- acrylic glas 1mm, ~ 78mm x ~36mm

<img src="../images/pmma_970.jpg" alt= "acrylic glas 1mm" width="800">

---

# <a name="moth_assembly">MOTH Assembly</a>

- Lay the display flat on a soft surface (i.e. a piece of soft cloth). On the backside of the display solder a connection from the pad corresponding to the A4 pin to the pad labelled with "busy".

<img src="../images/busy_800.png" alt= "connect busy" width="800">

- Place the microcontroller directly over the the display. Stick short pins (should portrude ~1mm out when fully inserted) through each microcontroller pinhole into the respective display pinholes. Do NOT use a pin header strip, the microcontroller must sit directly on the display. Do NOT add a pin to the RST pinhole. This pin will be bridged to from another pin for more control.

<img src="../images/pins_800.png" alt= "pins" width="800">

- Solder all pins (not RST) to the microcontroller. After soldering you can separate microcontroller and display. You should now have the microcontroller with short pins soldered to it.

<img src="../images/s2_pins_800.png" alt= "s2 with pins" width="800">


- 3d-print [STL "supports"](supports.stl), clean printed parts with a file or sandpaper, cleanup holes with a 2mm drill if required. Use 4 M2 x 5mm screws and 4 M2 nuts to connect the feather esp32-s2 microcontroller and the scd-41 sensor. Do not touch the CO₂ sensors central part to avoid contamination.

<img src="../images/s2_scd41_800.png" alt= "s2 scd41 and supports" width="800">

- Carefully reattach to display to get a unit of display, microcontroller and CO₂ sensor.

<img src="../images/s2_scd41__disp_800.png" alt= "s2 scd41 display" width="800">

- Connect the pinhole corresponding to the microcontrollers A5 pin to the display RST pinhole. Normally the display would have been hardwirded to be microcontrollers reset button, but we need to be able to reset the display at runtime and this gives the possiblity to do so.

<img src="../images/reset_800.png" alt= "s2 scd41 display" width="800">
