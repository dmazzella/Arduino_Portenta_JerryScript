/*
  MIT License

  Copyright (c) 2022 Damiano Mazzella

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef ARDUINO_PORTENTA_JERRYSCRIPT_H_
#define ARDUINO_PORTENTA_JERRYSCRIPT_H_

/******************************************************************************
 * INCLUDE
 ******************************************************************************/
#include "Arduino.h"
#include "mbed.h"

#include "jerryscript.h"
#include "jerryscript-ext.h"


/**
 * Major version of Arduino_Portenta_JerryScript API.
 */
#define ARDUINO_PORTENTA_JERRYSCRIPT_API_MAJOR_VERSION 0

/**
 * Minor version of Arduino_Portenta_JerryScript API.
 */
#define ARDUINO_PORTENTA_JERRYSCRIPT_API_MINOR_VERSION 1

/**
 * Patch version of Arduino_Portenta_JerryScript API.
 */
#define ARDUINO_PORTENTA_JERRYSCRIPT_API_PATCH_VERSION 0

/*******************************************************************************
 *                                  Arduino API                                *
 ******************************************************************************/

#define JERRYXX_BOOL_CHK(f)       \
    do                            \
    {                             \
        if( !( ret = (f) ) )      \
        {                         \
          goto cleanup;           \
        }                         \
    } while( 0 )

/**
 * Register a JavaScript property in the global object.
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool
jerryxx_register_global_property (const char *name_p, /**< name of the property */
                                  jerry_value_t value, /**< value of the property */
                                  bool free_value); /**< take ownership of the value */


/**
 * Register Arduino API into JavaScript global object.
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool
jerryxx_register_arduino_api(void);

/**
 * Arduino: pinMode
 */
jerry_value_t
js_pin_mode (const jerry_call_info_t *call_info_p, /**< call information */
             const jerry_value_t args_p[], /**< function arguments */
             const jerry_length_t args_cnt); /**< number of function arguments */

/**
 * Arduino: digitalWrite
 */
jerry_value_t
js_digital_write (const jerry_call_info_t *call_info_p, /**< call information */
                  const jerry_value_t args_p[], /**< function arguments */
                  const jerry_length_t args_cnt); /**< number of function arguments */

/**
 * Arduino: digitalRead
 */
jerry_value_t
js_digital_read (const jerry_call_info_t *call_info_p, /**< call information */
                 const jerry_value_t args_p[], /**< function arguments */
                 const jerry_length_t args_cnt); /**< number of function arguments */

/**
 * Arduino: delay
 */
jerry_value_t
js_delay (const jerry_call_info_t *call_info_p, /**< call information */
          const jerry_value_t args_p[], /**< function arguments */
          const jerry_length_t args_cnt); /**< number of function arguments */

/**
 * Arduino: delayMicroseconds
 */
jerry_value_t
js_delay_microseconds (const jerry_call_info_t *call_info_p, /**< call information */
                       const jerry_value_t args_p[], /**< function arguments */
                       const jerry_length_t args_cnt); /**< number of function arguments */

/**
 * Arduino: micros
 */
jerry_value_t
js_micros (const jerry_call_info_t *call_info_p, /**< call information */
           const jerry_value_t args_p[], /**< function arguments */
           const jerry_length_t args_cnt); /**< number of function arguments */

/**
 * Arduino: millis
 */
jerry_value_t
js_millis (const jerry_call_info_t *call_info_p, /**< call information */
           const jerry_value_t args_p[], /**< function arguments */
           const jerry_length_t args_cnt); /**< number of function arguments */

#endif /* ARDUINO_PORTENTA_JERRYSCRIPT_H_ */