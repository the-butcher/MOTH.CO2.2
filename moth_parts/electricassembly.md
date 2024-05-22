# Electric Assembly

Lay the display flat on a soft surface (i.e. a piece of soft cloth). On the backside of the display solder a connection from the pad corresponding to the A4 pin to the pad labelled with "busy".

<img src="../images/electric_assembly_00_800.png" alt= "connect busy" width="800">

---

Place the microcontroller directly over the the display. Stick short pins (should portrude ~1mm out when fully inserted) through each microcontroller pinhole into the respective display pinholes. Do NOT use a pin header strip, the microcontroller must sit directly on the display.

- Do NOT add a pin to the RST pinhole. This pin will be bridged to from the A0 pin to allow for display only reset. Adding the possibility to reset the display during runtime will save power between display updates. There is also an issue with gray speckles showing on the grayscale eInk display while powered, being able to hibernate the display and reset it later takes care of this issue.
- Do NOT add a pin to the D13 pinhole. The D13 pin on the Adafruit Feather ESP32-S2 is tied to the D13 pin, but this pin is also being used by the C-Button of the eInk display. Therefore the red led of the ESP will always be on when using INPUT_PULLUP for this pin. This consumes power and shortens battery life. This pin will be bridged to from the D6 pin. That way the C-Button is effecticely rerouted to D6 and the red led will no be on, thus saving power.

<img src="../images/electric_assembly_01_800.png" alt= "pins" width="800">

---

Solder all pins (not RST, not D13) to the microcontroller. After soldering you can separate microcontroller and display. You should now have the microcontroller with short pins soldered to it.

<img src="../images/electric_assembly_02_800.png" alt= "s2 with pins" width="800">

---

3d-print [STL "supports2"](supports2.stl), clean printed parts with a file or sandpaper, cleanup holes with a 2mm drill if required. Use 6 M2 x 5mm screws and 6 M2 nuts to connect the feather esp32-s2 microcontroller and the scd-41 sensor. Try not to touch the CO₂ sensors central part to avoid contamination. Connect microcontroller, RTC and scd-41 sensor with the Stemma QT cables (not pictured).

I removed the yellow leds from both RTC and scd-41 with a scalpel. This will significantly prolong battery lifetime, but you must be aware that it will void the warranty for the respective parts.

<img src="../images/electric_assembly_03_800.png" alt= "s2 scd41 and supports" width="800">

---

Carefully reattach to display to get a unit of display, microcontroller, RTC and CO₂ sensor.

<img src="../images/electric_assembly_04_800.png" alt= "s2 scd41 display" width="800">

---

- Connect the pinhole corresponding to the microcontrollers A0 pin to the display RST pinhole. Normally the display would have been hardwirded to be microcontrollers reset button, but we need to be able to reset the display at runtime and this gives the possiblity to do so.
- Connect the pinhole corresponding to the microcontrollers D6 pin to the pinhole corresponding to the microcontrollers A13 pin. This allows for operation of the C-Button on the display without having the microcontrollers red led on.
- Connect the SQW pin of the RTC to the microcontrollers A5 pin. I used thin pliers to bend the connection wire so it would follow the contour of the support parts.

<img src="../images/electric_assembly_05_800.png" alt= "s2 scd41 display" width="800">

---

Attach the Buzzer so its pins go into GND and A1.

<img src="../images/electric_assembly_06_800.png" alt= "s2 scd41 display" width="800">

---

The Electric Assembly is now ready to be configured and to have a sketch uploaded. Please refer to [Core](../moth_core/README.md) for further instructions.

---