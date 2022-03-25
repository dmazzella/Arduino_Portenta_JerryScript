# JerryScript port to ArduinoCore-Mbed

## Overview

Integrate [ArduinoCore-mbed](https://github.com/arduino/ArduinoCore-mbed) and modern JavaScript standards (ECMAScript 5/6/6+) powered by [JerryScript](https://github.com/jerryscript-project/jerryscript).

#### WARNING: 
This project is in beta stage and is subject to changes of the code-base, including project-wide name changes and API changes.
See Goals for a list of things supported and which will be supported in the future

## Tested on:
 - [Portenta H7](https://www.arduino.cc/pro/hardware/product/portenta-h7) on M7 core

## Goals:

 - [x] Working JerryScript engine
 - [x] Working Repl
 - [ ] Expose Arduino API in javascript
    <details><summary>in progress</summary>
    <p>

    - [x] digitalRead()
    - [x] digitalWrite()
    - [x] pinMode()


    - [ ] analogRead()
    - [ ] analogReference()
    - [ ] analogWrite()

    </p>
    </details>

## Examples:

<details><summary>Create a Javascript engine and execute the script `print ('Hello, World!', Math.random ());`
</summary>
<p>

ArduinoCoreMbedJS.ino
```c++
#include "Arduino.h"
#include "mbed.h"

#include "Arduino_Portenta_JerryScript.h"

REDIRECT_STDOUT_TO(Serial);


void setup() {
  /* Initialize Serial */
  Serial.begin(115200);

  /* Wait Serial */
  while (!Serial) {}

  printf("Mbed OS API: %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  printf("JerryScript API: %d.%d.%d\n", JERRY_API_MAJOR_VERSION, JERRY_API_MINOR_VERSION, JERRY_API_PATCH_VERSION);

  const jerry_char_t script[] = "print ('Hello, World!', Math.random ());";

  /* Initialize engine */
  jerry_init (JERRY_INIT_EMPTY);

  /* Set log level */
  jerry_log_set_level (JERRY_LOG_LEVEL_DEBUG);

  /* Register the print function in the global object */
  jerryx_register_global ("print", jerryx_handler_print);

  /* Setup Global scope code */
  jerry_value_t parsed_code = jerry_parse (script, sizeof (script) - 1, NULL);

  /* Check if there is any JS code parse error */
  if (jerry_value_is_error (parsed_code))
  {
    JERRYX_ERROR_MSG("parse: %d\n", jerry_value_is_error (parsed_code));
  }
  else
  {
    /* Execute the parsed source code in the Global scope */
    jerry_value_t ret_value = jerry_run (parsed_code);

    /* Check the execution return value if there is any error */
    if (jerry_value_is_error (ret_value))
    {
      JERRYX_ERROR_MSG("run: %d\n", jerry_value_is_error (ret_value));
    }

    /* Returned value must be freed */
    jerry_value_free (ret_value);
  }

  /* Parsed source code must be freed */
  jerry_value_free (parsed_code);

  /* Cleanup engine */
  jerry_cleanup ();
}

void loop() {
  delay(1000);
}
```

## Output
```
Mbed OS API: 6.15.1
JerryScript API: 3.0.0
Hello, World! 0.6900010318495333
```
</p>
</details>

#### for other see `examples` folder of this repository
