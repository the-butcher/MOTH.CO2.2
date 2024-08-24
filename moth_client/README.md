## CLient

---

The client subproject is react-web-application, storable on the device's SD-card and consumable with any browser directly from the device.

The Client UI is split into 3 main pages:

---

- #### client-chart

<img src="../images/device_ui_00_800.gif">

It is possible to show recent and historic data:

- CO₂ (ppm)
- temperature (°C)
- humidity (%RH)
- barometric pressure (hPa)
- battery (%)

<img src="../images/device_ui_01_800.gif">

Date/Time Pickers can be used to specify the range of data to be shown in the chart. Since the SD-card is large enough to store the entire lifetime of the sensor, so any date range from the life time of the sensor can be picked.

The displayed range of data can be exported to:

  - CSV
  - PNG

---

- #### client-config

<img src="../images/device_ui_02_800.gif">

This page offers a convenient way to configure the device through a reactive UI.

- #### client-api

<img src="../images/device_ui_03_800.png">

This is a helper page to descibe the device's api. This page can either be used as-is, or it can serve as the basis to understand the device api.

---

#### wifi-rest-api

A short list of the device's api:

|url|method|type|description|parameters|
|---|---|---|------------|--------------|
|/api/latest|GET|json|get the latest measurement as JSON|none|
|/api/valcsv|GET|csv|get the last hour of measurements as CSV|none|
|/api/datcsv|GET|csv|get the contents of a data file as CSV|"file"<br>i.e. 2024/05/20230526.dat|
|/api/valcsv|GET|binary|get the last hour of measurements as binary data|none|
|/api/datout|GET|binary|get the contents of a file as binary data|"file"<br>i.e. 2024/05/20230526.dat|
|/api/dirout|GET|json|list the contents of a folder|"folder"<br>i.e. "2024", "2024/05/", "config"|
|/api/upload|POST|json|upload files to the device|"file"<br>i.e. "server/root.html"<br>"content"<br>multipart file content|
|/api/datdel|GET|json|delete files from the device|"file"<br>i.e. 2024/05/20240526.dat|
|/api/dirdel|GET|json|delete folders from the device|"folder"<br>i.e. "2024", "2024/05/"|
|/api/dspset|GET|json|change device display|"p"<br>0: display mode -> v:[0:2]<br>1: display theme -> v:[0:1]<br>2: table value-> v:[0:2]<br>3: chart value-> v:[0:5]|
|/api/status|GET|json|get details about device status|none|
|/api/netout|GET|json|get a list of networks visible to the device|none|
|/api/netoff|GET|json|disconnect the device|none|
|/api/co2cal|GET|json|calibrate the CO₂ sensor to a given reference value|"ref"<br>i.e. 420|
|/api/co2rst|GET|json|reset the CO₂ sensor to factory||
|/api/esprst|GET|json|reset the device||
|/api/update|POST|json|update device firmware|"content"<br>multipart file containing the new binary|

---

There are multiple ways connectivity to the device can be established:

- When no wifi-connection has been configured yet, the device will start in AP-mode, providing it's own network. You can connect you mobile phone or computer to that network and configure i.e. your home wifi. The AP connections password is "CO2@420PPM".
- When wifi-connections have been configured, the device will pick from the configured networks the one with the highest signal strength.

What has been working for me is configuring my home network and my mobile's wifi-hotspot. That way the sensor will connect to the home network most of the time, but there is a fallback to the hotspot of my phone when i'm away. When in a hotel i add the hotel network, so i can save data on the hotspot.
