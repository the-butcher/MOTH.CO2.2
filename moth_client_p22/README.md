## MOTH CLient

---

A small webapp helping to understand and use the device api.

<img src="../images/mothapi_800.png">

---

The api lets you do the following

|url|method|type|description|parameters|
|---|---|---|------------|--------------|
|/api/latest|GET|json|get the latest measurement|none|
|/api/data|GET|csv|get the last hour of measurements|none|
|/api/status|GET|json|get details about device status|none|
|/api/folder|GET|json|list the contents of a folder on the device|"folder"<br>i.e. "2023", "2023/05/", "config"|
|/api/file|GET|json|get the contents of a file on the device|"file"<br>i.e. 2023/05/20230526.csv|
|/api/encrypt|GET|json|encrpyt a string for usage in either wifi.json or mqtt.json|"value"<br>i.e. "mysecret"|
|/api/upload|POST|json|upload files to the device||
|/api/delete|GET|json|delete files from the device|"file"<br>i.e. 2023/05/20230526.csv|
|/api/networks|GET|json|get a list of networks visible to the device|none|
|/api/disconnect|GET|json|disconnect the device|none|
|/api/calibrate|GET|json|calibrate the CO₂ sensor to a given reference value|"ref"|
|/api/hibernate|GET|json|hibernate the device||
|/api/co2_reset|GET|json|reset the CO₂ sensor to factory||