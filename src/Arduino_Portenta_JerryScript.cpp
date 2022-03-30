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
 *                                   Extra API                                 *
 ******************************************************************************/

static std::unordered_map<int, rtos::Thread*> jerryxx_scheduler_threads_map;
static rtos::Mutex jerryxx_scheduler_threads_mutex;

/**
 * Run JavaScript scheduler (user for switch setTimeout and setInterval threads).
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool
jerryxx_scheduler_yield(void)
{
  yield();
} /* jerryxx_scheduler_yield */

/**
 * Cleanup scheduler thread in state Deleted.
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool
jerryxx_cleanup_scheduler_map(void)
{
  std::unordered_map<int, rtos::Thread*>::const_iterator it;
  for (it = jerryxx_scheduler_threads_map.begin(); it != jerryxx_scheduler_threads_map.end(); it++)
  {
    int thid = it->first;
    rtos::Thread* thread = it->second;

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
 * Register Extra API into JavaScript global object.
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool
jerryxx_register_extra_api(void)
{
  bool ret = false;

  /* Register the print function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global ("print", jerryx_handler_print));

  /* Register the setTimeout function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global ("setTimeout", js_set_timeout));

  /* Register the clearTimeout function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global ("clearTimeout", js_clear_timeout));

  /* Register the setInterval function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global ("setInterval", js_set_interval));

  /* Register the clearInterval function in the global object */
  JERRYXX_BOOL_CHK(jerryx_register_global ("clearInterval", js_clear_interval));

cleanup:
  return ret;
} /* jerryxx_register_extra_api */

/**
 * Javascript: setTimeout
 */
JERRYXX_DECLARE_FUNCTION(set_timeout)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX((args_cnt != 1 && args_cnt != 2), "Wrong arguments count in 'setTimeout' function.");

  jerry_value_t callback_fn = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_function (callback_fn), "Wrong argument 'callback' must be a function.");

  jerry_value_t delay_time = (args_cnt == 2 ? args_p[1] : jerry_number (0));
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (delay_time), "Wrong argument 'delay' must be a number.");

  jerryxx_cleanup_scheduler_map();

  if (jerryxx_scheduler_threads_map.size() < JERRYXX_MAX_THREADS_NUMBER)
  {
    rtos::Thread * thread = new rtos::Thread;
    thread->start([callback_fn, delay_time](void) -> void {
      jerry_value_t callback_fn_copy = jerry_value_copy (callback_fn);
      jerry_value_t delay_time_copy = jerry_value_copy (delay_time);

      rtos::ThisThread::sleep_for(abs (jerry_value_as_number (delay_time_copy)));

      jerry_value_t global_obj_val = jerry_current_realm ();
      jerry_value_t result_val = jerry_call (callback_fn_copy, global_obj_val, NULL, 0);
      jerry_value_free (result_val);
      jerry_value_free (global_obj_val);
      jerry_value_free (callback_fn_copy);
      jerry_value_free (delay_time_copy);
    });
    int idx = (int)thread->get_id();

    jerryxx_scheduler_threads_mutex.lock();
    jerryxx_scheduler_threads_map.insert(std::make_pair(idx, thread));
    jerryxx_scheduler_threads_mutex.unlock();

    return jerry_number (idx);
  }

  return jerry_throw_sz (JERRY_ERROR_RANGE, "No scheduler slot free found.");
} /* js_set_timeout */

/**
 * Javascript: clearTimeout
 */
JERRYXX_DECLARE_FUNCTION(clear_timeout)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 1, "Wrong arguments count in 'clearTimeout' function.");

  jerry_value_t timeout_id = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (timeout_id), "Wrong argument 'timeoutId' must be a number.");

  int tid = jerry_value_as_number (timeout_id);
  
  std::unordered_map<int, rtos::Thread*>::const_iterator got = jerryxx_scheduler_threads_map.find (tid);
  if ( got != jerryxx_scheduler_threads_map.end() )
  {
    int idx = got->first;
    rtos::Thread* thread = got->second;
    thread->terminate();

    jerryxx_scheduler_threads_mutex.lock();
    jerryxx_scheduler_threads_map.erase(idx);
    jerryxx_scheduler_threads_mutex.unlock();
  }

  jerryxx_cleanup_scheduler_map();

  return jerry_undefined ();
} /* js_clear_timeout */

/**
 * Javascript: setInterval
 */
JERRYXX_DECLARE_FUNCTION(set_interval)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX((args_cnt != 1 && args_cnt != 2), "Wrong arguments count in 'setInterval' function.");

  jerry_value_t callback_fn = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_function (callback_fn), "Wrong argument 'callback' must be a function.");

  jerry_value_t delay_time = (args_cnt == 2 ? args_p[1] : jerry_number (0));
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (delay_time), "Wrong argument 'delay' must be a number.");

  jerryxx_cleanup_scheduler_map();

  if (jerryxx_scheduler_threads_map.size() < JERRYXX_MAX_THREADS_NUMBER)
  {
    rtos::Thread * thread = new rtos::Thread;
    thread->start([callback_fn, delay_time](void) -> void {
      while(true)
      {
        jerry_value_t callback_fn_copy = jerry_value_copy (callback_fn);
        jerry_value_t delay_time_copy = jerry_value_copy (delay_time);

        rtos::ThisThread::sleep_for(abs (jerry_value_as_number (delay_time_copy)));

        jerry_value_t global_obj_val = jerry_current_realm ();
        jerry_value_t result_val = jerry_call (callback_fn_copy, global_obj_val, NULL, 0);
        jerry_value_free (result_val);
        jerry_value_free (global_obj_val);
        jerry_value_free (callback_fn_copy);
        jerry_value_free (delay_time_copy);
      }
    });
    int idx = (int)thread->get_id();

    jerryxx_scheduler_threads_mutex.lock();
    jerryxx_scheduler_threads_map.insert(std::make_pair(idx, thread));
    jerryxx_scheduler_threads_mutex.unlock();

    return jerry_number (idx);
  }

  return jerry_throw_sz (JERRY_ERROR_RANGE, "No scheduler slot free found.");
} /* js_set_interval */

/**
 * Javascript: clearInterval
 */
JERRYXX_DECLARE_FUNCTION(clear_interval)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 1, "Wrong arguments count in 'clearInterval' function.");

  jerry_value_t interval_id = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (interval_id), "Wrong argument 'intervalId' must be a number.");

  int iid = jerry_value_as_number (interval_id);
  std::unordered_map<int, rtos::Thread*>::const_iterator got = jerryxx_scheduler_threads_map.find (iid);
  if ( got != jerryxx_scheduler_threads_map.end() )
  {
    int idx = got->first;
    rtos::Thread* thread = got->second;
    thread->terminate();

    jerryxx_scheduler_threads_mutex.lock();
    jerryxx_scheduler_threads_map.erase(idx);
    jerryxx_scheduler_threads_mutex.unlock();
  }

  jerryxx_cleanup_scheduler_map();

  return jerry_undefined ();
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
bool
jerryxx_register_arduino_api(void)
{
  bool ret = false;

  /* Register Constants in the global object */

  /* PINs Status */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("HIGH", jerry_number (HIGH), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LOW", jerry_number (LOW), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("CHANGE", jerry_number (CHANGE), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("RISING", jerry_number (RISING), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("FALLING", jerry_number (FALLING), true));

  /* PINs Mode */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("INPUT", jerry_number (INPUT), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("OUTPUT", jerry_number (OUTPUT), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("INPUT_PULLUP", jerry_number (INPUT_PULLUP), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("INPUT_PULLDOWN", jerry_number (INPUT_PULLDOWN), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("OUTPUT_OPENDRAIN", jerry_number (OUTPUT_OPENDRAIN), true));

  /* LEDs */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("PIN_LED", jerry_number (PIN_LED), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LED_BUILTIN", jerry_number (LED_BUILTIN), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LEDR", jerry_number (LEDR), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LEDG", jerry_number (LEDG), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("LEDB", jerry_number (LEDB), true));

  /* Analog pins */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A0", jerry_number (A0), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A1", jerry_number (A1), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A2", jerry_number (A2), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A3", jerry_number (A3), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A4", jerry_number (A4), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A5", jerry_number (A5), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A6", jerry_number (A6), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("A7", jerry_number (A7), true));

  /* Digital pins */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D0", jerry_number (D0), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D1", jerry_number (D1), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D2", jerry_number (D2), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D3", jerry_number (D3), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D4", jerry_number (D4), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D5", jerry_number (D5), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D6", jerry_number (D6), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D7", jerry_number (D7), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D8", jerry_number (D8), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D9", jerry_number (D9), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D10", jerry_number (D10), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D11", jerry_number (D11), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D12", jerry_number (D12), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D13", jerry_number (D13), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D14", jerry_number (D14), true));
  /* Pin D15, D16, D17 and D18 cannot be used as digital pin.*/
  /*
    JERRYXX_BOOL_CHK(jerryxx_register_global_property("D15", jerry_number (D15), true));
    JERRYXX_BOOL_CHK(jerryxx_register_global_property("D16", jerry_number (D16), true));
    JERRYXX_BOOL_CHK(jerryxx_register_global_property("D17", jerry_number (D17), true));
    JERRYXX_BOOL_CHK(jerryxx_register_global_property("D18", jerry_number (D18), true));
   */
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D19", jerry_number (D19), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D20", jerry_number (D20), true));
  JERRYXX_BOOL_CHK(jerryxx_register_global_property("D21", jerry_number (D21), true));

  /* Register Functions in the global object */

  /* Digital I/O */
  JERRYXX_BOOL_CHK(jerryx_register_global ("pinMode", js_pin_mode));
  JERRYXX_BOOL_CHK(jerryx_register_global ("digitalWrite", js_digital_write));
  JERRYXX_BOOL_CHK(jerryx_register_global ("digitalRead", js_digital_read));
  /* Time */
  JERRYXX_BOOL_CHK(jerryx_register_global ("delay", js_delay));
  JERRYXX_BOOL_CHK(jerryx_register_global ("delayMicroseconds", js_delay_microseconds));
  JERRYXX_BOOL_CHK(jerryx_register_global ("micros", js_micros));
  JERRYXX_BOOL_CHK(jerryx_register_global ("millis", js_millis));
  /* Math */
  /* Trigonometry */
  /* Random Numbers */
  JERRYXX_BOOL_CHK(jerryx_register_global ("randomSeed", js_random_seed));
  JERRYXX_BOOL_CHK(jerryx_register_global ("random", js_random));
  /* Bits and Bytes */
  /* Analog I/O */
  JERRYXX_BOOL_CHK(jerryx_register_global ("analogRead", js_analog_read));
  JERRYXX_BOOL_CHK(jerryx_register_global ("analogWrite", js_analog_write));
  JERRYXX_BOOL_CHK(jerryx_register_global ("analogReadResolution", js_analog_read_resolution));
  JERRYXX_BOOL_CHK(jerryx_register_global ("analogWriteResolution", js_analog_write_resolution));
  /* Advanced I/O */
  /* External Interrupts */
  JERRYXX_BOOL_CHK(jerryx_register_global ("attachInterrupt", js_attach_interrupt));
  JERRYXX_BOOL_CHK(jerryx_register_global ("detachInterrupt", js_detach_interrupt));
  /* Interrupts */
  JERRYXX_BOOL_CHK(jerryx_register_global ("interrupts", js_interrupts));
  JERRYXX_BOOL_CHK(jerryx_register_global ("noInterrupts", js_no_interrupts));
  /* Characters */

  /* Register Objects in the global object */

  /* Communication */

cleanup:
  return ret;
} /* jerryxx_register_arduino_api */

/**
 * Arduino: pinMode
 */
JERRYXX_DECLARE_FUNCTION(pin_mode)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 2, "Wrong arguments count in 'pinMode' function.");

  jerry_value_t pin = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (pin), "Wrong argument 'pin' must be a number.");
  
  jerry_value_t mode = args_p[1];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (mode), "Wrong argument 'mode' must be a number.");

  double pin_mode = jerry_value_as_number (mode);
  if(pin_mode != INPUT && pin_mode != OUTPUT && pin_mode != INPUT_PULLUP && pin_mode != INPUT_PULLDOWN)
  {
    return jerry_throw_sz (JERRY_ERROR_RANGE, "Wrong argument 'mode' must be INPUT, OUTPUT, INPUT_PULLUP or INPUT_PULLDOWN.");
  }

  pinMode (jerry_value_as_number (pin), pin_mode);

  return jerry_undefined ();
} /* js_pin_mode */

/**
 * Arduino: digitalWrite
 */
JERRYXX_DECLARE_FUNCTION(digital_write)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 2, "Wrong arguments count in 'digitalWrite' function.");

  jerry_value_t pin = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE((!jerry_value_is_number (pin)), "Wrong argument 'pin' must be a number.");

  jerry_value_t value = args_p[1];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE((!jerry_value_is_number (value) && !jerry_value_is_boolean (value)), "Wrong argument 'value' must be a number or boolean.");

  double pin_value = (jerry_value_is_boolean (value) ? (jerry_value_is_true (value) ? HIGH : LOW) : jerry_value_as_number (value));
  if(pin_value != HIGH && pin_value != LOW)
  {
    return jerry_throw_sz (JERRY_ERROR_RANGE, "Wrong argument 'value' must be HIGH or LOW.");
  }

  digitalWrite (jerry_value_as_number (pin), pin_value);

  return jerry_undefined ();
} /* js_digital_write */

/**
 * Arduino: digitalRead
 */
JERRYXX_DECLARE_FUNCTION(digital_read)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 1, "Wrong arguments count in 'digitalRead' function.");

  jerry_value_t pin = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (pin), "Wrong argument 'pin' must be a number.");

  return jerry_number (digitalRead (jerry_value_as_number (pin)));
} /* js_digital_read */

/**
 * Arduino: delay
 */
JERRYXX_DECLARE_FUNCTION(delay)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 1, "Wrong arguments count in 'delay' function.");

  jerry_value_t value = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (value), "Wrong argument 'value' must be a number.");

  delay (jerry_value_as_number (value));

  return jerry_undefined ();
} /* js_delay */

/**
 * Arduino: delayMicroseconds
 */
JERRYXX_DECLARE_FUNCTION(delay_microseconds)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 1, "Wrong arguments count in 'delayMicroseconds' function.");

  jerry_value_t value = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (value), "Wrong argument 'value' must be a number.");

  delayMicroseconds (jerry_value_as_number (value));

  return jerry_undefined ();
} /* js_delay_microseconds */

/**
 * Arduino: micros
 */
JERRYXX_DECLARE_FUNCTION(micros)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 0, "Wrong arguments count in 'micros' function.");

  return jerry_number (micros ());
} /* js_micros */

/**
 * Arduino: millis
 */
JERRYXX_DECLARE_FUNCTION(millis)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 0, "Wrong arguments count in 'millis' function.");

  return jerry_number (millis ());
} /* js_millis */

/**
 * Arduino: randomSeed
 */
JERRYXX_DECLARE_FUNCTION(random_seed)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 1, "Wrong arguments count in 'randomSeed' function.");

  jerry_value_t seed = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (seed), "Wrong argument 'seed' must be a number.");

  randomSeed (jerry_value_as_number (seed));

  return jerry_undefined();
} /* js_random_seed */

/**
 * Arduino: random
 */
JERRYXX_DECLARE_FUNCTION(random)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX((args_cnt != 1 && args_cnt != 2), "Wrong arguments count in 'random' function.");

  if (args_cnt == 1)
  {
      jerry_value_t r_max = args_p[0];
      JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (r_max), "Wrong argument 'max' must be a number.");
      return jerry_number (random (jerry_value_as_number (r_max)));
  }
  else if (args_cnt == 2)
  {
      jerry_value_t r_min = args_p[0];
      jerry_value_t r_max = args_p[1];
      JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (r_min), "Wrong argument 'min' must be a number.");
      JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (r_max), "Wrong argument 'max' must be a number.");
      return jerry_number (random (jerry_value_as_number (r_min), jerry_value_as_number (r_max)));
  }

  return jerry_number (0);
} /* js_random */

/**
 * Arduino: analogRead
 */
JERRYXX_DECLARE_FUNCTION(analog_read)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 1, "Wrong arguments count in 'analogRead' function.");

  jerry_value_t pin = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (pin), "Wrong argument 'pin' must be a number.");

  return jerry_number (analogRead (jerry_value_as_number (pin)));
} /* js_analog_read */

/**
 * Arduino: analogWrite
 */
JERRYXX_DECLARE_FUNCTION(analog_write)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 2, "Wrong arguments count in 'analogWrite' function.");

  jerry_value_t pin = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (pin), "Wrong argument 'pin' must be a number.");

  jerry_value_t value = args_p[1];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (value), "Wrong argument 'value' must be a number.");

  analogWrite (jerry_value_as_number (pin), jerry_value_as_number (value));

  return jerry_undefined ();
} /* js_analog_write */

/**
 * Arduino: analogReadResolution
 */
JERRYXX_DECLARE_FUNCTION(analog_read_resolution)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 1, "Wrong arguments count in 'analogReadResolution' function.");

  jerry_value_t bits = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (bits), "Wrong argument 'bits' must be a number.");

  analogReadResolution (jerry_value_as_number (bits));

  return jerry_undefined ();
} /* js_analog_read_resolution */

/**
 * Arduino: analogWriteResolution
 */
JERRYXX_DECLARE_FUNCTION(analog_write_resolution)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 1, "Wrong arguments count in 'analogWriteResolution' function.");

  jerry_value_t bits = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (bits), "Wrong argument 'bits' must be a number.");

  analogWriteResolution (jerry_value_as_number (bits));

  return jerry_undefined ();
} /* js_analog_write_resolution */

/**
 * Arduino: interrupts
 */
JERRYXX_DECLARE_FUNCTION(interrupts)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 0, "Wrong arguments count in 'interrupts' function.");

  interrupts();

  return jerry_undefined ();
} /* js_interrupts */

/**
 * Arduino: noInterrupts
 */
JERRYXX_DECLARE_FUNCTION(no_interrupts)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 0, "Wrong arguments count in 'noInterrupts' function.");

  noInterrupts();

  return jerry_undefined ();
} /* js_no_interrupts */

/**
 * Arduino: attachInterrupt
 */
JERRYXX_DECLARE_FUNCTION(attach_interrupt)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 3, "Wrong arguments count in 'attachInterrupt' function.");

  jerry_value_t pin = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (pin), "Wrong argument 'pin' must be a number.");

  jerry_value_t callback_fn = args_p[1];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_function (callback_fn), "Wrong argument 'ISR' must be a function.");

  jerry_value_t mode = args_p[2];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (mode), "Wrong argument 'mode' must be a number.");

  auto func = [](void* callback_fn) -> void {
    jerry_value_t global_obj_val = jerry_current_realm ();
    jerry_value_free (jerry_call ((jerry_value_t)callback_fn, global_obj_val, NULL, 0));
    jerry_value_free (global_obj_val);
  };

  attachInterruptParam ((pin_size_t)jerry_value_as_number (pin), (voidFuncPtrParam)func, (PinStatus)jerry_value_as_number (mode), (void*) callback_fn);

  return jerry_undefined ();
} /* js_attach_interrupt */

/**
 * Arduino: detachInterrupt
 */
JERRYXX_DECLARE_FUNCTION(detach_interrupt)
{
  JERRYX_UNUSED (call_info_p);
  JERRYXX_ON_ARGS_COUNT_THROW_ERROR_SYNTAX(args_cnt != 1, "Wrong arguments count in 'detachInterrupt' function.");

  jerry_value_t pin = args_p[0];
  JERRYXX_ON_TYPE_CHECK_THROW_ERROR_TYPE(!jerry_value_is_number (pin), "Wrong argument 'pin' must be a number.");

  detachInterrupt (jerry_value_as_number (pin));

  return jerry_undefined ();
} /* js_detach_interrupt */
