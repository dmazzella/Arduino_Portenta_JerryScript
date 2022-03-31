# JerryScript port to ArduinoCore-Mbed

## Overview

Integrate [ArduinoCore-mbed](https://github.com/arduino/ArduinoCore-mbed) and modern JavaScript standards (ECMAScript 5/6/6+) powered by [JerryScript](https://github.com/jerryscript-project/jerryscript).

#### WARNING: 
This project is in beta stage and is subject to changes of the code-base, including project-wide name changes and API changes.

<details><summary>The build have below flags enabled, if needed you can disable this for reduce code size</summary>
<p>

jerryscript-config.h
```c
// Build differences from default:
#define JERRY_LOGGING 1
#define JERRY_LINE_INFO 1
#define JERRY_ERROR_MESSAGES 1
#define JERRY_GLOBAL_HEAP_SIZE 128
#define JERRY_CPOINTER_32_BIT 1
```
</p>
</details>


## Tested on:
 - [Portenta H7](https://www.arduino.cc/pro/hardware/product/portenta-h7) on M7 core

## Goals:

 - [x] Working JerryScript engine
 - [x] Working Repl
 - [ ] Expose Arduino API in javascript (In progress)
    <details><summary>Details</summary>
    <p>

    ### Constants:

      - [x] HIGH | LOW | CHANGE | RISING | FALLING
      - [x] INPUT | OUTPUT | INPUT_PULLUP
      - [x] LSBFIRST | MSBFIRST
      - [x] PIN_LED | LED_BUILTIN | LEDR | LEDG | LEDB
      - [x] A0 | A1 | A2 | A3 | A4 | A5 | A6 | A7
      - [x] D0 | D1 | D2 | D3 | D4 | D5 | D6 | D7 | D8 | D9 | D10 | D11 | D12 | D13 | D14 | D19 | D20 | D21

    ### Functions:

      - Digital I/O:
        - [x] pinMode()
        - [x] digitalWrite()
        - [x] digitalRead()

      - Time:
        - [x] delay()
        - [x] delayMicroseconds()
        - [x] micros()
        - [x] millis()

      - Math:
        - [ ] abs()
        - [ ] constrain()
        - [ ] map()
        - [ ] max()
        - [ ] min()
        - [ ] pow()
        - [ ] sq()
        - [ ] sqrt()

      - Trigonometry:
        - [ ] cos()
        - [ ] sin()
        - [ ] tan()

      - Random Numbers:
        - [x] random()
        - [x] randomSeed()

      - Bits and Bytes:
        - [x] bit()
        - [x] bitClear()
        - [x] bitRead()
        - [x] bitSet()
        - [x] bitWrite()
        - [x] highByte()
        - [x] lowByte()

      - Analog I/O:
        - [x] analogRead()
        - [x] analogWrite()
        - [x] analogReadResolution()
        - [x] analogWriteResolution()

      - Advanced I/O:
        - [x] noTone()
        - [x] pulseIn()
        - [x] pulseInLong()
        - [x] shiftIn()
        - [x] shiftOut()
        - [x] tone()

      - External Interrupts:
        - [x] attachInterrupt()
        - [x] detachInterrupt()

      - Interrupts:
        - [x] interrupts()
        - [x] noInterrupts()

      - Characters:
        - [ ] isAlpha()
        - [ ] isAlphaNumeric()
        - [ ] isAscii()
        - [ ] isControl()
        - [ ] isDigit()
        - [ ] isGraph()
        - [ ] isHexadecimalDigit()
        - [ ] isLowerCase()
        - [ ] isPrintable()
        - [ ] isPunct()
        - [ ] isSpace()
        - [ ] isUpperCase()
        - [ ] isWhitespace()

      - Communication:
        - [ ] Serial
        - [ ] Stream

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

  /* Run script with 'eval' */
  jerry_value_free (jerry_eval (script, sizeof (script) - 1, JERRY_PARSE_NO_OPTS));

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

for other see `examples` folder of this repository
