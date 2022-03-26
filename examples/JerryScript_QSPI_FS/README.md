#### Description
- Create and mount a FAT filesystem of 10Mb into the external QSPI of the Portenta H7 (exposed as usb removable disk)
- Create Javascript engine, load and execute the script `index.js` from the filesystem

##### ArduinoCoreMbedJS.ino
```c++
#include "Arduino.h"
#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"
#include "PluggableUSBMSD.h"
#include "mbed.h"

#include "Arduino_Portenta_JerryScript.h"

REDIRECT_STDOUT_TO(Serial);

QSPIFBlockDevice root(QSPI_SO0, QSPI_SO1, QSPI_SO2, QSPI_SO3, QSPI_SCK, QSPI_CS, QSPIF_POLARITY_MODE_1, 40000000);
USBMSD MassStorage(&root);

mbed::MBRBlockDevice js_data(&root, 1);
mbed::FATFileSystem js_data_fs("js");

void print_directory(const char *path_p) {
  /* list the content of the partitions, you may need to restart the board for the list to update if you copied new files */
  DIR *d = NULL;
  if ((d = opendir(path_p)) != NULL) {
    struct dirent *p = NULL;
    while ((p = readdir(d)) != NULL) {
      printf("%s\n", p->d_name);
    }
  }
  closedir(d);
}

void USBMSD::begin() {
  mbed::MBRBlockDevice::partition(&root, 1, 0x0B, 0, (1024 * 1024) * 10);
  if (js_data_fs.mount(&js_data)) {
    /* Reformat if we can't mount the filesystem this should only happen on the first boot */
    printf("No filesystem was found. Formatting the filsystem\n");
    js_data_fs.reformat(&js_data);
  }
}

void setup() {
  /* Initialize Serial */
  Serial.begin(115200);

  /* Initialize MassStorage */
  MassStorage.begin();

  /* Wait Serial */
  while (!Serial) {}

  printf("Mbed OS API: %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  printf("JerryScript API: %d.%d.%d\n", JERRY_API_MAJOR_VERSION, JERRY_API_MINOR_VERSION, JERRY_API_PATCH_VERSION);
}

void loop() {
  if (MassStorage.media_removed()) {

    /* Initialize engine */
    jerry_init(JERRY_INIT_EMPTY);

    /* Set log level */
    jerry_log_set_level(JERRY_LOG_LEVEL_DEBUG);

    /* Register the print function in the global object */
    jerryx_register_global("print", jerryx_handler_print);

    /* Register the Arduino API in the global object */
    jerryxx_register_arduino_api ();

    jerry_size_t source_size;
    jerry_char_t *source_p = NULL;
    if ((source_p = jerry_port_source_read("/js/index.js", &source_size)) != NULL) {

      jerry_parse_options_t parse_options;
      parse_options.options = JERRY_PARSE_NO_OPTS | JERRY_PARSE_MODULE;

      /* Setup Global scope code */
      jerry_value_t parsed_code = jerry_parse(source_p, source_size, &parse_options);

      /* Check if there is any JS code parse error */
      if (jerry_value_is_error(parsed_code)) {
        JERRYX_ERROR_MSG("parse: %d\n", jerry_value_is_error(parsed_code));
      } else {
        if (parse_options.options & JERRY_PARSE_MODULE) {
          /* Link modules to their dependencies */
          jerry_value_t ret_value = jerry_module_link(parsed_code, NULL, NULL);
          if (jerry_value_is_error(ret_value)) {
            JERRYX_ERROR_MSG("module_link: %d\n", jerry_value_is_error(ret_value));
          } else {
            /* Returned value must be freed */
            jerry_value_free(ret_value);

            /* Execute the parsed source code in the Global scope */
            ret_value = jerry_module_evaluate(parsed_code);

            /* Returned value must be freed */
            jerry_value_free(ret_value);
          }
        } else {
          /* Execute the parsed source code in the Global scope */
          jerry_value_t ret_value = jerry_run(parsed_code);

          /* Check the execution return value if there is any error */
          if (jerry_value_is_error(ret_value)) {
            JERRYX_ERROR_MSG("run: %d\n", jerry_value_is_error(ret_value));
          }

          /* Returned value must be freed */
          jerry_value_free(ret_value);
        }
      }

      /* Parsed source code must be freed */
      jerry_value_free(parsed_code);

      jerry_port_source_free(source_p);
    }

    /* Cleanup engine */
    jerry_cleanup();
  }

  delay(1000);
}
```

#### Steps
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
