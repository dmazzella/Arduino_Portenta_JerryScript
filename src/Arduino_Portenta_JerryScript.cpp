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
#include <unordered_map>

#include "Arduino_Portenta_JerryScript.h"

/**
 * Default implementation of jerry_port_fatal. Calls 'abort' if exit code is
 * non-zero, 'exit' otherwise.
 */
void JERRY_ATTR_WEAK
jerry_port_fatal(jerry_fatal_code_t code) /**< cause of error */
{
  exit((int)code);
} /* jerry_port_fatal */

/**
 * Implementation of jerry_port_current_time.
 *
 * @return current timer's counter value in milliseconds
 */
double
jerry_port_current_time(void)
{
  static uint64_t last_tick = 0;
  static time_t last_time = 0;
  static uint32_t skew = 0;

  uint64_t curr_tick = us_ticker_read(); /* The value is in microseconds. */
  time_t curr_time = time(NULL);         /*  The value is in seconds. */
  double result = curr_time * 1000;

  /* The us_ticker_read () has an overflow for each UINT_MAX microseconds
   * (~71 mins). For each overflow event the ticker-based clock is about 33
   * milliseconds fast. Without a timer thread the milliseconds part of the
   * time can be corrected if the difference of two get_current_time calls
   * are within the mentioned 71 mins. Above that interval we can assume
   * that the milliseconds part of the time is negligibe.
   */
  if (curr_time - last_time > (time_t)(((uint32_t)-1) / 1000000))
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
jerry_port_local_tza(double unix_ms)
{
  (void)unix_ms;

  /* We live in UTC. */
  return 0;
} /* jerry_port_local_tza */

/**
 * Default implementation of jerry_port_log. Prints log messages to stderr.
 */
void JERRY_ATTR_WEAK
jerry_port_log(const char *message_p) /**< message */
{
  while (*message_p != '\0')
  {
    if (*message_p == '\n')
    {
      /* add CR for proper display in serial monitors */
      fputc('\r', stderr);
    }

    fputc(*message_p++, stderr);
  }
} /* jerry_port_log */

/**
 * Default implementation of jerry_port_print_byte. Uses 'putchar' to
 * print a single character to standard output.
 */
void JERRY_ATTR_WEAK
jerry_port_print_byte(jerry_char_t byte) /**< the character to print */
{
  fputc(byte, stderr);
} /* jerry_port_print_byte */

/**
 * Default implementation of jerry_port_print_buffer. Uses 'jerry_port_print_byte' to
 * print characters of the input buffer.
 */
void JERRY_ATTR_WEAK
jerry_port_print_buffer(const jerry_char_t *buffer_p, /**< string buffer */
                        jerry_size_t buffer_size)     /**< string size*/
{
  for (jerry_size_t i = 0; i < buffer_size; i++)
  {
    jerry_port_print_byte(buffer_p[i]);
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
jerry_port_line_read(jerry_size_t *out_size_p)
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

      *out_size_p = data.length();
      return (jerry_char_t *)((*out_size_p > 0) ? (new String(data))->c_str() : NULL);
    }
  }

} /* jerry_port_line_read */

/**
 * Free a line buffer allocated by jerry_port_line_read
 *
 * @param buffer_p: buffer that has been allocated by jerry_port_line_read
 */
void JERRY_ATTR_WEAK
jerry_port_line_free(jerry_char_t *buffer_p)
{
  free(buffer_p);
} /* jerry_port_line_free */

/**
 * Determines the size of the given file.
 * @return size of the file
 */
static jerry_size_t
jerry_port_get_file_size(FILE *file_p) /**< opened file */
{
  fseek(file_p, 0, SEEK_END);
  long size = ftell(file_p);
  fseek(file_p, 0, SEEK_SET);

  return (jerry_size_t)size;
} /* jerry_port_get_file_size */

/**
 * Opens file with the given path and reads its source.
 * @return the source of the file
 */
jerry_char_t *JERRY_ATTR_WEAK
jerry_port_source_read(const char *file_name_p,  /**< file name */
                       jerry_size_t *out_size_p) /**< [out] read bytes */
{
  FILE *file_p = fopen(file_name_p, "rb");

  if (file_p == NULL)
  {
    return NULL;
  }

  jerry_size_t file_size = jerry_port_get_file_size(file_p);
  jerry_char_t *buffer_p = (jerry_char_t *)malloc(file_size);

  if (buffer_p == NULL)
  {
    fclose(file_p);
    return NULL;
  }

  size_t bytes_read = fread(buffer_p, 1u, file_size, file_p);

  if (bytes_read != file_size)
  {
    fclose(file_p);
    free(buffer_p);
    return NULL;
  }

  fclose(file_p);
  *out_size_p = (jerry_size_t)bytes_read;

  return buffer_p;
} /* jerry_port_source_read */

/**
 * Release the previously opened file's content.
 */
void JERRY_ATTR_WEAK
jerry_port_source_free(uint8_t *buffer_p) /**< buffer to free */
{
  free(buffer_p);
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
jerry_port_path_normalize(const jerry_char_t *path_p, /**< input path */
                          jerry_size_t path_size)     /**< size of the path */
{
  jerry_char_t *buffer_p = (jerry_char_t *)malloc(path_size + 1);

  if (buffer_p == NULL)
  {
    return NULL;
  }

  /* Also copy terminating zero byte. */
  memcpy(buffer_p, path_p, path_size + 1);

  return buffer_p;
} /* jerry_port_normalize_path */

/**
 * Free a path buffer returned by jerry_port_path_normalize.
 *
 * @param path_p: the path to free
 */
void JERRY_ATTR_WEAK
jerry_port_path_free(jerry_char_t *path_p)
{
  free(path_p);
} /* jerry_port_normalize_path */

/**
 * Computes the end of the directory part of a path.
 *
 * @return end of the directory part of a path.
 */
jerry_size_t JERRY_ATTR_WEAK
jerry_port_path_base(const jerry_char_t *path_p) /**< path */
{
  const jerry_char_t *basename_p = (jerry_char_t *)strrchr((char *)path_p, '/') + 1;

  if (basename_p == NULL)
  {
    return 0;
  }

  return (jerry_size_t)(basename_p - path_p);
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
jerry_port_context_alloc(size_t context_size)
{
  size_t total_size = context_size + JERRY_GLOBAL_HEAP_SIZE * 1024;
  current_context_p = (jerry_context_t *)malloc(total_size);

  return total_size;
} /* jerry_port_context_alloc */

/**
 * Get the current context.
 *
 * @return the pointer to the current context
 */
jerry_context_t *JERRY_ATTR_WEAK
jerry_port_context_get(void)
{
  return current_context_p;
} /* jerry_port_context_get */

/**
 * Free the currently allocated external context.
 */
void JERRY_ATTR_WEAK
jerry_port_context_free(void)
{
  free(current_context_p);
} /* jerry_port_context_free */

/*******************************************************************************
 *                                   Extra API                                 *
 ******************************************************************************/

static std::unordered_map<int, rtos::Thread *> jerryxx_scheduler_threads_map;
static rtos::Mutex jerryxx_scheduler_threads_mutex;

/**
 * Run JavaScript scheduler (user for switch setTimeout and setInterval threads).
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool jerryxx_scheduler_yield(void)
{
  yield();
} /* jerryxx_scheduler_yield */

/**
 * Cleanup scheduler thread in state Deleted.
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool jerryxx_cleanup_scheduler_map(void)
{
  std::unordered_map<int, rtos::Thread *>::const_iterator it;
  for (it = jerryxx_scheduler_threads_map.begin(); it != jerryxx_scheduler_threads_map.end(); it++)
  {
    int thid = it->first;
    rtos::Thread *thread = it->second;

    if (thread->get_state() == rtos::Thread::Deleted)
    {
      thread->terminate();
      jerryxx_scheduler_threads_mutex.lock();
      jerryxx_scheduler_threads_map.erase(thid);
      jerryxx_scheduler_threads_mutex.unlock();
    }
  }
} /* jerryxx_cleanup_scheduler_map */

/**
 * Register a JavaScript property in the global object.
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool jerryxx_register_global_property(const char *name_p,  /**< name of the property */
                                      jerry_value_t value, /**< value of the property */
                                      bool free_value)     /**< take ownership of the value */
{
  jerry_value_t global_obj_val = jerry_current_realm();
  jerry_value_t property_name_val = jerry_string_sz(name_p);

  jerry_value_t result_val = jerry_object_set(global_obj_val, property_name_val, value);
  bool result = jerry_value_is_true(result_val);

  jerry_value_free(result_val);
  jerry_value_free(property_name_val);
  jerry_value_free(global_obj_val);

  if (free_value)
  {
    jerry_value_free(value);
  }

  return result;
} /* jerryxx_register_global_property */

/**
 * Register Extra API into JavaScript global object.
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool jerryxx_register_extra_api(void)
{
  bool ret = false;

  /* Register the print function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global("print", jerryx_handler_print));

  /* Register the setTimeout function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global("setTimeout", js_set_timeout));

  /* Register the clearTimeout function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global("clearTimeout", js_clear_timeout));

  /* Register the setInterval function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global("setInterval", js_set_interval));

  /* Register the clearInterval function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global("clearInterval", js_clear_interval));

cleanup:
  return ret;
} /* jerryxx_register_extra_api */

/**
 * Javascript: setTimeout
 */
JERRYXX_DECLARE_FUNCTION(set_timeout)
{
  JERRYX_UNUSED(call_info_p);
  jerry_value_t callback_fn = 0;
  uint32_t delay_time = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_function(&callback_fn, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&delay_time, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_OPTIONAL),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  jerryxx_cleanup_scheduler_map();

  if (jerryxx_scheduler_threads_map.size() < JERRYXX_MAX_THREADS_NUMBER)
  {
    rtos::Thread *thread = new rtos::Thread;
    thread->start([callback_fn, delay_time](void) -> void
                  {
      jerry_value_t callback_fn_copy = jerry_value_copy (callback_fn);

      rtos::ThisThread::sleep_for(abs (delay_time));

      jerry_value_t global_obj_val = jerry_current_realm ();
      jerry_value_t result_val = jerry_call (callback_fn_copy, global_obj_val, NULL, 0);
      jerry_value_free (result_val);
      jerry_value_free (global_obj_val);
      jerry_value_free (callback_fn_copy); });
    int idx = (int)thread->get_id();

    jerryxx_scheduler_threads_mutex.lock();
    jerryxx_scheduler_threads_map.insert(std::make_pair(idx, thread));
    jerryxx_scheduler_threads_mutex.unlock();

    return jerry_number(idx);
  }

  return jerry_throw_sz(JERRY_ERROR_RANGE, "No scheduler slot free found.");
} /* js_set_timeout */

/**
 * Javascript: clearTimeout
 */
JERRYXX_DECLARE_FUNCTION(clear_timeout)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t timeout_id = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&timeout_id, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  std::unordered_map<int, rtos::Thread *>::const_iterator got = jerryxx_scheduler_threads_map.find(timeout_id);
  if (got != jerryxx_scheduler_threads_map.end())
  {
    int idx = got->first;
    rtos::Thread *thread = got->second;
    thread->terminate();

    jerryxx_scheduler_threads_mutex.lock();
    jerryxx_scheduler_threads_map.erase(idx);
    jerryxx_scheduler_threads_mutex.unlock();
  }

  jerryxx_cleanup_scheduler_map();

  return jerry_undefined();
} /* js_clear_timeout */

/**
 * Javascript: setInterval
 */
JERRYXX_DECLARE_FUNCTION(set_interval)
{
  JERRYX_UNUSED(call_info_p);
  jerry_value_t callback_fn = 0;
  uint32_t delay_time = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_function(&callback_fn, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&delay_time, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_OPTIONAL),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  jerryxx_cleanup_scheduler_map();

  if (jerryxx_scheduler_threads_map.size() < JERRYXX_MAX_THREADS_NUMBER)
  {
    rtos::Thread *thread = new rtos::Thread;
    thread->start([callback_fn, delay_time](void) -> void
                  {
      while(true)
      {
        jerry_value_t callback_fn_copy = jerry_value_copy (callback_fn);

        rtos::ThisThread::sleep_for(abs (delay_time));

        jerry_value_t global_obj_val = jerry_current_realm ();
        jerry_value_t result_val = jerry_call (callback_fn_copy, global_obj_val, NULL, 0);
        jerry_value_free (result_val);
        jerry_value_free (global_obj_val);
        jerry_value_free (callback_fn_copy);
      } });
    int idx = (int)thread->get_id();

    jerryxx_scheduler_threads_mutex.lock();
    jerryxx_scheduler_threads_map.insert(std::make_pair(idx, thread));
    jerryxx_scheduler_threads_mutex.unlock();

    return jerry_number(idx);
  }

  return jerry_throw_sz(JERRY_ERROR_RANGE, "No scheduler slot free found.");
} /* js_set_interval */

/**
 * Javascript: clearInterval
 */
JERRYXX_DECLARE_FUNCTION(clear_interval)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t interval_id = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&interval_id, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  std::unordered_map<int, rtos::Thread *>::const_iterator got = jerryxx_scheduler_threads_map.find(interval_id);
  if (got != jerryxx_scheduler_threads_map.end())
  {
    int idx = got->first;
    rtos::Thread *thread = got->second;
    thread->terminate();

    jerryxx_scheduler_threads_mutex.lock();
    jerryxx_scheduler_threads_map.erase(idx);
    jerryxx_scheduler_threads_mutex.unlock();
  }

  jerryxx_cleanup_scheduler_map();

  return jerry_undefined();
} /* js_clear_interval */

/*******************************************************************************
 *                                  Arduino API                                *
 ******************************************************************************/

/**
 * Register Arduino API into JavaScript global object.
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool jerryxx_register_arduino_api(void)
{
  bool ret = false;

  /* Register Constants in the global object */

  /* Bit Order */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LSBFIRST", jerry_number(LSBFIRST), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("MSBFIRST", jerry_number(MSBFIRST), true));

  /* PINs Status */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("HIGH", jerry_number(HIGH), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LOW", jerry_number(LOW), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("CHANGE", jerry_number(CHANGE), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("RISING", jerry_number(RISING), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("FALLING", jerry_number(FALLING), true));

  /* PINs Mode */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("INPUT", jerry_number(INPUT), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("OUTPUT", jerry_number(OUTPUT), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("INPUT_PULLUP", jerry_number(INPUT_PULLUP), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("INPUT_PULLDOWN", jerry_number(INPUT_PULLDOWN), true));
#if defined(OUTPUT_OPENDRAIN)
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("OUTPUT_OPENDRAIN", jerry_number(OUTPUT_OPENDRAIN), true));
#endif

  /* LEDs */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("PIN_LED", jerry_number(PIN_LED), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LED_BUILTIN", jerry_number(LED_BUILTIN), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LEDR", jerry_number(LEDR), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LEDG", jerry_number(LEDG), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LEDB", jerry_number(LEDB), true));

  /* Analog pins */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A0", jerry_number(A0), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A1", jerry_number(A1), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A2", jerry_number(A2), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A3", jerry_number(A3), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A4", jerry_number(A4), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A5", jerry_number(A5), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A6", jerry_number(A6), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A7", jerry_number(A7), true));

  /* Digital pins */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D0", jerry_number(D0), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D1", jerry_number(D1), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D2", jerry_number(D2), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D3", jerry_number(D3), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D4", jerry_number(D4), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D5", jerry_number(D5), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D6", jerry_number(D6), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D7", jerry_number(D7), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D8", jerry_number(D8), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D9", jerry_number(D9), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D10", jerry_number(D10), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D11", jerry_number(D11), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D12", jerry_number(D12), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D13", jerry_number(D13), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D14", jerry_number(D14), true));
/* Pin D15, D16, D17 and D18 cannot be used as digital pin.*/
/*
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D15", jerry_number (D15), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D16", jerry_number (D16), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D17", jerry_number (D17), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D18", jerry_number (D18), true));
 */
#if defined(CORE_CM7)
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D19", jerry_number(D19), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D20", jerry_number(D20), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D21", jerry_number(D21), true));
#endif

  /* Register Functions in the global object */

  /* Digital I/O */
  JERRYXX_BOOL_CHK(jerryx_register_global("pinMode", js_pin_mode));
  JERRYXX_BOOL_CHK(jerryx_register_global("digitalWrite", js_digital_write));
  JERRYXX_BOOL_CHK(jerryx_register_global("digitalRead", js_digital_read));
  /* Time */
  JERRYXX_BOOL_CHK(jerryx_register_global("delay", js_delay));
  JERRYXX_BOOL_CHK(jerryx_register_global("delayMicroseconds", js_delay_microseconds));
  JERRYXX_BOOL_CHK(jerryx_register_global("micros", js_micros));
  JERRYXX_BOOL_CHK(jerryx_register_global("millis", js_millis));
  /* Math */
  JERRYXX_BOOL_CHK(jerryx_register_global("constrain", js_constrain));
  JERRYXX_BOOL_CHK(jerryx_register_global("map", js_map));
  JERRYXX_BOOL_CHK(jerryx_register_global("sq", js_sq));
  /* 'abs', 'max', 'min', 'pow', 'sqrt' functions are supported via javascript 'Math' module */
  /* Trigonometry */
  /* 'cos', 'sin' and 'tan' functions are supported via javascript 'Math' module */
  /* Random Numbers */
  JERRYXX_BOOL_CHK(jerryx_register_global("randomSeed", js_random_seed));
  JERRYXX_BOOL_CHK(jerryx_register_global("random", js_random));
  /* Bits and Bytes */
  JERRYXX_BOOL_CHK(jerryx_register_global("bit", js_bit));
  JERRYXX_BOOL_CHK(jerryx_register_global("bitClear", js_bit_clear));
  JERRYXX_BOOL_CHK(jerryx_register_global("bitRead", js_bit_read));
  JERRYXX_BOOL_CHK(jerryx_register_global("bitSet", js_bit_set));
  JERRYXX_BOOL_CHK(jerryx_register_global("bitWrite", js_bit_write));
  JERRYXX_BOOL_CHK(jerryx_register_global("highByte", js_high_byte));
  JERRYXX_BOOL_CHK(jerryx_register_global("lowByte", js_low_byte));
  /* Analog I/O */
  JERRYXX_BOOL_CHK(jerryx_register_global("analogRead", js_analog_read));
  JERRYXX_BOOL_CHK(jerryx_register_global("analogWrite", js_analog_write));
  JERRYXX_BOOL_CHK(jerryx_register_global("analogReadResolution", js_analog_read_resolution));
  JERRYXX_BOOL_CHK(jerryx_register_global("analogWriteResolution", js_analog_write_resolution));
  /* Advanced I/O */
  JERRYXX_BOOL_CHK(jerryx_register_global("noTone", js_no_tone));
  JERRYXX_BOOL_CHK(jerryx_register_global("pulseIn", js_pulse_in));
  JERRYXX_BOOL_CHK(jerryx_register_global("pulseInLong", js_pulse_in_long));
  JERRYXX_BOOL_CHK(jerryx_register_global("shiftIn", js_shift_in));
  JERRYXX_BOOL_CHK(jerryx_register_global("shiftOut", js_shift_out));
  JERRYXX_BOOL_CHK(jerryx_register_global("tone", js_tone));
  /* External Interrupts */
  JERRYXX_BOOL_CHK(jerryx_register_global("attachInterrupt", js_attach_interrupt));
  JERRYXX_BOOL_CHK(jerryx_register_global("detachInterrupt", js_detach_interrupt));
  /* Interrupts */
  JERRYXX_BOOL_CHK(jerryx_register_global("interrupts", js_interrupts));
  JERRYXX_BOOL_CHK(jerryx_register_global("noInterrupts", js_no_interrupts));
  /* Characters */
  JERRYXX_BOOL_CHK(jerryx_register_global("isAlpha", js_is_alpha));
  JERRYXX_BOOL_CHK(jerryx_register_global("isAlphaNumeric", js_is_alpha_numeric));
  JERRYXX_BOOL_CHK(jerryx_register_global("isAscii", js_is_ascii));
  JERRYXX_BOOL_CHK(jerryx_register_global("isControl", js_is_control));
  JERRYXX_BOOL_CHK(jerryx_register_global("isDigit", js_is_digit));
  JERRYXX_BOOL_CHK(jerryx_register_global("isGraph", js_is_graph));
  JERRYXX_BOOL_CHK(jerryx_register_global("isHexadecimalDigit", js_is_hexadecimal_digit));
  JERRYXX_BOOL_CHK(jerryx_register_global("isLowerCase", js_is_lower_case));
  JERRYXX_BOOL_CHK(jerryx_register_global("isPrintable", js_is_printable));
  JERRYXX_BOOL_CHK(jerryx_register_global("isPunct", js_is_punct));
  JERRYXX_BOOL_CHK(jerryx_register_global("isSpace", js_is_space));
  JERRYXX_BOOL_CHK(jerryx_register_global("isUpperCase", js_is_upper_case));
  JERRYXX_BOOL_CHK(jerryx_register_global("isWhitespace", js_is_whitespace));

  /* Register Objects in the global object */

  /* Communication */
  /* Serial */
  /* SPI */
  /* Stream */
  /* Wire */

cleanup:
  return ret;
} /* jerryxx_register_arduino_api */

/**
 * Arduino: pinMode
 */
JERRYXX_DECLARE_FUNCTION(pin_mode)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t pin = 0;
  uint32_t mode = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&pin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&mode, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  if (mode != INPUT && mode != OUTPUT && mode != INPUT_PULLUP && mode != INPUT_PULLDOWN)
  {
    return jerry_throw_sz(JERRY_ERROR_RANGE, "Wrong argument 'mode' must be INPUT, OUTPUT, INPUT_PULLUP or INPUT_PULLDOWN.");
  }

  pinMode(pin, mode);

  return jerry_undefined();
} /* js_pin_mode */

/**
 * Arduino: digitalWrite
 */
JERRYXX_DECLARE_FUNCTION(digital_write)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t pin = 0;
  uint32_t value = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&pin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&value, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  if (value != HIGH && value != LOW)
  {
    return jerry_throw_sz(JERRY_ERROR_RANGE, "Wrong argument 'value' must be HIGH or LOW.");
  }

  digitalWrite(pin, value);

  return jerry_undefined();
} /* js_digital_write */

/**
 * Arduino: digitalRead
 */
JERRYXX_DECLARE_FUNCTION(digital_read)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t pin = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&pin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(digitalRead(pin));
} /* js_digital_read */

/**
 * Arduino: delay
 */
JERRYXX_DECLARE_FUNCTION(delay)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t value = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&value, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  delay(value);

  return jerry_undefined();
} /* js_delay */

/**
 * Arduino: delayMicroseconds
 */
JERRYXX_DECLARE_FUNCTION(delay_microseconds)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t value = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&value, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  delayMicroseconds(value);

  return jerry_undefined();
} /* js_delay_microseconds */

/**
 * Arduino: micros
 */
JERRYXX_DECLARE_FUNCTION(micros)
{
  JERRYX_UNUSED(call_info_p);
  JERRYX_UNUSED(args_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 0, "Wrong arguments count");

  return jerry_number(micros());
} /* js_micros */

/**
 * Arduino: millis
 */
JERRYXX_DECLARE_FUNCTION(millis)
{
  JERRYX_UNUSED(call_info_p);
  JERRYX_UNUSED(args_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 0, "Wrong arguments count");

  return jerry_number(millis());
} /* js_millis */

/**
 * Arduino: randomSeed
 */
JERRYXX_DECLARE_FUNCTION(random_seed)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t seed = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&seed, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  randomSeed(seed);

  return jerry_undefined();
} /* js_random_seed */

/**
 * Arduino: random
 */
JERRYXX_DECLARE_FUNCTION(random)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t a = 0;
  uint32_t b = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&a, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&b, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_OPTIONAL),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  if (args_cnt == 1)
  {
    return jerry_number(random(a));
  }
  else if (args_cnt == 2)
  {
    return jerry_number(random(a, b));
  }

  return jerry_number(0);
} /* js_random */

/**
 * Arduino: analogRead
 */
JERRYXX_DECLARE_FUNCTION(analog_read)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t pin = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&pin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(analogRead(pin));
} /* js_analog_read */

/**
 * Arduino: analogWrite
 */
JERRYXX_DECLARE_FUNCTION(analog_write)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t pin = 0;
  uint32_t value = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&pin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&value, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  analogWrite(pin, value);

  return jerry_undefined();
} /* js_analog_write */

/**
 * Arduino: analogReadResolution
 */
JERRYXX_DECLARE_FUNCTION(analog_read_resolution)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t bits = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&bits, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  analogReadResolution(bits);

  return jerry_undefined();
} /* js_analog_read_resolution */

/**
 * Arduino: analogWriteResolution
 */
JERRYXX_DECLARE_FUNCTION(analog_write_resolution)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t bits = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&bits, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  analogWriteResolution(bits);

  return jerry_undefined();
} /* js_analog_write_resolution */

/**
 * Arduino: interrupts
 */
JERRYXX_DECLARE_FUNCTION(interrupts)
{
  JERRYX_UNUSED(call_info_p);
  JERRYX_UNUSED(args_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 0, "Wrong arguments count");

  interrupts();

  return jerry_undefined();
} /* js_interrupts */

/**
 * Arduino: noInterrupts
 */
JERRYXX_DECLARE_FUNCTION(no_interrupts)
{
  JERRYX_UNUSED(call_info_p);
  JERRYX_UNUSED(args_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 0, "Wrong arguments count");

  noInterrupts();

  return jerry_undefined();
} /* js_no_interrupts */

/**
 * Arduino: attachInterrupt
 */
JERRYXX_DECLARE_FUNCTION(attach_interrupt)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t pin = 0;
  jerry_value_t callback_fn = 0;
  uint32_t mode = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&pin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_function(&callback_fn, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&mode, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  auto func = [](void *callback_fn) -> void
  {
    jerry_value_t global_obj_val = jerry_current_realm();
    jerry_value_free(jerry_call((jerry_value_t)callback_fn, global_obj_val, NULL, 0));
    jerry_value_free(global_obj_val);
  };

  attachInterruptParam((pin_size_t)pin, (voidFuncPtrParam)func, (PinStatus)mode, (void *)callback_fn);

  return jerry_undefined();
} /* js_attach_interrupt */

/**
 * Arduino: detachInterrupt
 */
JERRYXX_DECLARE_FUNCTION(detach_interrupt)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t pin = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&pin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  detachInterrupt(pin);

  return jerry_undefined();
} /* js_detach_interrupt */

/**
 * Arduino: noTone
 */
JERRYXX_DECLARE_FUNCTION(no_tone)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t pin = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&pin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  noTone(pin);

  return jerry_undefined();
} /* js_no_tone */

/**
 * Arduino: pulseIn
 */
JERRYXX_DECLARE_FUNCTION(pulse_in)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t pin = 0;
  uint32_t value = 0;
  uint32_t timeout = 1000000;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&pin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&value, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&timeout, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_OPTIONAL),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(pulseIn(pin, value, timeout));
} /* js_pulse_in */

/**
 * Arduino: pulseInLong
 */
JERRYXX_DECLARE_FUNCTION(pulse_in_long)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t pin = 0;
  uint32_t value = 0;
  uint32_t timeout = 1000000;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&pin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&value, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&timeout, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_OPTIONAL),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(pulseInLong(pin, value, timeout));
} /* js_pulse_in_long */

/**
 * Arduino: shiftIn
 */
JERRYXX_DECLARE_FUNCTION(shift_in)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t dataPin = 0;
  uint32_t clockPin = 0;
  uint32_t bitOrder = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&dataPin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&clockPin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&bitOrder, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  if (bitOrder != MSBFIRST && bitOrder != LSBFIRST)
  {
    return jerry_throw_sz(JERRY_ERROR_RANGE, "Wrong argument 'bitOrder' must be MSBFIRST or LSBFIRST.");
  }

  return jerry_number(shiftIn((pin_size_t)dataPin, (pin_size_t)clockPin, (BitOrder)bitOrder));
} /* js_shift_in */

/**
 * Arduino: shiftOut
 */
JERRYXX_DECLARE_FUNCTION(shift_out)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t dataPin = 0;
  uint32_t clockPin = 0;
  uint32_t bitOrder = 0;
  uint32_t value = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&dataPin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&clockPin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&bitOrder, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&value, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  if (bitOrder != MSBFIRST && bitOrder != LSBFIRST)
  {
    return jerry_throw_sz(JERRY_ERROR_RANGE, "Wrong argument 'bitOrder' must be MSBFIRST or LSBFIRST.");
  }

  shiftOut((pin_size_t)dataPin, (pin_size_t)clockPin, (BitOrder)bitOrder, value);

  return jerry_undefined();
} /* js_shift_out */

/**
 * Arduino: tone
 */
JERRYXX_DECLARE_FUNCTION(tone)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t pin = 0;
  uint32_t frequency = 0;
  uint32_t duration = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&pin, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&frequency, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&duration, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_OPTIONAL),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  tone(pin, frequency, duration);

  return jerry_undefined();
} /* js_tone */

/**
 * Arduino: bit
 */
JERRYXX_DECLARE_FUNCTION(bit)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t n = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&n, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(bit(n));
} /* js_bit */

/**
 * Arduino: bitClear
 */
JERRYXX_DECLARE_FUNCTION(bit_clear)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t x = 0;
  uint32_t n = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&n, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(bitClear(x, n));
} /* js_bit_clear */

/**
 * Arduino: bitRead
 */
JERRYXX_DECLARE_FUNCTION(bit_read)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t x = 0;
  uint32_t n = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&n, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(bitRead(x, n));
} /* js_bit_read */

/**
 * Arduino: bitSet
 */
JERRYXX_DECLARE_FUNCTION(bit_set)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t x = 0;
  uint32_t n = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&n, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  bitSet(x, n);

  return jerry_undefined();
} /* js_bit_set */

/**
 * Arduino: bitWrite
 */
JERRYXX_DECLARE_FUNCTION(bit_write)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t x = 0;
  uint32_t n = 0;
  uint32_t b = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&n, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&b, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  bitWrite(x, n, b);

  return jerry_undefined();
} /* js_bit_write */

/**
 * Arduino: highByte
 */
JERRYXX_DECLARE_FUNCTION(high_byte)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(highByte(x));
} /* js_low_byte */

/**
 * Arduino: lowByte
 */
JERRYXX_DECLARE_FUNCTION(low_byte)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(lowByte(x));
} /* js_low_byte */

/**
 * Arduino: constrain
 */
JERRYXX_DECLARE_FUNCTION(constrain)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t x = 0;
  uint32_t a = 0;
  uint32_t b = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&a, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&b, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(constrain(x, a, b));
} /* js_constrain */

/**
 * Arduino: map
 */
JERRYXX_DECLARE_FUNCTION(map)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t value = 0;
  uint32_t fromLow = 0;
  uint32_t fromHigh = 0;
  uint32_t toLow = 0;
  uint32_t toHigh = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&value, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&fromLow, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&fromHigh, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&toLow, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
          jerryx_arg_uint32(&toHigh, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(map(value, fromLow, fromHigh, toLow, toHigh));
} /* js_map */

/**
 * Arduino: sq
 */
JERRYXX_DECLARE_FUNCTION(sq)
{
  JERRYX_UNUSED(call_info_p);
  uint32_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_uint32(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(sq(x));
} /* js_sq */

/**
 * Arduino: isAlpha
 */
JERRYXX_DECLARE_FUNCTION(is_alpha)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isAlpha(x));
} /* js_is_alpha */

/**
 * Arduino: isAlphaNumeric
 */
JERRYXX_DECLARE_FUNCTION(is_alpha_numeric)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isAlphaNumeric(x));
} /* js_is_alpha_numeric */

/**
 * Arduino: isAscii
 */
JERRYXX_DECLARE_FUNCTION(is_ascii)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isAscii(x));
} /* js_is_ascii */

/**
 * Arduino: isControl
 */
JERRYXX_DECLARE_FUNCTION(is_control)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isControl(x));
} /* js_is_control */

/**
 * Arduino: isDigit
 */
JERRYXX_DECLARE_FUNCTION(is_digit)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isDigit(x));
} /* js_is_digit */

/**
 * Arduino: isGraph
 */
JERRYXX_DECLARE_FUNCTION(is_graph)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isGraph(x));
} /* js_is_graph */

/**
 * Arduino: isHexadecimalDigit
 */
JERRYXX_DECLARE_FUNCTION(is_hexadecimal_digit)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isHexadecimalDigit(x));
} /* js_is_hexadecimal_digit */

/**
 * Arduino: isLowerCase
 */
JERRYXX_DECLARE_FUNCTION(is_lower_case)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isLowerCase(x));
} /* js_is_lower_case */

/**
 * Arduino: isPrintable
 */
JERRYXX_DECLARE_FUNCTION(is_printable)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isPrintable(x));
} /* js_is_printable */

/**
 * Arduino: isPunct
 */
JERRYXX_DECLARE_FUNCTION(is_punct)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isPunct(x));
} /* js_is_punct */

/**
 * Arduino: isSpace
 */
JERRYXX_DECLARE_FUNCTION(is_space)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isSpace(x));
} /* js_is_space */

/**
 * Arduino: isUpperCase
 */
JERRYXX_DECLARE_FUNCTION(is_upper_case)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isUpperCase(x));
} /* js_is_upper_case */

/**
 * Arduino: isWhitespace
 */
JERRYXX_DECLARE_FUNCTION(is_whitespace)
{
  JERRYX_UNUSED(call_info_p);
  int8_t x = 0;

  const jerryx_arg_t mapping[] =
      {
          jerryx_arg_int8(&x, JERRYX_ARG_CEIL, JERRYX_ARG_NO_CLAMP, JERRYX_ARG_NO_COERCE, JERRYX_ARG_REQUIRED),
      };

  const jerry_value_t rv = jerryx_arg_transform_args(args_p, args_cnt, mapping, JERRYXX_ARRAY_SIZE(mapping));
  if (jerry_value_is_exception(rv))
  {
    return rv;
  }

  return jerry_number(isWhitespace(x));
} /* js_is_whitespace */
