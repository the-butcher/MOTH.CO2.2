## MOTH CLient

---

A small webapp helping to understand and use the device api.

<img src="../images/mothapi_800.png">

---

The api lets you do the following

|url|method|type|description|
|---|---|---|------------|
|/latest|GET|json|get the latest measurement|
|/data|GET|csv|get the last hour of measurements|
|/status|GET|json|get details about device status|
|/folder|GET|json|list the contents of a folder on the device|
|/file|GET|json|get the contents of a file on the device|
|/upload|POST|json|upload files to the device|
|/delete|GET|json|delete files from the device|
|/networks|GET|json|get a list of networks visible to the device|
|/disconnect|GET|json|disconnect the device|
|/calibrate|GET|json|calibrate the CO₂ sensor to a given reference value|
|/hibernate|GET|json|hibernate the device|
|/co2_reset|GET|json|reset the CO₂ sensor to factory|