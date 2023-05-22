## MOTH CLient

---

A small webapp helping to understand and use the device api.

<img src="../images/mothapi_800.png">

---

The api lets you do the following

|url|method|type|description|
|---|---|---|------------|
|/api/latest|GET|json|get the latest measurement|
|/api/data|GET|csv|get the last hour of measurements|
|/api/status|GET|json|get details about device status|
|/api/folder|GET|json|list the contents of a folder on the device|
|/api/file|GET|json|get the contents of a file on the device|
|/api/encrypt|GET|json|encrpyt a string for usage in either wifi.json or mqtt.json|
|/api/upload|POST|json|upload files to the device|
|/api/delete|GET|json|delete files from the device|
|/api/networks|GET|json|get a list of networks visible to the device|
|/api/disconnect|GET|json|disconnect the device|
|/api/calibrate|GET|json|calibrate the CO₂ sensor to a given reference value|
|/api/hibernate|GET|json|hibernate the device|
|/api/co2_reset|GET|json|reset the CO₂ sensor to factory|