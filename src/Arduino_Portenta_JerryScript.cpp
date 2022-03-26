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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "Arduino_Portenta_JerryScript.h"


/**
 * Default implementation of jerry_port_fatal. Calls 'abort' if exit code is
 * non-zero, 'exit' otherwise.
 */
void JERRY_ATTR_WEAK
jerry_port_fatal (jerry_fatal_code_t code) /**< cause of error */
{
  exit ((int) code);
} /* jerry_port_fatal */

/**
 * Implementation of jerry_port_current_time.
 *
 * @return current timer's counter value in milliseconds
 */
double
jerry_port_current_time (void)
{
  static uint64_t last_tick = 0;
  static time_t last_time = 0;
  static uint32_t skew = 0;

  uint64_t curr_tick = us_ticker_read (); /* The value is in microseconds. */
  time_t curr_time = time (NULL); /*  The value is in seconds. */
  double result = curr_time * 1000;

  /* The us_ticker_read () has an overflow for each UINT_MAX microseconds
   * (~71 mins). For each overflow event the ticker-based clock is about 33
   * milliseconds fast. Without a timer thread the milliseconds part of the
   * time can be corrected if the difference of two get_current_time calls
   * are within the mentioned 71 mins. Above that interval we can assume
   * that the milliseconds part of the time is negligibe.
   */
  if (curr_time - last_time > (time_t) (((uint32_t) -1) / 1000000))
  {
    skew = 0;
  }
  else if (last_tick > curr_tick)
  {
    skew = (skew + 33) % 1000;
  }
  result += (curr_tick / 1000 - skew) % 1000;

  last_tick = curr_tick;
  last_time = curr_time;
  return result;
} /* jerry_port_current_time */

/**
 * Dummy function to get the time zone adjustment.
 *
 * @return 0
 */
int32_t
jerry_port_local_tza (double unix_ms)
{
  (void) unix_ms;

  /* We live in UTC. */
  return 0;
} /* jerry_port_local_tza */

/**
 * Default implementation of jerry_port_log. Prints log messages to stderr.
 */
void JERRY_ATTR_WEAK
jerry_port_log (const char *message_p) /**< message */
{
  while (*message_p != '\0')
  {
    if (*message_p == '\n')
    {
      /* add CR for proper display in serial monitors */
      fputc ('\r', stderr);
    }

    fputc (*message_p++, stderr);
  }
} /* jerry_port_log */

/**
 * Default implementation of jerry_port_print_byte. Uses 'putchar' to
 * print a single character to standard output.
 */
void JERRY_ATTR_WEAK
jerry_port_print_byte (jerry_char_t byte) /**< the character to print */
{
  fputc (byte, stderr);
} /* jerry_port_print_byte */

/**
 * Default implementation of jerry_port_print_buffer. Uses 'jerry_port_print_byte' to
 * print characters of the input buffer.
 */
void JERRY_ATTR_WEAK
jerry_port_print_buffer (const jerry_char_t *buffer_p, /**< string buffer */
                         jerry_size_t buffer_size) /**< string size*/
{
  for (jerry_size_t i = 0; i < buffer_size; i++)
  {
    jerry_port_print_byte (buffer_p[i]);
  }
} /* jerry_port_print_byte */

/**
 * Read a line from standard input as a zero-terminated string.
 *
 * @param out_size_p: length of the string
 *
 * @return pointer to the buffer storing the string,
 *         or NULL if end of input
 */
jerry_char_t *JERRY_ATTR_WEAK
jerry_port_line_read (jerry_size_t *out_size_p)
{
  while (true)
  {
    if (Serial.available())
    {
      String data = "";
      while (Serial.available())
      {
        data += (char)Serial.read();
      }

      if (data != "")
      {
        char *line_p = NULL;
        if ((line_p = (char *) malloc (data.length() * sizeof(char))) != NULL)
        {
          *out_size_p = data.length();
          memcpy(line_p, data.c_str(), *out_size_p);
          return (jerry_char_t *) line_p;
        }
      }

      *out_size_p = 0;
      return (jerry_char_t *) NULL;
    }
  }

} /* jerry_port_line_read */

/**
 * Free a line buffer allocated by jerry_port_line_read
 *
 * @param buffer_p: buffer that has been allocated by jerry_port_line_read
 */
void JERRY_ATTR_WEAK
jerry_port_line_free (jerry_char_t *buffer_p)
{
  free (buffer_p);
} /* jerry_port_line_free */

/**
 * Determines the size of the given file.
 * @return size of the file
 */
static jerry_size_t
jerry_port_get_file_size (FILE *file_p) /**< opened file */
{
  fseek (file_p, 0, SEEK_END);
  long size = ftell (file_p);
  fseek (file_p, 0, SEEK_SET);

  return (jerry_size_t) size;
} /* jerry_port_get_file_size */

/**
 * Opens file with the given path and reads its source.
 * @return the source of the file
 */
jerry_char_t *JERRY_ATTR_WEAK
jerry_port_source_read (const char *file_name_p, /**< file name */
                        jerry_size_t *out_size_p) /**< [out] read bytes */
{
  FILE *file_p = fopen (file_name_p, "rb");

  if (file_p == NULL)
  {
    return NULL;
  }

  jerry_size_t file_size = jerry_port_get_file_size (file_p);
  jerry_char_t *buffer_p = (jerry_char_t *) malloc (file_size);

  if (buffer_p == NULL)
  {
    fclose (file_p);
    return NULL;
  }

  size_t bytes_read = fread (buffer_p, 1u, file_size, file_p);

  if (bytes_read != file_size)
  {
    fclose (file_p);
    free (buffer_p);
    return NULL;
  }

  fclose (file_p);
  *out_size_p = (jerry_size_t) bytes_read;

  return buffer_p;
} /* jerry_port_source_read */

/**
 * Release the previously opened file's content.
 */
void JERRY_ATTR_WEAK
jerry_port_source_free (uint8_t *buffer_p) /**< buffer to free */
{
  free (buffer_p);
} /* jerry_port_source_free */

#ifndef JERRY_GLOBAL_HEAP_SIZE
#define JERRY_GLOBAL_HEAP_SIZE 512
#endif /* JERRY_GLOBAL_HEAP_SIZE */

/**
 * Normalize a file path.
 *
 * @return a newly allocated buffer with the normalized path if the operation is successful,
 *         NULL otherwise
 */
jerry_char_t *JERRY_ATTR_WEAK
jerry_port_path_normalize (const jerry_char_t *path_p, /**< input path */
                           jerry_size_t path_size) /**< size of the path */
{
  jerry_char_t *buffer_p = (jerry_char_t *) malloc (path_size + 1);

  if (buffer_p == NULL)
  {
    return NULL;
  }

  /* Also copy terminating zero byte. */
  memcpy (buffer_p, path_p, path_size + 1);

  return buffer_p;
} /* jerry_port_normalize_path */

/**
 * Free a path buffer returned by jerry_port_path_normalize.
 *
 * @param path_p: the path to free
 */
void JERRY_ATTR_WEAK
jerry_port_path_free (jerry_char_t *path_p)
{
  free (path_p);
} /* jerry_port_normalize_path */

/**
 * Computes the end of the directory part of a path.
 *
 * @return end of the directory part of a path.
 */
jerry_size_t JERRY_ATTR_WEAK
jerry_port_path_base (const jerry_char_t *path_p) /**< path */
{
  const jerry_char_t *basename_p = (jerry_char_t *) strrchr ((char *) path_p, '/') + 1;

  if (basename_p == NULL)
  {
    return 0;
  }

  return (jerry_size_t) (basename_p - path_p);
} /* jerry_port_get_directory_end */

/**
 * Pointer to the current context.
 * Note that it is a global variable, and is not a thread safe implementation.
 */
static jerry_context_t *current_context_p = NULL;

/**
 * Allocate a new external context.
 *
 * @param context_size: requested context size
 *
 * @return total allcoated size
 */
size_t JERRY_ATTR_WEAK
jerry_port_context_alloc (size_t context_size)
{
  size_t total_size = context_size + JERRY_GLOBAL_HEAP_SIZE * 1024;
  current_context_p = (jerry_context_t *) malloc (total_size);

  return total_size;
} /* jerry_port_context_alloc */

/**
 * Get the current context.
 *
 * @return the pointer to the current context
 */
jerry_context_t *JERRY_ATTR_WEAK
jerry_port_context_get (void)
{
  return current_context_p;
} /* jerry_port_context_get */

/**
 * Free the currently allocated external context.
 */
void JERRY_ATTR_WEAK
jerry_port_context_free (void)
{
  free (current_context_p);
} /* jerry_port_context_free */

/*******************************************************************************
 *                                  Arduino API                                *
 ******************************************************************************/

/**
 * Register a JavaScript property in the global object.
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool
jerryxx_register_global_property (const char *name_p, /**< name of the property */
                                  jerry_value_t value, /**< value of the property */
                                  bool free_value) /**< take ownership of the value */
{
  jerry_value_t global_obj_val = jerry_current_realm ();
  jerry_value_t property_name_val = jerry_string_sz (name_p);

  jerry_value_t result_val = jerry_object_set (global_obj_val, property_name_val, value);
  bool result = jerry_value_is_true (result_val);

  jerry_value_free (result_val);
  jerry_value_free (property_name_val);
  jerry_value_free (global_obj_val);

  if (free_value)
  {
    jerry_value_free (value);
  }

  return result;
} /* jerryxx_register_global_property */

/**
 * Register Arduino API into JavaScript global object.
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool
jerryxx_register_arduino_api(void)
{
  bool ret = false;

  /* Register the HIGH in the global object */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("HIGH", jerry_number (HIGH), true));

  /* Register the LOW in the global object */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LOW", jerry_number (LOW), true));

  /* Register the INPUT in the global object */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("INPUT", jerry_number (INPUT), true));

  /* Register the OUTPUT in the global object */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("OUTPUT", jerry_number (OUTPUT), true));

  /* Register the INPUT_PULLUP in the global object */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("INPUT_PULLUP", jerry_number (INPUT_PULLUP), true));

  /* Register the INPUT_PULLDOWN in the global object */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("INPUT_PULLDOWN", jerry_number (INPUT_PULLDOWN), true));

  /* Register the LED_BUILTIN in the global object */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LED_BUILTIN", jerry_number (LED_BUILTIN), true));

  /* Register the pinMode function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global ("pinMode", js_pin_mode));

  /* Register the digitalWrite function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global ("digitalWrite", js_digital_write));

  /* Register the digitalRead function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global ("digitalRead", js_digital_read));
  
  /* Register the delay function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global ("delay", js_delay));

  /* Register the delayMicroseconds function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global ("delayMicroseconds", js_delay_microseconds));

  /* Register the micros function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global ("micros", js_micros));

  /* Register the millis function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global ("millis", js_millis));

cleanup:
  return ret;
} /* jerryxx_register_arduino_api */

/**
 * Arduino: pinMode
 */
jerry_value_t
js_pin_mode (const jerry_call_info_t *call_info_p, /**< call information */
             const jerry_value_t args_p[], /**< function arguments */
             const jerry_length_t args_cnt) /**< number of function arguments */
{
  JERRYX_UNUSED (call_info_p);
  if (args_cnt != 2)
  {
    return jerry_throw_sz (JERRY_ERROR_SYNTAX, "Wrong arguments count in 'pinMode' function.");
  }

  jerry_value_t pin = args_p[0];
  if(!jerry_value_is_number (pin))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "Wrong argument 'pin' must be a number.");
  }

  jerry_value_t mode = args_p[1];
  if(!jerry_value_is_number (mode))
  {
    jerry_throw_sz (JERRY_ERROR_TYPE, "Wrong argument 'mode' must be a number.");
  }

  double pin_mode = jerry_value_as_number (mode);
  if(pin_mode != INPUT && pin_mode != OUTPUT && pin_mode != INPUT_PULLUP && pin_mode != INPUT_PULLDOWN)
  {
    return jerry_throw_sz (JERRY_ERROR_RANGE, "Wrong argument 'mode' must be INPUT, OUTPUT, INPUT_PULLUP or INPUT_PULLDOWN.");
  }

  double pin_number = jerry_value_as_number (pin);
  pinMode (pin_number, pin_mode);

  return jerry_undefined ();
} /* js_pin_mode */

/**
 * Arduino: digitalWrite
 */
jerry_value_t
js_digital_write (const jerry_call_info_t *call_info_p, /**< call information */
                  const jerry_value_t args_p[], /**< function arguments */
                  const jerry_length_t args_cnt) /**< number of function arguments */
{
  JERRYX_UNUSED (call_info_p);
  if (args_cnt != 2)
  {
    return jerry_throw_sz (JERRY_ERROR_SYNTAX, "Wrong arguments count in 'digitalWrite' function.");
  }

  jerry_value_t pin = args_p[0];
  if(!jerry_value_is_number (pin ))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "Wrong argument 'pin' must be a number.");
  }

  jerry_value_t value = args_p[1];
  if(!jerry_value_is_number (value))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "Wrong argument 'value' must be a number.");
  }

  double pin_value = jerry_value_as_number (value);
  if(pin_value != HIGH && pin_value != LOW)
  {
    return jerry_throw_sz (JERRY_ERROR_RANGE, "Wrong argument 'value' must be HIGH or LOW.");
  }

  double pin_number = jerry_value_as_number (pin);
  digitalWrite (pin_number, pin_value);

  return jerry_undefined ();
} /* js_digital_write */

/**
 * Arduino: digitalRead
 */
jerry_value_t
js_digital_read (const jerry_call_info_t *call_info_p, /**< call information */
                 const jerry_value_t args_p[], /**< function arguments */
                 const jerry_length_t args_cnt) /**< number of function arguments */
{
  JERRYX_UNUSED (call_info_p);
  if (args_cnt != 1)
  {
    return jerry_throw_sz (JERRY_ERROR_SYNTAX, "Wrong arguments count in 'digitalWrite' function.");
  }

  jerry_value_t pin = args_p[0];
  if(!jerry_value_is_number (pin ))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "Wrong argument 'pin' must be a number.");
  }

  double pin_number = jerry_value_as_number (pin);
  int pin_status = digitalRead (pin_number);

  return jerry_number (pin_status);
} /* js_digital_read */

/**
 * Arduino: delay
 */
jerry_value_t
js_delay (const jerry_call_info_t *call_info_p, /**< call information */
          const jerry_value_t args_p[], /**< function arguments */
          const jerry_length_t args_cnt) /**< number of function arguments */
{
  JERRYX_UNUSED (call_info_p);
  if (args_cnt != 1)
  {
    return jerry_throw_sz (JERRY_ERROR_SYNTAX, "Wrong arguments count in 'delay' function.");
  }

  jerry_value_t value = args_p[0];
  if(!jerry_value_is_number (value))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "Wrong argument 'value' must be a number.");
  }

  delay (jerry_value_as_number (value));

  return jerry_undefined ();
} /* js_delay */

/**
 * Arduino: delayMicroseconds
 */
jerry_value_t
js_delay_microseconds (const jerry_call_info_t *call_info_p, /**< call information */
                       const jerry_value_t args_p[], /**< function arguments */
                       const jerry_length_t args_cnt) /**< number of function arguments */
{
  JERRYX_UNUSED (call_info_p);
  if (args_cnt != 1)
  {
    return jerry_throw_sz (JERRY_ERROR_SYNTAX, "Wrong arguments count in 'delayMicroseconds' function.");
  }

  jerry_value_t value = args_p[0];
  if(!jerry_value_is_number (value))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "Wrong argument 'value' must be a number.");
  }

  delayMicroseconds (jerry_value_as_number (value));

  return jerry_undefined ();
} /* js_delay_microseconds */

/**
 * Arduino: micros
 */
jerry_value_t
js_micros (const jerry_call_info_t *call_info_p, /**< call information */
           const jerry_value_t args_p[], /**< function arguments */
           const jerry_length_t args_cnt) /**< number of function arguments */
{
  JERRYX_UNUSED (call_info_p);
  if (args_cnt != 0)
  {
    return jerry_throw_sz (JERRY_ERROR_SYNTAX, "Wrong arguments count in 'micros' function.");
  }

  return jerry_number (micros ());
} /* js_micros */

/**
 * Arduino: millis
 */
jerry_value_t
js_millis (const jerry_call_info_t *call_info_p, /**< call information */
           const jerry_value_t args_p[], /**< function arguments */
           const jerry_length_t args_cnt) /**< number of function arguments */
{
  JERRYX_UNUSED (call_info_p);
  if (args_cnt != 0)
  {
    return jerry_throw_sz (JERRY_ERROR_SYNTAX, "Wrong arguments count in 'millis' function.");
  }

  return jerry_number (millis ());
} /* js_millis */
