## Description
- Create and mount a FAT filesystem of 10Mb into the external QSPI of the Portenta H7 (exposed as usb removable disk)
- Create Javascript engine, load and execute the script `index.js` from the filesystem

## Steps:
- Create a file named `index.js` into the usb removable disk and copy into the code below:

##### index.js
```javascript
import { exported_value } from "/js/modules/module.js"
import { getFeatureDetails } from "/js/modules/module_2.js"

print("module.js >> ", exported_value);
print("module_2.js >> ", getFeatureDetails());

print("index.js >> ", "Hello from FS");

```

- Create a folder named `modules` into the usb removable disk

- Create a file named `module.js` into the folder `modules` of the usb removable disk and copy into the code below:

##### module.js
```javascript
export var exported_value = 42;

```

- Create a file named `module_2.js` into the folder `modules` of the usb removable disk and copy into the code below:
##### module_2.js
```javascript
var featureName = "ECMAScript modules";
var year = 2018;

export function getFeatureDetails() {
    return "Feature name: " + featureName + " | developed in " + year;
}

```

## Output
```
Mbed OS API: 6.15.1
JerryScript API: 3.0.0
module.js >> 42
module_2.js >> Feature name: ECMAScript modules | developed in 2018
index.js >> Hello from FS
```
