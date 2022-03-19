/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "jerryscript-ext.h"

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/**
 * Print buffer size
 */
#define JERRYX_PRINT_BUFFER_SIZE 64

/**
 * Max line size that will be printed on a Syntax Error
 */
#define JERRYX_SYNTAX_ERROR_MAX_LINE_LENGTH 256

/**
 * Struct for buffering print outpu
 */
typedef struct
{
  jerry_size_t index; /**< write index */
  jerry_char_t data[JERRYX_PRINT_BUFFER_SIZE]; /**< buffer data */
} jerryx_print_buffer_t;

/**
 * Callback used by jerryx_print_value to batch written characters and print them in bulk.
 * NULL bytes are escaped and written as '\u0000'.
 *
 * @param value:  encoded byte value
 * @param user_p: user pointer
 */
static void
jerryx_buffered_print (uint32_t value, void *user_p)
{
  jerryx_print_buffer_t *buffer_p = (jerryx_print_buffer_t *) user_p;

  if (value == '\0')
  {
    jerryx_print_buffer (buffer_p->data, buffer_p->index);
    buffer_p->index = 0;

    jerryx_print_string ("\\u0000");
    return;
  }

  assert (value <= UINT8_MAX);
  buffer_p->data[buffer_p->index++] = (uint8_t) value;

  if (buffer_p->index >= JERRYX_PRINT_BUFFER_SIZE)
  {
    jerryx_print_buffer (buffer_p->data, buffer_p->index);
    buffer_p->index = 0;
  }
} /* jerryx_buffered_print */

/**
 * Convert a value to string and print it to standard output.
 * NULL characters are escaped to "\u0000", other characters are output as-is.
 *
 * @param value: input value
 */
jerry_value_t
jerryx_print_value (const jerry_value_t value)
{
  jerry_value_t string;

  if (jerry_value_is_symbol (value))
  {
    string = jerry_symbol_descriptive_string (value);
  }
  else
  {
    string = jerry_value_to_string (value);

    if (jerry_value_is_exception (string))
    {
      return string;
    }
  }

  jerryx_print_buffer_t buffer;
  buffer.index = 0;

  jerry_string_iterate (string, JERRY_ENCODING_UTF8, &jerryx_buffered_print, &buffer);
  jerry_value_free (string);

  jerryx_print_buffer (buffer.data, buffer.index);

  return jerry_undefined ();
} /* jerryx_print */

/**
 * Print a character to standard output, also sending it to the debugger, if connected.
 *
 * @param ch: input character
 */
void
jerryx_print_byte (jerry_char_t byte)
{
  jerry_port_print_byte (byte);
#if JERRY_DEBUGGER
  jerry_debugger_send_output (&byte, 1);
#endif /* JERRY_DEBUGGER */
} /* jerryx_print_char */

/**
 * Print a buffer to standard output, also sending it to the debugger, if connected.
 *
 * @param buffer_p: inptut string buffer
 * @param buffer_size: size of the string
 */
void
jerryx_print_buffer (const jerry_char_t *buffer_p, jerry_size_t buffer_size)
{
  jerry_port_print_buffer (buffer_p, buffer_size);
#if JERRY_DEBUGGER
  jerry_debugger_send_output (buffer_p, buffer_size);
#endif /* JERRY_DEBUGGER */
} /* jerryx_print_buffer */

/**
 * Print a zero-terminated string to standard output, also sending it to the debugger, if connected.
 *
 * @param buffer_p: inptut string buffer
 * @param buffer_size: size of the string
 */
void
jerryx_print_string (const char *str_p)
{
  const jerry_char_t *buffer_p = (jerry_char_t *) str_p;
  jerry_size_t buffer_size = (jerry_size_t) (strlen (str_p));

  jerry_port_print_buffer (buffer_p, buffer_size);
#if JERRY_DEBUGGER
  jerry_debugger_send_output (buffer_p, buffer_size);
#endif /* JERRY_DEBUGGER */
} /* jerryx_print_string */

/**
 * Print backtrace as log messages up to a specific depth.
 *
 * @param depth: backtrace depth
 */
void
jerryx_print_backtrace (unsigned depth)
{
  if (!jerry_feature_enabled (JERRY_FEATURE_LINE_INFO))
  {
    return;
  }

  jerry_log (JERRY_LOG_LEVEL_ERROR, "Script backtrace (top %u):\n", depth);

  jerry_value_t backtrace_array = jerry_backtrace (depth);
  unsigned array_length = jerry_array_length (backtrace_array);

  for (unsigned idx = 0; idx < array_length; idx++)
  {
    jerry_value_t property = jerry_object_get_index (backtrace_array, idx);

    jerry_char_t buffer[JERRYX_PRINT_BUFFER_SIZE];

    jerry_size_t copied = jerry_string_to_buffer (property, JERRY_ENCODING_UTF8, buffer, JERRYX_PRINT_BUFFER_SIZE - 1);
    buffer[copied] = '\0';

    jerry_log (JERRY_LOG_LEVEL_ERROR, " %u: %s\n", idx, buffer);
    jerry_value_free (property);
  }

  jerry_value_free (backtrace_array);
} /* jerryx_handler_assert_fatal */

/**
 * Print an unhandled exception value
 *
 * The function will take ownership of the value, and free it.
 *
 * @param exception: unhandled exception value
 */
void
jerryx_print_unhandled_exception (jerry_value_t exception) /**< exception value */
{
  assert (jerry_value_is_exception (exception));
  jerry_value_t value = jerry_exception_value (exception, true);

  JERRY_VLA (jerry_char_t, buffer_p, JERRYX_PRINT_BUFFER_SIZE);

  jerry_value_t string = jerry_value_to_string (value);

  jerry_size_t copied = jerry_string_to_buffer (string, JERRY_ENCODING_UTF8, buffer_p, JERRYX_PRINT_BUFFER_SIZE - 1);
  buffer_p[copied] = '\0';

  if (jerry_feature_enabled (JERRY_FEATURE_ERROR_MESSAGES) && jerry_error_type (value) == JERRY_ERROR_SYNTAX)
  {
    jerry_char_t *string_end_p = buffer_p + copied;
    jerry_size_t err_line = 0;
    jerry_size_t err_col = 0;
    char *path_str_p = NULL;
    char *path_str_end_p = NULL;

    /* 1. parse column and line information */
    for (jerry_char_t *current_p = buffer_p; current_p < string_end_p; current_p++)
    {
      if (*current_p == '[')
      {
        current_p++;

        if (*current_p == '<')
        {
          break;
        }

        path_str_p = (char *) current_p;
        while (current_p < string_end_p && *current_p != ':')
        {
          current_p++;
        }

        path_str_end_p = (char *) current_p++;

        err_line = (unsigned int) strtol ((char *) current_p, (char **) &current_p, 10);

        current_p++;

        err_col = (unsigned int) strtol ((char *) current_p, NULL, 10);
        break;
      }
    } /* for */

    if (err_line > 0 && err_col > 0 && err_col < JERRYX_SYNTAX_ERROR_MAX_LINE_LENGTH)
    {
      /* Temporarily modify the error message, so we can use the path. */
      *path_str_end_p = '\0';

      jerry_size_t source_size;
      jerry_char_t *source_p = jerry_port_source_read (path_str_p, &source_size);

      /* Revert the error message. */
      *path_str_end_p = ':';

      if (source_p != NULL)
      {
        uint32_t curr_line = 1;
        jerry_size_t pos = 0;

        /* 2. seek and print */
        while (pos < source_size && curr_line < err_line)
        {
          if (source_p[pos] == '\n')
          {
            curr_line++;
          }

          pos++;
        }

        /* Print character if:
         * - The max line length is not reached.
         * - The current position is valid (it is not the end of the source).
         * - The current character is not a newline.
         **/
        for (uint32_t char_count = 0;
             (char_count < JERRYX_SYNTAX_ERROR_MAX_LINE_LENGTH) && (pos < source_size) && (source_p[pos] != '\n');
             char_count++, pos++)
        {
          jerry_log (JERRY_LOG_LEVEL_ERROR, "%c", source_p[pos]);
        }

        jerry_log (JERRY_LOG_LEVEL_ERROR, "\n");
        jerry_port_source_free (source_p);

        while (--err_col)
        {
          jerry_log (JERRY_LOG_LEVEL_ERROR, "~");
        }

        jerry_log (JERRY_LOG_LEVEL_ERROR, "^\n\n");
      }
    }
  }

  jerry_log (JERRY_LOG_LEVEL_ERROR, "Unhandled exception: %s\n", buffer_p);
  jerry_value_free (string);

  if (jerry_value_is_object (value))
  {
    jerry_value_t backtrace_val = jerry_object_get_sz (value, "stack");

    if (jerry_value_is_array (backtrace_val))
    {
      uint32_t length = jerry_array_length (backtrace_val);

      /* This length should be enough. */
      if (length > 32)
      {
        length = 32;
      }

      for (unsigned i = 0; i < length; i++)
      {
        jerry_value_t item_val = jerry_object_get_index (backtrace_val, i);

        if (jerry_value_is_string (item_val))
        {
          copied = jerry_string_to_buffer (item_val, JERRY_ENCODING_UTF8, buffer_p, JERRYX_PRINT_BUFFER_SIZE - 1);
          buffer_p[copied] = '\0';

          jerry_log (JERRY_LOG_LEVEL_ERROR, " %u: %s\n", i, buffer_p);
        }

        jerry_value_free (item_val);
      }
    }

    jerry_value_free (backtrace_val);
  }

  jerry_value_free (value);
} /* jerryx_print_unhandled_exception */

/**
 * Print unhandled promise rejection.
 *
 * @param result: promise rejection result
 */
void
jerryx_print_unhandled_rejection (jerry_value_t result) /**< result value */
{
  jerry_value_t reason = jerry_value_to_string (result);

  if (!jerry_value_is_exception (reason))
  {
    JERRY_VLA (jerry_char_t, buffer_p, JERRYX_PRINT_BUFFER_SIZE);
    jerry_size_t copied = jerry_string_to_buffer (reason, JERRY_ENCODING_UTF8, buffer_p, JERRYX_PRINT_BUFFER_SIZE - 1);
    buffer_p[copied] = '\0';

    jerry_log (JERRY_LOG_LEVEL_WARNING, "Uncaught Promise rejection: %s\n", buffer_p);
  }
  else
  {
    jerry_log (JERRY_LOG_LEVEL_WARNING, "Uncaught Promise rejection: (reason cannot be converted to string)\n");
  }

  jerry_value_free (reason);
} /* jerryx_print_unhandled_rejection */


/**
 * Provide a 'print' implementation for scripts.
 *
 * The routine converts all of its arguments to strings and outputs them
 * char-by-char using jerry_port_print_byte.
 *
 * The NUL character is output as "\u0000", other characters are output
 * bytewise.
 *
 * Note:
 *      This implementation does not use standard C `printf` to print its
 *      output. This allows more flexibility but also extends the core
 *      JerryScript engine port API. Applications that want to use
 *      `jerryx_handler_print` must ensure that their port implementation also
 *      provides `jerry_port_print_byte`.
 *
 * @return undefined - if all arguments could be converted to strings,
 *         error - otherwise.
 */
jerry_value_t
jerryx_handler_print (const jerry_call_info_t *call_info_p, /**< call information */
                      const jerry_value_t args_p[], /**< function arguments */
                      const jerry_length_t args_cnt) /**< number of function arguments */
{
  (void) call_info_p; /* unused */

  for (jerry_length_t index = 0; index < args_cnt; index++)
  {
    if (index > 0)
    {
      jerryx_print_byte (' ');
    }

    jerry_value_t result = jerryx_print_value (args_p[index]);

    if (jerry_value_is_exception (result))
    {
      return result;
    }
  }

  jerryx_print_byte ('\n');
  return jerry_undefined ();
} /* jerryx_handler_print */

/**
 * Hard assert for scripts. The routine calls jerry_port_fatal on assertion failure.
 *
 * Notes:
 *  * If the `JERRY_FEATURE_LINE_INFO` runtime feature is enabled (build option: `JERRY_LINE_INFO`)
 *    a backtrace is also printed out.
 *
 * @return true - if only one argument was passed and that argument was a boolean true.
 *         Note that the function does not return otherwise.
 */
jerry_value_t
jerryx_handler_assert (const jerry_call_info_t *call_info_p, /**< call information */
                       const jerry_value_t args_p[], /**< function arguments */
                       const jerry_length_t args_cnt) /**< number of function arguments */
{
  (void) call_info_p; /* unused */

  if (args_cnt == 1 && jerry_value_is_true (args_p[0]))
  {
    return jerry_boolean (true);
  }

  /* Assert failed, print a bit of JS backtrace */
  jerry_log (JERRY_LOG_LEVEL_ERROR, "Script Error: assertion failed\n");
  jerryx_print_backtrace (5);

  jerry_port_fatal (JERRY_FATAL_FAILED_ASSERTION);
} /* jerryx_handler_assert_fatal */

/**
 * Expose garbage collector to scripts.
 *
 * @return undefined.
 */
jerry_value_t
jerryx_handler_gc (const jerry_call_info_t *call_info_p, /**< call information */
                   const jerry_value_t args_p[], /**< function arguments */
                   const jerry_length_t args_cnt) /**< number of function arguments */
{
  (void) call_info_p; /* unused */

  jerry_gc_mode_t mode =
    ((args_cnt > 0 && jerry_value_to_boolean (args_p[0])) ? JERRY_GC_PRESSURE_HIGH : JERRY_GC_PRESSURE_LOW);

  jerry_heap_gc (mode);
  return jerry_undefined ();
} /* jerryx_handler_gc */

/**
 * Get the resource name (usually a file name) of the currently executed script or the given function object
 *
 * Note: returned value must be freed with jerry_value_free, when it is no longer needed
 *
 * @return JS string constructed from
 *         - the currently executed function object's resource name, if the given value is undefined
 *         - resource name of the function object, if the given value is a function object
 *         - "<anonymous>", otherwise
 */
jerry_value_t
jerryx_handler_source_name (const jerry_call_info_t *call_info_p, /**< call information */
                            const jerry_value_t args_p[], /**< function arguments */
                            const jerry_length_t args_cnt) /**< number of function arguments */
{
  (void) call_info_p; /* unused */

  jerry_value_t undefined_value = jerry_undefined ();
  jerry_value_t source_name = jerry_source_name (args_cnt > 0 ? args_p[0] : undefined_value);
  jerry_value_free (undefined_value);

  return source_name;
} /* jerryx_handler_source_name */

/**
 * Create a new realm.
 *
 * @return new Realm object
 */
jerry_value_t
jerryx_handler_create_realm (const jerry_call_info_t *call_info_p, /**< call information */
                             const jerry_value_t args_p[], /**< function arguments */
                             const jerry_length_t args_cnt) /**< number of function arguments */
{
  (void) call_info_p; /* unused */
  (void) args_p; /* unused */
  (void) args_cnt; /* unused */
  return jerry_realm ();
} /* jerryx_handler_create_realm */

/**
 * Handler for unhandled promise rejection events.
 */
void
jerryx_handler_promise_reject (jerry_promise_event_type_t event_type, /**< event type */
                               const jerry_value_t object, /**< target object */
                               const jerry_value_t value, /**< optional argument */
                               void *user_p) /**< user pointer passed to the callback */
{
  (void) value;
  (void) user_p;

  if (event_type != JERRY_PROMISE_EVENT_REJECT_WITHOUT_HANDLER)
  {
    return;
  }

  jerry_value_t result = jerry_promise_result (object);
  jerryx_print_unhandled_rejection (result);

  jerry_value_free (result);
} /* jerryx_handler_promise_reject */

/**
 * Runs the source code received by jerry_debugger_wait_for_client_source.
 *
 * @return result fo the source code execution
 */
jerry_value_t
jerryx_handler_source_received (const jerry_char_t *source_name_p, /**< resource name */
                                size_t source_name_size, /**< size of resource name */
                                const jerry_char_t *source_p, /**< source code */
                                size_t source_size, /**< source code size */
                                void *user_p) /**< user pointer */
{
  (void) user_p; /* unused */

  jerry_parse_options_t parse_options;
  parse_options.options = JERRY_PARSE_HAS_SOURCE_NAME;
  parse_options.source_name = jerry_string (source_name_p, (jerry_size_t) source_name_size, JERRY_ENCODING_UTF8);

  jerry_value_t ret_val = jerry_parse (source_p, source_size, &parse_options);

  jerry_value_free (parse_options.source_name);

  if (!jerry_value_is_exception (ret_val))
  {
    jerry_value_t func_val = ret_val;
    ret_val = jerry_run (func_val);
    jerry_value_free (func_val);
  }

  return ret_val;
} /* jerryx_handler_source_received */

/**
 * Register a JavaScript function in the global object.
 *
 * @return true - if the operation was successful,
 *         false - otherwise.
 */
bool
jerryx_register_global (const char *name_p, /**< name of the function */
                        jerry_external_handler_t handler_p) /**< function callback */
{
  jerry_value_t global_obj_val = jerry_current_realm ();
  jerry_value_t function_name_val = jerry_string_sz (name_p);
  jerry_value_t function_val = jerry_function_external (handler_p);

  jerry_value_t result_val = jerry_object_set (global_obj_val, function_name_val, function_val);
  bool result = jerry_value_is_true (result_val);

  jerry_value_free (result_val);
  jerry_value_free (function_val);
  jerry_value_free (function_name_val);
  jerry_value_free (global_obj_val);

  return result;
} /* jerryx_register_global */

/**
 * Set multiple properties on a target object.
 *
 * The properties are an array of (name, property value) pairs and
 * this list must end with a (NULL, 0) entry.
 *
 * Notes:
 *  - Each property value in the input array is released after a successful property registration.
 *  - The property name must be a zero terminated UTF-8 string.
 *  - There should be no '\0' (NULL) character in the name excluding the string terminator.
 *  - The method `jerryx_release_property_entry` must be called if there is any failed registration
 *    to release the values in the entries array.
 *
 * @return `jerryx_register_result` struct - if everything is ok with the (undefined, property entry count) values.
 *         In case of error the (error object, registered property count) pair.
 */
jerryx_register_result
jerryx_set_properties (const jerry_value_t target_object, /**< target object */
                       const jerryx_property_entry entries[]) /**< array of method entries */
{
#define JERRYX_SET_PROPERTIES_RESULT(VALUE, IDX) ((jerryx_register_result){ VALUE, IDX })
  uint32_t idx = 0;

  if (entries == NULL)
  {
    return JERRYX_SET_PROPERTIES_RESULT (jerry_undefined (), 0);
  }

  for (; (entries[idx].name != NULL); idx++)
  {
    const jerryx_property_entry *entry = &entries[idx];

    jerry_value_t prop_name = jerry_string_sz (entry->name);
    jerry_value_t result = jerry_object_set (target_object, prop_name, entry->value);

    jerry_value_free (prop_name);

    // By API definition:
    // The jerry_object_set returns TRUE if there is no problem
    // and error object if there is any problem.
    // Thus there is no need to check if the boolean value is false or not.
    if (!jerry_value_is_boolean (result))
    {
      return JERRYX_SET_PROPERTIES_RESULT (result, idx);
    }

    jerry_value_free (entry->value);
    jerry_value_free (result);
  }

  return JERRYX_SET_PROPERTIES_RESULT (jerry_undefined (), idx);
#undef JERRYX_SET_PROPERTIES_RESULT
} /* jerryx_set_properties */

/**
 * Release all jerry_value_t in a jerryx_property_entry array based on
 * a previous jerryx_set_properties call.
 *
 * In case of a successful registration it is safe to call this method.
 */
void
jerryx_release_property_entry (const jerryx_property_entry entries[], /**< list of property entries */
                               const jerryx_register_result register_result) /**< previous result of registration */
{
  if (entries == NULL)
  {
    return;
  }

  for (uint32_t idx = register_result.registered; entries[idx].name != NULL; idx++)
  {
    jerry_value_free (entries[idx].value);
  }
} /* jerryx_release_property_entry */

static const char *module_name_property_name = "moduleName";
static const char *module_not_found = "Module not found";
static const char *module_name_not_string = "Module name is not a string";

/**
 * Create an error related to modules
 *
 * Creates an error object of the requested type with the additional property "moduleName" the value of which is a
 * string containing the name of the module that was requested when the error occurred.
 *
 * @return the error
 */
static jerry_value_t
jerryx_module_create_error (jerry_error_t error_type, /**< the type of error to create */
                            const char *message, /**< the error message */
                            const jerry_value_t module_name) /**< the module name */
{
  jerry_value_t error_object = jerry_error_sz (error_type, message);
  jerry_value_t property_name = jerry_string_sz (module_name_property_name);

  jerry_value_free (jerry_object_set (error_object, property_name, module_name));

  jerry_value_free (property_name);
  return jerry_throw_value (error_object, true);
} /* jerryx_module_create_error */

/**
 * Initialize the module manager extension.
 */
static void
jerryx_module_manager_init (void *user_data_p)
{
  *((jerry_value_t *) user_data_p) = jerry_object ();
} /* jerryx_module_manager_init */

/**
 * Deinitialize the module manager extension.
 */
static void
jerryx_module_manager_deinit (void *user_data_p) /**< context pointer to deinitialize */
{
  jerry_value_free (*(jerry_value_t *) user_data_p);
} /* jerryx_module_manager_deinit */

/**
 * Declare the context data manager for modules.
 */
static const jerry_context_data_manager_t jerryx_module_manager = { .init_cb = jerryx_module_manager_init,
                                                                    .deinit_cb = jerryx_module_manager_deinit,
                                                                    .bytes_needed = sizeof (jerry_value_t) };

/**
 * Global static entry point to the linked list of available modules.
 */
static jerryx_native_module_t *first_module_p = NULL;

void
jerryx_native_module_register (jerryx_native_module_t *module_p)
{
  module_p->next_p = first_module_p;
  first_module_p = module_p;
} /* jerryx_native_module_register */

void
jerryx_native_module_unregister (jerryx_native_module_t *module_p)
{
  jerryx_native_module_t *parent_p = NULL, *iter_p = NULL;

  for (iter_p = first_module_p; iter_p != NULL; parent_p = iter_p, iter_p = iter_p->next_p)
  {
    if (iter_p == module_p)
    {
      if (parent_p)
      {
        parent_p->next_p = module_p->next_p;
      }
      else
      {
        first_module_p = module_p->next_p;
      }
      module_p->next_p = NULL;
    }
  }
} /* jerryx_native_module_unregister */

/**
 * Attempt to retrieve a module by name from a cache, and return false if not found.
 */
static bool
jerryx_module_check_cache (jerry_value_t cache, /**< cache from which to attempt to retrieve the module by name */
                           jerry_value_t module_name, /**< JerryScript string value holding the module name */
                           jerry_value_t *result) /**< Resulting value */
{
  bool ret = false;

  /* Check if the cache has the module. */
  jerry_value_t js_has_property = jerry_object_has (cache, module_name);

  /* If we succeed in getting an answer, we examine the answer. */
  if (!jerry_value_is_exception (js_has_property))
  {
    bool has_property = jerry_value_is_true (js_has_property);

    /* If the module is indeed in the cache, we return it. */
    if (has_property)
    {
      if (result != NULL)
      {
        (*result) = jerry_object_get (cache, module_name);
      }
      ret = true;
    }
  }

  jerry_value_free (js_has_property);

  return ret;
} /* jerryx_module_check_cache */

/**
 * Attempt to cache a loaded module.
 *
 * @return the module on success, otherwise the error encountered when attempting to cache. In the latter case, the
 * @p module is released.
 */
static jerry_value_t
jerryx_module_add_to_cache (jerry_value_t cache, /**< cache to which to add the module */
                            jerry_value_t module_name, /**< key at which to cache the module */
                            jerry_value_t module) /**< the module to cache */
{
  jerry_value_t ret = jerry_object_set (cache, module_name, module);

  if (jerry_value_is_exception (ret))
  {
    jerry_value_free (module);
  }
  else
  {
    jerry_value_free (ret);
    ret = module;
  }

  return ret;
} /* jerryx_module_add_to_cache */

static const char *on_resolve_absent = "Module on_resolve () must not be NULL";

/**
 * Declare and define the default module resolver - one which examines what modules are defined in the above linker
 * section and loads one that matches the requested name, caching the result for subsequent requests using the context
 * data mechanism.
 */
static bool
jerryx_resolve_native_module (const jerry_value_t canonical_name, /**< canonical name of the module */
                              jerry_value_t *result) /**< [out] where to put the resulting module instance */
{
  const jerryx_native_module_t *module_p = NULL;

  jerry_size_t name_size = jerry_string_size (canonical_name, JERRY_ENCODING_UTF8);
  JERRY_VLA (jerry_char_t, name_string, name_size);
  jerry_string_to_buffer (canonical_name, JERRY_ENCODING_UTF8, name_string, name_size);

  /* Look for the module by its name in the list of module definitions. */
  for (module_p = first_module_p; module_p != NULL; module_p = module_p->next_p)
  {
    if (module_p->name_p != NULL && strlen ((char *) module_p->name_p) == name_size
        && !strncmp ((char *) module_p->name_p, (char *) name_string, name_size))
    {
      /* If we find the module by its name we load it and cache it if it has an on_resolve () and complain otherwise. */
      (*result) =
        ((module_p->on_resolve_p) ? module_p->on_resolve_p ()
                                  : jerryx_module_create_error (JERRY_ERROR_TYPE, on_resolve_absent, canonical_name));
      return true;
    }
  }

  return false;
} /* jerryx_resolve_native_module */

jerryx_module_resolver_t jerryx_module_native_resolver = { .get_canonical_name_p = NULL,
                                                           .resolve_p = jerryx_resolve_native_module };

static void
jerryx_module_resolve_local (const jerry_value_t name, /**< name of the module to load */
                             const jerryx_module_resolver_t **resolvers_p, /**< list of resolvers */
                             size_t resolver_count, /**< number of resolvers in @p resolvers */
                             jerry_value_t *result) /**< location to store the result, or NULL to remove the module */
{
  size_t index;
  size_t canonical_names_used = 0;
  jerry_value_t instances;
  JERRY_VLA (jerry_value_t, canonical_names, resolver_count);
  jerry_value_t (*get_canonical_name_p) (const jerry_value_t name);
  bool (*resolve_p) (const jerry_value_t canonical_name, jerry_value_t *result);

  if (!jerry_value_is_string (name))
  {
    if (result != NULL)
    {
      *result = jerryx_module_create_error (JERRY_ERROR_COMMON, module_name_not_string, name);
    }
    goto done;
  }

  instances = *(jerry_value_t *) jerry_context_data (&jerryx_module_manager);

  /**
   * Establish the canonical name for the requested module. Each resolver presents its own canonical name. If one of
   * the canonical names matches a cached module, it is returned as the result.
   */
  for (index = 0; index < resolver_count; index++)
  {
    get_canonical_name_p = (resolvers_p[index] == NULL ? NULL : resolvers_p[index]->get_canonical_name_p);
    canonical_names[index] = ((get_canonical_name_p == NULL) ? jerry_value_copy (name) : get_canonical_name_p (name));
    canonical_names_used++;
    if (jerryx_module_check_cache (instances, canonical_names[index], result))
    {
      /* A NULL for result indicates that we are to delete the module from the cache if found. Let's do that here.*/
      if (result == NULL)
      {
        jerry_object_delete (instances, canonical_names[index]);
      }
      goto done;
    }
  }

  if (result == NULL)
  {
    goto done;
  }

  /**
   * Past this point we assume a module is wanted, and therefore result is not NULL. So, we try each resolver until one
   * manages to resolve the module.
   */
  for (index = 0; index < resolver_count; index++)
  {
    resolve_p = (resolvers_p[index] == NULL ? NULL : resolvers_p[index]->resolve_p);
    if (resolve_p != NULL && resolve_p (canonical_names[index], result))
    {
      if (!jerry_value_is_exception (*result))
      {
        *result = jerryx_module_add_to_cache (instances, canonical_names[index], *result);
      }
      goto done;
    }
  }

  /* If none of the resolvers manage to find the module, complain with "Module not found" */
  *result = jerryx_module_create_error (JERRY_ERROR_COMMON, module_not_found, name);

done:
  /* Release the canonical names as returned by the various resolvers. */
  for (index = 0; index < canonical_names_used; index++)
  {
    jerry_value_free (canonical_names[index]);
  }
} /* jerryx_module_resolve_local */

/**
 * Resolve a single module using the module resolvers available in the section declared above and load it into the
 * current context.
 *
 * @p name - name of the module to resolve
 * @p resolvers - list of resolvers to invoke
 * @p count - number of resolvers in the list
 *
 * @return a jerry_value_t containing one of the followings:
 *   - the result of having loaded the module named @p name, or
 *   - the result of a previous successful load, or
 *   - an error indicating that something went wrong during the attempt to load the module.
 */
jerry_value_t
jerryx_module_resolve (const jerry_value_t name, /**< name of the module to load */
                       const jerryx_module_resolver_t **resolvers_p, /**< list of resolvers */
                       size_t resolver_count) /**< number of resolvers in @p resolvers */
{
  /* Set to zero to circumvent fatal warning. */
  jerry_value_t ret = 0;
  jerryx_module_resolve_local (name, resolvers_p, resolver_count, &ret);
  return ret;
} /* jerryx_module_resolve */

void
jerryx_module_clear_cache (const jerry_value_t name, /**< name of the module to remove, or undefined */
                           const jerryx_module_resolver_t **resolvers_p, /**< list of resolvers */
                           size_t resolver_count) /**< number of resolvers in @p resolvers */
{
  void *instances_p = jerry_context_data (&jerryx_module_manager);

  if (jerry_value_is_undefined (name))
  {
    /* We were requested to clear the entire cache, so we bounce the context data in the most agnostic way possible. */
    jerryx_module_manager.deinit_cb (instances_p);
    jerryx_module_manager.init_cb (instances_p);
    return;
  }

  /* Delete the requested module from the cache if it's there. */
  jerryx_module_resolve_local (name, resolvers_p, resolver_count, NULL);
} /* jerryx_module_clear_cache */


void
jerryx_repl (const char *prompt_p)
{
  jerry_value_t result;

  while (true)
  {
    jerryx_print_string (prompt_p);

    jerry_size_t length;
    jerry_char_t *line_p = jerry_port_line_read (&length);

    if (line_p == NULL)
    {
      jerryx_print_byte ('\n');
      return;
    }

    if (length == 0)
    {
      continue;
    }

    if (!jerry_validate_string (line_p, length, JERRY_ENCODING_UTF8))
    {
      jerry_port_line_free (line_p);
      result = jerry_throw_sz (JERRY_ERROR_SYNTAX, "Input is not a valid UTF-8 string");
      goto exception;
    }

    result = jerry_parse (line_p, length, NULL);
    jerry_port_line_free (line_p);

    if (jerry_value_is_exception (result))
    {
      goto exception;
    }

    jerry_value_t script = result;
    result = jerry_run (script);
    jerry_value_free (script);

    if (jerry_value_is_exception (result))
    {
      goto exception;
    }

    jerry_value_t print_result = jerryx_print_value (result);
    jerry_value_free (result);
    result = print_result;

    if (jerry_value_is_exception (result))
    {
      goto exception;
    }

    jerryx_print_byte ('\n');

    jerry_value_free (result);
    result = jerry_run_jobs ();

    if (jerry_value_is_exception (result))
    {
      goto exception;
    }

    jerry_value_free (result);
    continue;

exception:
    jerryx_print_unhandled_exception (result);
  }
} /* jerryx_repl */

