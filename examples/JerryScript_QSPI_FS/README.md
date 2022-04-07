## Description
- Create and mount a FAT filesystem of 10Mb into the external QSPI of the Portenta H7 (exposed as usb removable disk)
- Create Javascript engine, load and execute the script `main.mjs` from the filesystem

<details><summary>Details</summary>
<p>

## Steps:
- Create a file named `main.mjs` into the usb removable disk and copy into the code below:

##### main.mjs
```javascript
import { exported_value } from "/js/modules/module.mjs"
import { getFeatureDetails } from "/js/modules/module_2.mjs"

print("module.mjs >> ", exported_value);
print("module_2.mjs >> ", getFeatureDetails());

print("main.mjs >> ", "Hello from FS");

```

- Create a folder named `modules` into the usb removable disk

- Create a file named `module.mjs` into the folder `modules` of the usb removable disk and copy into the code below:

##### module.mjs
```javascript
export var exported_value = 42;

```

- Create a file named `module_2.mjs` into the folder `modules` of the usb removable disk and copy into the code below:
##### module_2.mjs
```javascript
var featureName = "ECMAScript modules";
var year = 2018;

export function getFeatureDetails() {
    return "Feature name: " + featureName + " | developed in " + year;
}

```
### All files used in this example are available also into the forder `QSPI_FS` 

</p>
</details>

## Output
```
Arduino Core API: 3.0.1
Mbed OS API: 6.15.1
JerryScript API: 3.0.0
module.mjs >> 42
module_2.mjs >> Feature name: ECMAScript modules | developed in 2018
main.mjs >> Hello from FS
```
