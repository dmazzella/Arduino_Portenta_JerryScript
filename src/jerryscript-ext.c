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
#include "jerryscript.h"
#include "jerryscript-ext.h"




#ifndef JERRYX_ARG_INTERNAL_H
#define JERRYX_ARG_INTERNAL_H


/**
 * The iterator structor for JS arguments.
 */
struct jerryx_arg_js_iterator_t
{
  const jerry_value_t *js_arg_p; /**< the JS arguments */
  const jerry_length_t js_arg_cnt; /**< the total num of JS arguments */
  jerry_length_t js_arg_idx; /**< current index of JS argument */
};

#endif /* !JERRYX_ARG_INTERNAL_H */

#ifndef JEXT_COMMON_H
#define JEXT_COMMON_H

#include <stdio.h>
#include <string.h>


/*
 * Make sure unused parameters, variables, or expressions trigger no compiler warning.
 */
#define JERRYX_UNUSED(x) ((void) (x))

/*
 * Asserts
 *
 * Warning:
 *         Don't use JERRY_STATIC_ASSERT in headers, because
 *         __LINE__ may be the same for asserts in a header
 *         and in an implementation file.
 */
#define JERRYX_STATIC_ASSERT_GLUE_(a, b, c) a##b##_##c
#define JERRYX_STATIC_ASSERT_GLUE(a, b, c)  JERRYX_STATIC_ASSERT_GLUE_ (a, b, c)
#define JERRYX_STATIC_ASSERT(x, msg)                                                  \
  enum                                                                                \
  {                                                                                   \
    JERRYX_STATIC_ASSERT_GLUE (static_assertion_failed_, __LINE__, msg) = 1 / (!!(x)) \
  }

#ifndef JERRY_NDEBUG
void JERRY_ATTR_NORETURN jerry_assert_fail (const char *assertion,
                                            const char *file,
                                            const char *function,
                                            const uint32_t line);
void JERRY_ATTR_NORETURN jerry_unreachable (const char *file, const char *function, const uint32_t line);

#define JERRYX_ASSERT(x)                                    \
  do                                                        \
  {                                                         \
    if (JERRY_UNLIKELY (!(x)))                              \
    {                                                       \
      jerry_assert_fail (#x, __FILE__, __func__, __LINE__); \
    }                                                       \
  } while (0)

#define JERRYX_UNREACHABLE()                          \
  do                                                  \
  {                                                   \
    jerry_unreachable (__FILE__, __func__, __LINE__); \
  } while (0)
#else /* JERRY_NDEBUG */
#define JERRYX_ASSERT(x) \
  do                     \
  {                      \
    if (false)           \
    {                    \
      JERRYX_UNUSED (x); \
    }                    \
  } while (0)

#ifdef __GNUC__
#define JERRYX_UNREACHABLE() __builtin_unreachable ()
#endif /* __GNUC__ */

#ifdef _MSC_VER
#define JERRYX_UNREACHABLE() _assume (0)
#endif /* _MSC_VER */

#ifndef JERRYX_UNREACHABLE
#define JERRYX_UNREACHABLE()
#endif /* !JERRYX_UNREACHABLE */

#endif /* !JERRY_NDEBUG */

/*
 * Logging
 */
#define JERRYX_ERROR_MSG(...)   jerry_log (JERRY_LOG_LEVEL_ERROR, __VA_ARGS__)
#define JERRYX_WARNING_MSG(...) jerry_log (JERRY_LOG_LEVEL_WARNING, __VA_ARGS__)
#define JERRYX_DEBUG_MSG(...)   jerry_log (JERRY_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define JERRYX_TRACE_MSG(...)   jerry_log (JERRY_LOG_LEVEL_TRACE, __VA_ARGS__)

#endif /* !JEXT_COMMON_H */

JERRYX_STATIC_ASSERT (sizeof (jerryx_arg_int_option_t) <= sizeof (((jerryx_arg_t *) 0)->extra_info),
                      jerryx_arg_number_options_t_must_fit_into_extra_info);

/**
 * Validate the JS arguments and assign them to the native arguments.
 *
 * @return jerry undefined: all validators passed,
 *         jerry error: a validator failed.
 */
jerry_value_t
jerryx_arg_transform_args (const jerry_value_t *js_arg_p, /**< points to the array with JS arguments */
                           const jerry_length_t js_arg_cnt, /**< the count of the `js_arg_p` array */
                           const jerryx_arg_t *c_arg_p, /**< points to the array of validation/transformation steps */
                           jerry_length_t c_arg_cnt) /**< the count of the `c_arg_p` array */
{
  jerry_value_t ret = jerry_undefined ();

  jerryx_arg_js_iterator_t iterator = { .js_arg_p = js_arg_p, .js_arg_cnt = js_arg_cnt, .js_arg_idx = 0 };

  for (; c_arg_cnt != 0 && !jerry_value_is_exception (ret); c_arg_cnt--, c_arg_p++)
  {
    ret = c_arg_p->func (&iterator, c_arg_p);
  }

  return ret;
} /* jerryx_arg_transform_args */

/**
 * Validate the this value and the JS arguments,
 * and assign them to the native arguments.
 * This function is useful to perform input validation inside external
 * function handlers (see jerry_external_handler_t).
 * @note this_val is processed as the first value, before the array of arguments.
 *
 * @return jerry undefined: all validators passed,
 *         jerry error: a validator failed.
 */
jerry_value_t
jerryx_arg_transform_this_and_args (const jerry_value_t this_val, /**< the this_val for the external function */
                                    const jerry_value_t *js_arg_p, /**< points to the array with JS arguments */
                                    const jerry_length_t js_arg_cnt, /**< the count of the `js_arg_p` array */
                                    const jerryx_arg_t *c_arg_p, /**< points to the array of transformation steps */
                                    jerry_length_t c_arg_cnt) /**< the count of the `c_arg_p` array */
{
  if (c_arg_cnt == 0)
  {
    return jerry_undefined ();
  }

  jerryx_arg_js_iterator_t iterator = { .js_arg_p = &this_val, .js_arg_cnt = 1, .js_arg_idx = 0 };

  jerry_value_t ret = c_arg_p->func (&iterator, c_arg_p);

  if (jerry_value_is_exception (ret))
  {
    jerry_value_free (ret);

    return jerry_throw_sz (JERRY_ERROR_TYPE, "'this' validation failed.");
  }

  return jerryx_arg_transform_args (js_arg_p, js_arg_cnt, c_arg_p + 1, c_arg_cnt - 1);
} /* jerryx_arg_transform_this_and_args */

/**
 * Validate the `obj_val`'s properties,
 * and assign them to the native arguments.
 *
 * @return jerry undefined: all validators passed,
 *         jerry error: a validator failed.
 */
jerry_value_t
jerryx_arg_transform_object_properties (const jerry_value_t obj_val, /**< the JS object */
                                        const jerry_char_t **name_p, /**< property name list of the JS object */
                                        const jerry_length_t name_cnt, /**< count of the name list */
                                        const jerryx_arg_t *c_arg_p, /**< points to the array of transformation steps */
                                        jerry_length_t c_arg_cnt) /**< the count of the `c_arg_p` array */
{
  if (!jerry_value_is_object (obj_val))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "Not an object.");
  }

  JERRY_VLA (jerry_value_t, prop, name_cnt);

  for (jerry_length_t i = 0; i < name_cnt; i++, name_p++)
  {
    const jerry_value_t name_str = jerry_string_sz ((char *) (*name_p));
    prop[i] = jerry_object_get (obj_val, name_str);
    jerry_value_free (name_str);

    if (jerry_value_is_exception (prop[i]))
    {
      for (jerry_length_t j = 0; j < i; j++)
      {
        jerry_value_free (prop[j]);
      }

      return prop[i];
    }
  }

  const jerry_value_t ret = jerryx_arg_transform_args (prop, name_cnt, c_arg_p, c_arg_cnt);

  for (jerry_length_t i = 0; i < name_cnt; i++)
  {
    jerry_value_free (prop[i]);
  }

  return ret;
} /* jerryx_arg_transform_object_properties */

/**
 * Validate the items in the JS array and assign them to the native arguments.
 *
 * @return jerry undefined: all validators passed,
 *         jerry error: a validator failed.
 */
jerry_value_t
jerryx_arg_transform_array (const jerry_value_t array_val, /**< points to the JS array */
                            const jerryx_arg_t *c_arg_p, /**< points to the array of validation/transformation steps */
                            jerry_length_t c_arg_cnt) /**< the count of the `c_arg_p` array */
{
  if (!jerry_value_is_array (array_val))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "Not an array.");
  }

  JERRY_VLA (jerry_value_t, arr, c_arg_cnt);

  for (jerry_length_t i = 0; i < c_arg_cnt; i++)
  {
    arr[i] = jerry_object_get_index (array_val, i);

    if (jerry_value_is_exception (arr[i]))
    {
      for (jerry_length_t j = 0; j < i; j++)
      {
        jerry_value_free (arr[j]);
      }

      return arr[i];
    }
  }

  const jerry_value_t ret = jerryx_arg_transform_args (arr, c_arg_cnt, c_arg_p, c_arg_cnt);

  for (jerry_length_t i = 0; i < c_arg_cnt; i++)
  {
    jerry_value_free (arr[i]);
  }

  return ret;
} /* jerryx_arg_transform_array */



/**
 * Pop the current JS argument from the iterator.
 * It will change the index and js_arg_p value in the iterator.
 *
 * @return the current JS argument.
 */
jerry_value_t
jerryx_arg_js_iterator_pop (jerryx_arg_js_iterator_t *js_arg_iter_p) /**< the JS arg iterator */
{
  return (js_arg_iter_p->js_arg_idx++ < js_arg_iter_p->js_arg_cnt ? *js_arg_iter_p->js_arg_p++ : jerry_undefined ());
} /* jerryx_arg_js_iterator_pop */

/**
 * Restore the previous JS argument from the iterator.
 * It will change the index and js_arg_p value in the iterator.
 *
 * @return the restored (now current) JS argument.
 */
jerry_value_t
jerryx_arg_js_iterator_restore (jerryx_arg_js_iterator_t *js_arg_iter_p) /**< the JS arg iterator */
{
  if (js_arg_iter_p->js_arg_idx == 0)
  {
    return jerry_undefined ();
  }

  --js_arg_iter_p->js_arg_idx;
  --js_arg_iter_p->js_arg_p;

  return *js_arg_iter_p->js_arg_p;
} /* jerryx_arg_js_iterator_restore */

/**
 * Get the current JS argument from the iterator.
 *
 * Note:
 *     Unlike jerryx_arg_js_iterator_pop, it will not change index and
 *     js_arg_p value in the iterator.
 *
 * @return the current JS argument.
 */
jerry_value_t
jerryx_arg_js_iterator_peek (jerryx_arg_js_iterator_t *js_arg_iter_p) /**< the JS arg iterator */
{
  return (js_arg_iter_p->js_arg_idx < js_arg_iter_p->js_arg_cnt ? *js_arg_iter_p->js_arg_p : jerry_undefined ());
} /* jerryx_arg_js_iterator_peek */

/**
 * Get the index of the current JS argument
 *
 * @return the index
 */
jerry_length_t
jerryx_arg_js_iterator_index (jerryx_arg_js_iterator_t *js_arg_iter_p) /**< the JS arg iterator */
{
  return js_arg_iter_p->js_arg_idx;
} /* jerryx_arg_js_iterator_index */

#include <math.h>



/**
 * The common function to deal with optional arguments.
 * The core transform function is provided by argument `func`.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_optional (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                               const jerryx_arg_t *c_arg_p, /**< native arg */
                               jerryx_arg_transform_func_t func) /**< the core transform function */
{
  jerry_value_t js_arg = jerryx_arg_js_iterator_peek (js_arg_iter_p);

  if (jerry_value_is_undefined (js_arg))
  {
    return jerryx_arg_js_iterator_pop (js_arg_iter_p);
  }

  return func (js_arg_iter_p, c_arg_p);
} /* jerryx_arg_transform_optional */

/**
 * The common part in transforming a JS argument to a number (double or certain int) type.
 * Type coercion is not allowed.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
static jerry_value_t
jerryx_arg_transform_number_strict_common (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                           double *number_p) /**< [out] the number in JS arg */
{
  jerry_value_t js_arg = jerryx_arg_js_iterator_pop (js_arg_iter_p);

  if (!jerry_value_is_number (js_arg))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "It is not a number.");
  }

  *number_p = jerry_value_as_number (js_arg);

  return jerry_undefined ();
} /* jerryx_arg_transform_number_strict_common */

/**
 * The common part in transforming a JS argument to a number (double or certain int) type.
 * Type coercion is allowed.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
static jerry_value_t
jerryx_arg_transform_number_common (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                    double *number_p) /**< [out] the number in JS arg */
{
  jerry_value_t js_arg = jerryx_arg_js_iterator_pop (js_arg_iter_p);

  jerry_value_t to_number = jerry_value_to_number (js_arg);

  if (jerry_value_is_exception (to_number))
  {
    jerry_value_free (to_number);

    return jerry_throw_sz (JERRY_ERROR_TYPE, "It can not be converted to a number.");
  }

  *number_p = jerry_value_as_number (to_number);
  jerry_value_free (to_number);

  return jerry_undefined ();
} /* jerryx_arg_transform_number_common */

/**
 * Transform a JS argument to a double. Type coercion is not allowed.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_number_strict (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                    const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  return jerryx_arg_transform_number_strict_common (js_arg_iter_p, c_arg_p->dest);
} /* jerryx_arg_transform_number_strict */

/**
 * Transform a JS argument to a double. Type coercion is allowed.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_number (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                             const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  return jerryx_arg_transform_number_common (js_arg_iter_p, c_arg_p->dest);
} /* jerryx_arg_transform_number */

/**
 * Helper function to process a double number before converting it
 * to an integer.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
static jerry_value_t
jerryx_arg_helper_process_double (double *d, /**< [in, out] the number to be processed */
                                  double min, /**< the min value for clamping */
                                  double max, /**< the max value for clamping */
                                  jerryx_arg_int_option_t option) /**< the converting policies */
{
  if (*d != *d) /* isnan (*d) triggers conversion warning on clang<9 */
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "The number is NaN.");
  }

  if (option.clamp == JERRYX_ARG_NO_CLAMP)
  {
    if (*d > max || *d < min)
    {
      return jerry_throw_sz (JERRY_ERROR_TYPE, "The number is out of range.");
    }
  }
  else
  {
    *d = *d < min ? min : *d;
    *d = *d > max ? max : *d;
  }

  if (option.round == JERRYX_ARG_ROUND)
  {
    *d = (*d >= 0.0) ? floor (*d + 0.5) : ceil (*d - 0.5);
  }
  else if (option.round == JERRYX_ARG_FLOOR)
  {
    *d = floor (*d);
  }
  else
  {
    *d = ceil (*d);
  }

  return jerry_undefined ();
} /* jerryx_arg_helper_process_double */

/**
 * Use the macro to define thr transform functions for int type.
 */
#define JERRYX_ARG_TRANSFORM_FUNC_FOR_INT_TEMPLATE(type, suffix, min, max)                    \
  jerry_value_t jerryx_arg_transform_##type##suffix (jerryx_arg_js_iterator_t *js_arg_iter_p, \
                                                     const jerryx_arg_t *c_arg_p)             \
  {                                                                                           \
    double tmp = 0.0;                                                                         \
    jerry_value_t rv = jerryx_arg_transform_number##suffix##_common (js_arg_iter_p, &tmp);    \
    if (jerry_value_is_exception (rv))                                                        \
    {                                                                                         \
      return rv;                                                                              \
    }                                                                                         \
    jerry_value_free (rv);                                                                    \
    union                                                                                     \
    {                                                                                         \
      jerryx_arg_int_option_t int_option;                                                     \
      uintptr_t extra_info;                                                                   \
    } u = { .extra_info = c_arg_p->extra_info };                                              \
    rv = jerryx_arg_helper_process_double (&tmp, min, max, u.int_option);                     \
    if (jerry_value_is_exception (rv))                                                        \
    {                                                                                         \
      return rv;                                                                              \
    }                                                                                         \
    *(type##_t *) c_arg_p->dest = (type##_t) tmp;                                             \
    return rv;                                                                                \
  }

#define JERRYX_ARG_TRANSFORM_FUNC_FOR_INT(type, min, max)              \
  JERRYX_ARG_TRANSFORM_FUNC_FOR_INT_TEMPLATE (type, _strict, min, max) \
  JERRYX_ARG_TRANSFORM_FUNC_FOR_INT_TEMPLATE (type, , min, max)

JERRYX_ARG_TRANSFORM_FUNC_FOR_INT (uint8, 0, UINT8_MAX)
JERRYX_ARG_TRANSFORM_FUNC_FOR_INT (int8, INT8_MIN, INT8_MAX)
JERRYX_ARG_TRANSFORM_FUNC_FOR_INT (uint16, 0, UINT16_MAX)
JERRYX_ARG_TRANSFORM_FUNC_FOR_INT (int16, INT16_MIN, INT16_MAX)
JERRYX_ARG_TRANSFORM_FUNC_FOR_INT (uint32, 0, UINT32_MAX)
JERRYX_ARG_TRANSFORM_FUNC_FOR_INT (int32, INT32_MIN, INT32_MAX)

#undef JERRYX_ARG_TRANSFORM_FUNC_FOR_INT_TEMPLATE
#undef JERRYX_ARG_TRANSFORM_FUNC_FOR_INT
/**
 * Transform a JS argument to a boolean. Type coercion is not allowed.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_boolean_strict (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                     const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  jerry_value_t js_arg = jerryx_arg_js_iterator_pop (js_arg_iter_p);

  if (!jerry_value_is_boolean (js_arg))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "It is not a boolean.");
  }

  bool *dest = c_arg_p->dest;
  *dest = jerry_value_is_true (js_arg);

  return jerry_undefined ();
} /* jerryx_arg_transform_boolean_strict */

/**
 * Transform a JS argument to a boolean. Type coercion is allowed.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_boolean (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                              const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  jerry_value_t js_arg = jerryx_arg_js_iterator_pop (js_arg_iter_p);

  bool to_boolean = jerry_value_to_boolean (js_arg);

  bool *dest = c_arg_p->dest;
  *dest = to_boolean;

  return jerry_undefined ();
} /* jerryx_arg_transform_boolean */

/**
 * The common routine for string transformer.
 * It works for both CESU-8 and UTF-8 string.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
static jerry_value_t
jerryx_arg_string_to_buffer_common_routine (jerry_value_t js_arg, /**< JS arg */
                                            const jerryx_arg_t *c_arg_p, /**< native arg */
                                            jerry_encoding_t encoding) /**< string encoding */
{
  jerry_char_t *target_p = (jerry_char_t *) c_arg_p->dest;
  jerry_size_t target_buf_size = (jerry_size_t) c_arg_p->extra_info;

  jerry_size_t size = jerry_string_size (js_arg, encoding);

  if (size > target_buf_size - 1)
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "Buffer size is not large enough.");
  }

  jerry_string_to_buffer (js_arg, encoding, target_p, target_buf_size);
  target_p[size] = '\0';

  return jerry_undefined ();
} /* jerryx_arg_string_to_buffer_common_routine */

/**
 * Transform a JS argument to a UTF-8/CESU-8 char array. Type coercion is not allowed.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
static jerry_value_t
jerryx_arg_transform_string_strict_common (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                           const jerryx_arg_t *c_arg_p, /**< the native arg */
                                           jerry_encoding_t encoding) /**< string encoding */
{
  jerry_value_t js_arg = jerryx_arg_js_iterator_pop (js_arg_iter_p);

  if (!jerry_value_is_string (js_arg))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "It is not a string.");
  }

  return jerryx_arg_string_to_buffer_common_routine (js_arg, c_arg_p, encoding);
} /* jerryx_arg_transform_string_strict_common */

/**
 * Transform a JS argument to a UTF-8/CESU-8 char array. Type coercion is allowed.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
static jerry_value_t
jerryx_arg_transform_string_common (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                    const jerryx_arg_t *c_arg_p, /**< the native arg */
                                    jerry_encoding_t encoding) /**< string encoding */
{
  jerry_value_t js_arg = jerryx_arg_js_iterator_pop (js_arg_iter_p);

  jerry_value_t to_string = jerry_value_to_string (js_arg);

  if (jerry_value_is_exception (to_string))
  {
    jerry_value_free (to_string);

    return jerry_throw_sz (JERRY_ERROR_TYPE, "It can not be converted to a string.");
  }

  jerry_value_t ret = jerryx_arg_string_to_buffer_common_routine (to_string, c_arg_p, encoding);
  jerry_value_free (to_string);

  return ret;
} /* jerryx_arg_transform_string_common */

/**
 * Transform a JS argument to a cesu8 char array. Type coercion is not allowed.
 *
 * Note:
 *      returned value must be freed with jerry_value_free, when it is no longer needed.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_string_strict (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                    const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  return jerryx_arg_transform_string_strict_common (js_arg_iter_p, c_arg_p, JERRY_ENCODING_CESU8);
} /* jerryx_arg_transform_string_strict */

/**
 * Transform a JS argument to a utf8 char array. Type coercion is not allowed.
 *
 * Note:
 *      returned value must be freed with jerry_value_free, when it is no longer needed.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_utf8_string_strict (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                         const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  return jerryx_arg_transform_string_strict_common (js_arg_iter_p, c_arg_p, JERRY_ENCODING_UTF8);
} /* jerryx_arg_transform_utf8_string_strict */

/**
 * Transform a JS argument to a cesu8 char array. Type coercion is allowed.
 *
 * Note:
 *      returned value must be freed with jerry_value_free, when it is no longer needed.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_string (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                             const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  return jerryx_arg_transform_string_common (js_arg_iter_p, c_arg_p, JERRY_ENCODING_CESU8);
} /* jerryx_arg_transform_string */

/**
 * Transform a JS argument to a utf8 char array. Type coercion is allowed.
 *
 * Note:
 *      returned value must be freed with jerry_value_free, when it is no longer needed.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_utf8_string (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                  const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  return jerryx_arg_transform_string_common (js_arg_iter_p, c_arg_p, JERRY_ENCODING_UTF8);
} /* jerryx_arg_transform_utf8_string */

/**
 * Check whether the JS argument is jerry function, if so, assign to the native argument.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_function (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                               const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  jerry_value_t js_arg = jerryx_arg_js_iterator_pop (js_arg_iter_p);

  if (!jerry_value_is_function (js_arg))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "It is not a function.");
  }

  jerry_value_t *func_p = c_arg_p->dest;
  *func_p = jerry_value_copy (js_arg);

  return jerry_undefined ();
} /* jerryx_arg_transform_function */

/**
 * Check whether the native pointer has the expected type info.
 * If so, assign it to the native argument.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_native_pointer (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                     const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  jerry_value_t js_arg = jerryx_arg_js_iterator_pop (js_arg_iter_p);

  if (!jerry_value_is_object (js_arg))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "It is not an object.");
  }

  const jerry_object_native_info_t *expected_info_p;
  expected_info_p = (const jerry_object_native_info_t *) c_arg_p->extra_info;
  void **ptr_p = (void **) c_arg_p->dest;
  *ptr_p = jerry_object_get_native_ptr (js_arg, expected_info_p);

  if (*ptr_p == NULL)
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "The object has no native pointer or type does not match.");
  }

  return jerry_undefined ();
} /* jerryx_arg_transform_native_pointer */

/**
 * Check whether the JS object's properties have expected types, and transform them into native args.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_object_props (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                   const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  jerry_value_t js_arg = jerryx_arg_js_iterator_pop (js_arg_iter_p);

  const jerryx_arg_object_props_t *object_props = (const jerryx_arg_object_props_t *) c_arg_p->extra_info;

  return jerryx_arg_transform_object_properties (js_arg,
                                                 object_props->name_p,
                                                 object_props->name_cnt,
                                                 object_props->c_arg_p,
                                                 object_props->c_arg_cnt);
} /* jerryx_arg_transform_object_props */

/**
 * Check whether the JS array's items have expected types, and transform them into native args.
 *
 * @return jerry undefined: the transformer passes,
 *         jerry error: the transformer fails.
 */
jerry_value_t
jerryx_arg_transform_array_items (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                  const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  jerry_value_t js_arg = jerryx_arg_js_iterator_pop (js_arg_iter_p);

  const jerryx_arg_array_items_t *array_items_p = (const jerryx_arg_array_items_t *) c_arg_p->extra_info;

  return jerryx_arg_transform_array (js_arg, array_items_p->c_arg_p, array_items_p->c_arg_cnt);
} /* jerryx_arg_transform_array_items */

/**
 * Define transformer for optional argument.
 */
#define JERRYX_ARG_TRANSFORM_OPTIONAL(type)                                                      \
  jerry_value_t jerryx_arg_transform_##type##_optional (jerryx_arg_js_iterator_t *js_arg_iter_p, \
                                                        const jerryx_arg_t *c_arg_p)             \
  {                                                                                              \
    return jerryx_arg_transform_optional (js_arg_iter_p, c_arg_p, jerryx_arg_transform_##type);  \
  }

JERRYX_ARG_TRANSFORM_OPTIONAL (number)
JERRYX_ARG_TRANSFORM_OPTIONAL (number_strict)
JERRYX_ARG_TRANSFORM_OPTIONAL (boolean)
JERRYX_ARG_TRANSFORM_OPTIONAL (boolean_strict)
JERRYX_ARG_TRANSFORM_OPTIONAL (string)
JERRYX_ARG_TRANSFORM_OPTIONAL (string_strict)
JERRYX_ARG_TRANSFORM_OPTIONAL (utf8_string)
JERRYX_ARG_TRANSFORM_OPTIONAL (utf8_string_strict)
JERRYX_ARG_TRANSFORM_OPTIONAL (function)
JERRYX_ARG_TRANSFORM_OPTIONAL (native_pointer)
JERRYX_ARG_TRANSFORM_OPTIONAL (object_props)
JERRYX_ARG_TRANSFORM_OPTIONAL (array_items)

JERRYX_ARG_TRANSFORM_OPTIONAL (uint8)
JERRYX_ARG_TRANSFORM_OPTIONAL (uint16)
JERRYX_ARG_TRANSFORM_OPTIONAL (uint32)
JERRYX_ARG_TRANSFORM_OPTIONAL (int8)
JERRYX_ARG_TRANSFORM_OPTIONAL (int16)
JERRYX_ARG_TRANSFORM_OPTIONAL (int32)
JERRYX_ARG_TRANSFORM_OPTIONAL (int8_strict)
JERRYX_ARG_TRANSFORM_OPTIONAL (int16_strict)
JERRYX_ARG_TRANSFORM_OPTIONAL (int32_strict)
JERRYX_ARG_TRANSFORM_OPTIONAL (uint8_strict)
JERRYX_ARG_TRANSFORM_OPTIONAL (uint16_strict)
JERRYX_ARG_TRANSFORM_OPTIONAL (uint32_strict)

#undef JERRYX_ARG_TRANSFORM_OPTIONAL

/**
 * Ignore the JS argument.
 *
 * @return jerry undefined
 */
jerry_value_t
jerryx_arg_transform_ignore (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                             const jerryx_arg_t *c_arg_p) /**< the native arg */
{
  (void) js_arg_iter_p; /* unused */
  (void) c_arg_p; /* unused */

  return jerry_undefined ();
} /* jerryx_arg_transform_ignore */

#include <stdlib.h>


#ifndef JERRYX_HANDLE_SCOPE_INTERNAL_H
#define JERRYX_HANDLE_SCOPE_INTERNAL_H



JERRY_C_API_BEGIN

/** MARK: - handle-scope-allocator.c */
typedef struct jerryx_handle_scope_pool_s jerryx_handle_scope_pool_t;
/**
 * A linear allocating memory pool for type jerryx_handle_scope_t,
 * in which allocated item shall be released in reversed order of allocation
 */
struct jerryx_handle_scope_pool_s
{
  jerryx_handle_scope_t prelist[JERRYX_SCOPE_PRELIST_SIZE]; /**< inlined handle scopes in the pool */
  size_t count; /**< number of handle scopes in the pool */
  jerryx_handle_scope_dynamic_t *start; /**< start address of dynamically allocated handle scope list */
};

jerryx_handle_scope_t *jerryx_handle_scope_get_parent (jerryx_handle_scope_t *scope);

jerryx_handle_scope_t *jerryx_handle_scope_get_child (jerryx_handle_scope_t *scope);

jerryx_handle_scope_t *jerryx_handle_scope_alloc (void);

void jerryx_handle_scope_free (jerryx_handle_scope_t *scope);
/** MARK: - END handle-scope-allocator.c */

/** MARK: - handle-scope.c */
void jerryx_handle_scope_release_handles (jerryx_handle_scope scope);

jerry_value_t jerryx_hand_scope_escape_handle_from_prelist (jerryx_handle_scope scope, size_t idx);

jerry_value_t jerryx_handle_scope_add_handle_to (jerryx_handle_t *handle, jerryx_handle_scope scope);

jerryx_handle_scope_status jerryx_escape_handle_internal (jerryx_escapable_handle_scope scope,
                                                          jerry_value_t escapee,
                                                          jerry_value_t *result,
                                                          bool should_promote);
/** MARK: - END handle-scope.c */

JERRY_C_API_END

#endif /* !JERRYX_HANDLE_SCOPE_INTERNAL_H */

JERRYX_STATIC_ASSERT (JERRYX_SCOPE_PRELIST_SIZE < 32, JERRYX_SCOPE_PRELIST_SIZE_MUST_BE_LESS_THAN_SIZE_OF_UINT8_T);

/**
 * Opens a new handle scope and attach it to current global scope as a child scope.
 *
 * @param result - [out value] opened scope.
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_open_handle_scope (jerryx_handle_scope *result)
{
  *result = jerryx_handle_scope_alloc ();
  return jerryx_handle_scope_ok;
} /* jerryx_open_handle_scope */

/**
 * Release all jerry values attached to given scope
 *
 * @param scope - the scope of handles to be released.
 */
void
jerryx_handle_scope_release_handles (jerryx_handle_scope scope)
{
  size_t prelist_handle_count = scope->prelist_handle_count;
  if (prelist_handle_count == JERRYX_HANDLE_PRELIST_SIZE && scope->handle_ptr != NULL)
  {
    jerryx_handle_t *a_handle = scope->handle_ptr;
    while (a_handle != NULL)
    {
      jerry_value_free (a_handle->jval);
      jerryx_handle_t *sibling = a_handle->sibling;
      jerry_heap_free (a_handle, sizeof (jerryx_handle_t));
      a_handle = sibling;
    }
    scope->handle_ptr = NULL;
    prelist_handle_count = JERRYX_HANDLE_PRELIST_SIZE;
  }

  for (size_t idx = 0; idx < prelist_handle_count; idx++)
  {
    jerry_value_free (scope->handle_prelist[idx]);
  }
  scope->prelist_handle_count = 0;
} /* jerryx_handle_scope_release_handles */

/**
 * Close the scope and its child scopes and release all jerry values that
 * resides in the scopes.
 * Scopes must be closed in the reverse order from which they were created.
 *
 * @param scope - the scope closed.
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_close_handle_scope (jerryx_handle_scope scope)
{
  /**
   * Release all handles related to given scope and its child scopes
   */
  jerryx_handle_scope a_scope = scope;
  do
  {
    jerryx_handle_scope_release_handles (a_scope);
    jerryx_handle_scope child = jerryx_handle_scope_get_child (a_scope);
    jerryx_handle_scope_free (a_scope);
    a_scope = child;
  } while (a_scope != NULL);

  return jerryx_handle_scope_ok;
} /* jerryx_close_handle_scope */

/**
 * Opens a new handle scope from which one object can be promoted to the outer scope
 * and attach it to current global scope as a child scope.
 *
 * @param result - [out value] opened escapable handle scope.
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_open_escapable_handle_scope (jerryx_handle_scope *result)
{
  return jerryx_open_handle_scope (result);
} /* jerryx_open_escapable_handle_scope */

/**
 * Close the scope and its child scopes and release all jerry values that
 * resides in the scopes.
 * Scopes must be closed in the reverse order from which they were created.
 *
 * @param scope - the one to be closed.
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_close_escapable_handle_scope (jerryx_handle_scope scope)
{
  return jerryx_close_handle_scope (scope);
} /* jerryx_close_escapable_handle_scope */

/**
 * Internal helper.
 * Escape a jerry value from the scope, yet did not promote it to outer scope.
 * An assertion of if parent exists shall be made before invoking this function.
 *
 * @param scope - scope of the handle added to.
 * @param idx - expected index of the handle in the scope's prelist.
 * @returns escaped jerry value id
 */
jerry_value_t
jerryx_hand_scope_escape_handle_from_prelist (jerryx_handle_scope scope, size_t idx)
{
  jerry_value_t jval = scope->handle_prelist[idx];
  if (scope->prelist_handle_count == JERRYX_HANDLE_PRELIST_SIZE && scope->handle_ptr != NULL)
  {
    jerryx_handle_t *handle = scope->handle_ptr;
    scope->handle_ptr = handle->sibling;
    scope->handle_prelist[idx] = handle->jval;
    jerry_heap_free (handle, sizeof (jerryx_handle_t));
    return jval;
  }

  if (idx < JERRYX_HANDLE_PRELIST_SIZE - 1)
  {
    scope->handle_prelist[idx] = scope->handle_prelist[scope->prelist_handle_count - 1];
  }
  return jval;
} /* jerryx_hand_scope_escape_handle_from_prelist */

/**
 * Internal helper.
 * Escape a jerry value from the given escapable handle scope.
 *
 * @param scope - the expected scope to be escaped from.
 * @param escapee - the jerry value to be escaped.
 * @param result - [out value] escaped jerry value result.
 * @param should_promote - true if the escaped value should be added to parent
 *                         scope after escaped from the given handle scope.
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_escape_handle_internal (jerryx_escapable_handle_scope scope,
                               jerry_value_t escapee,
                               jerry_value_t *result,
                               bool should_promote)
{
  if (scope->escaped)
  {
    return jerryx_escape_called_twice;
  }

  jerryx_handle_scope parent = jerryx_handle_scope_get_parent (scope);
  if (parent == NULL)
  {
    return jerryx_handle_scope_mismatch;
  }

  bool found = false;
  {
    size_t found_idx = 0;
    size_t prelist_count = scope->prelist_handle_count;
    /**
     * Search prelist in a reversed order since last added handle
     * is possible the one to be escaped
     */
    for (size_t idx_plus_1 = prelist_count; idx_plus_1 > 0; --idx_plus_1)
    {
      if (escapee == scope->handle_prelist[idx_plus_1 - 1])
      {
        found = true;
        found_idx = idx_plus_1 - 1;
        break;
      }
    }

    if (found)
    {
      *result = jerryx_hand_scope_escape_handle_from_prelist (scope, found_idx);

      --scope->prelist_handle_count;
      if (should_promote)
      {
        scope->escaped = true;
        /**
         * Escape handle to parent scope
         */
        jerryx_create_handle_in_scope (*result, jerryx_handle_scope_get_parent (scope));
      }
      return jerryx_handle_scope_ok;
    }
  };

  if (scope->prelist_handle_count <= JERRYX_HANDLE_PRELIST_SIZE && scope->handle_ptr == NULL)
  {
    return jerryx_handle_scope_mismatch;
  }

  /**
   * Handle chain list is already in an reversed order,
   * search through it as it is
   */
  jerryx_handle_t *handle = scope->handle_ptr;
  jerryx_handle_t *memo_handle = NULL;
  jerryx_handle_t *found_handle = NULL;
  while (!found)
  {
    if (handle == NULL)
    {
      return jerryx_handle_scope_mismatch;
    }
    if (handle->jval != escapee)
    {
      memo_handle = handle;
      handle = handle->sibling;
      continue;
    }
    /**
     * Remove found handle from current scope's handle chain
     */
    found = true;
    found_handle = handle;
    if (memo_handle == NULL)
    {
      // found handle is the first handle in heap
      scope->handle_ptr = found_handle->sibling;
    }
    else
    {
      memo_handle->sibling = found_handle->sibling;
    }
  }

  if (should_promote)
  {
    /**
     * Escape handle to parent scope
     */
    *result = jerryx_handle_scope_add_handle_to (found_handle, parent);
  }

  if (should_promote)
  {
    scope->escaped = true;
  }
  return jerryx_handle_scope_ok;
} /* jerryx_escape_handle_internal */

/**
 * Promotes the handle to the JavaScript object so that it is valid for the lifetime of
 * the outer scope. It can only be called once per scope. If it is called more than
 * once an error will be returned.
 *
 * @param scope - the expected scope to be escaped from.
 * @param escapee - the jerry value to be escaped.
 * @param result - [out value] escaped jerry value result.
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_escape_handle (jerryx_escapable_handle_scope scope, jerry_value_t escapee, jerry_value_t *result)
{
  return jerryx_escape_handle_internal (scope, escapee, result, true);
} /* jerryx_escape_handle */

/**
 * Escape a handle from scope yet do not promote it to the outer scope.
 * Leave the handle's life time management up to user.
 *
 * @param scope - the expected scope to be removed from.
 * @param escapee - the jerry value to be removed.
 * @param result - [out value] removed jerry value result.
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_remove_handle (jerryx_escapable_handle_scope scope, jerry_value_t escapee, jerry_value_t *result)
{
  return jerryx_escape_handle_internal (scope, escapee, result, false);
} /* jerryx_remove_handle */

/**
 * Try to reuse given handle if possible while adding to the scope.
 *
 * @param handle - the one to be added to the scope.
 * @param scope - the scope of handle to be added to.
 * @returns the jerry value id wrapped by given handle.
 */
jerry_value_t
jerryx_handle_scope_add_handle_to (jerryx_handle_t *handle, jerryx_handle_scope scope)
{
  size_t prelist_handle_count = scope->prelist_handle_count;
  if (prelist_handle_count < JERRYX_HANDLE_PRELIST_SIZE)
  {
    ++scope->prelist_handle_count;
    jerry_value_t jval = handle->jval;
    jerry_heap_free (handle, sizeof (jerryx_handle_t));
    scope->handle_prelist[prelist_handle_count] = jval;
    return jval;
  }

  handle->sibling = scope->handle_ptr;
  scope->handle_ptr = handle;
  return handle->jval;
} /* jerryx_handle_scope_add_handle_to */

/**
 * Add given jerry value to the scope.
 *
 * @param jval - jerry value to be added to scope.
 * @param scope - the scope of the jerry value been expected to be added to.
 * @return jerry value that added to scope.
 */
jerry_value_t
jerryx_create_handle_in_scope (jerry_value_t jval, jerryx_handle_scope scope)
{
  size_t prelist_handle_count = scope->prelist_handle_count;
  if (prelist_handle_count < JERRYX_HANDLE_PRELIST_SIZE)
  {
    scope->handle_prelist[prelist_handle_count] = jval;

    ++scope->prelist_handle_count;
    return jval;
  }
  jerryx_handle_t *handle = (jerryx_handle_t *) jerry_heap_alloc (sizeof (jerryx_handle_t));
  JERRYX_ASSERT (handle != NULL);
  handle->jval = jval;

  handle->sibling = scope->handle_ptr;
  scope->handle_ptr = handle;

  return jval;
} /* jerryx_create_handle_in_scope */

/**
 * Add given jerry value to current top scope.
 *
 * @param jval - jerry value to be added to scope.
 * @return jerry value that added to scope.
 */
jerry_value_t
jerryx_create_handle (jerry_value_t jval)
{
  return jerryx_create_handle_in_scope (jval, jerryx_handle_scope_get_current ());
} /* jerryx_create_handle */

#include <stdlib.h>


static jerryx_handle_scope_t jerryx_handle_scope_root = {
  .prelist_handle_count = 0,
  .handle_ptr = NULL,
};
static jerryx_handle_scope_t *jerryx_handle_scope_current = &jerryx_handle_scope_root;
static jerryx_handle_scope_pool_t jerryx_handle_scope_pool = {
  .count = 0,
  .start = NULL,
};

#define JERRYX_HANDLE_SCOPE_POOL_PRELIST_LAST jerryx_handle_scope_pool.prelist + JERRYX_SCOPE_PRELIST_SIZE - 1

#define JERRYX_HANDLE_SCOPE_PRELIST_IDX(scope) (scope - jerryx_handle_scope_pool.prelist)

/**
 * Get current handle scope top of stack.
 */
jerryx_handle_scope_t *
jerryx_handle_scope_get_current (void)
{
  return jerryx_handle_scope_current;
} /* jerryx_handle_scope_get_current */

/**
 * Get root handle scope.
 */
jerryx_handle_scope_t *
jerryx_handle_scope_get_root (void)
{
  return &jerryx_handle_scope_root;
} /* jerryx_handle_scope_get_root */

/**
 * Determines if given handle scope is located in pre-allocated list.
 *
 * @param scope - the one to be determined.
 */
static bool
jerryx_handle_scope_is_in_prelist (jerryx_handle_scope_t *scope)
{
  return (jerryx_handle_scope_pool.prelist <= scope)
         && (scope <= (jerryx_handle_scope_pool.prelist + JERRYX_SCOPE_PRELIST_SIZE - 1));
} /* jerryx_handle_scope_is_in_prelist */

/**
 * Get the parent of given handle scope.
 * If given handle scope is in prelist, the parent must be in prelist too;
 * if given is the first item of heap chain list, the parent must be the last one of prelist;
 * the parent must be in chain list otherwise.
 *
 * @param scope - the one to be permformed on.
 * @returns - the parent of the given scope.
 */
jerryx_handle_scope_t *
jerryx_handle_scope_get_parent (jerryx_handle_scope_t *scope)
{
  if (scope == &jerryx_handle_scope_root)
  {
    return NULL;
  }
  if (!jerryx_handle_scope_is_in_prelist (scope))
  {
    jerryx_handle_scope_dynamic_t *dy_scope = (jerryx_handle_scope_dynamic_t *) scope;
    if (dy_scope == jerryx_handle_scope_pool.start)
    {
      return JERRYX_HANDLE_SCOPE_POOL_PRELIST_LAST;
    }
    jerryx_handle_scope_dynamic_t *parent = dy_scope->parent;
    return (jerryx_handle_scope_t *) parent;
  }
  if (scope == jerryx_handle_scope_pool.prelist)
  {
    return &jerryx_handle_scope_root;
  }
  return jerryx_handle_scope_pool.prelist + JERRYX_HANDLE_SCOPE_PRELIST_IDX (scope) - 1;
} /* jerryx_handle_scope_get_parent */

/**
 * Get the child of given handle scope.
 * If the given handle scope is in heap chain list, its child must be in heap chain list too;
 * if the given handle scope is the last one of prelist, its child must be the first item of chain list;
 * the children are in prelist otherwise.
 *
 * @param scope - the one to be permformed on.
 * @returns the child of the given scope.
 */
jerryx_handle_scope_t *
jerryx_handle_scope_get_child (jerryx_handle_scope_t *scope)
{
  if (scope == &jerryx_handle_scope_root)
  {
    if (jerryx_handle_scope_pool.count > 0)
    {
      return jerryx_handle_scope_pool.prelist;
    }
    return NULL;
  }
  if (!jerryx_handle_scope_is_in_prelist (scope))
  {
    jerryx_handle_scope_dynamic_t *child = ((jerryx_handle_scope_dynamic_t *) scope)->child;
    return (jerryx_handle_scope_t *) child;
  }
  if (scope == JERRYX_HANDLE_SCOPE_POOL_PRELIST_LAST)
  {
    return (jerryx_handle_scope_t *) jerryx_handle_scope_pool.start;
  }
  ptrdiff_t idx = JERRYX_HANDLE_SCOPE_PRELIST_IDX (scope);
  if (idx < 0)
  {
    return NULL;
  }
  if ((unsigned long) idx >= jerryx_handle_scope_pool.count - 1)
  {
    return NULL;
  }
  return jerryx_handle_scope_pool.prelist + idx + 1;
} /* jerryx_handle_scope_get_child */

/**
 * Claims a handle scope either from prelist or allocating a new memory block,
 * and increment pool's scope count by 1, and set current scope to the newly claimed one.
 * If there are still available spaces in prelist, claims a block in prelist;
 * otherwise allocates a new memory block from heap and sets its fields to default values,
 * and link it to previously dynamically allocated scope, or link it to pool's start pointer.
 *
 * @returns the newly claimed handle scope pointer.
 */
jerryx_handle_scope_t *
jerryx_handle_scope_alloc (void)
{
  jerryx_handle_scope_t *scope;
  if (jerryx_handle_scope_pool.count < JERRYX_SCOPE_PRELIST_SIZE)
  {
    scope = jerryx_handle_scope_pool.prelist + jerryx_handle_scope_pool.count;
  }
  else
  {
    jerryx_handle_scope_dynamic_t *dy_scope;
    dy_scope = (jerryx_handle_scope_dynamic_t *) jerry_heap_alloc (sizeof (jerryx_handle_scope_dynamic_t));
    JERRYX_ASSERT (dy_scope != NULL);
    dy_scope->child = NULL;

    if (jerryx_handle_scope_pool.count != JERRYX_SCOPE_PRELIST_SIZE)
    {
      jerryx_handle_scope_dynamic_t *dy_current = (jerryx_handle_scope_dynamic_t *) jerryx_handle_scope_current;
      dy_scope->parent = dy_current;
      dy_current->child = dy_scope;
    }
    else
    {
      jerryx_handle_scope_pool.start = dy_scope;
      dy_scope->parent = NULL;
    }

    scope = (jerryx_handle_scope_t *) dy_scope;
  }

  scope->prelist_handle_count = 0;
  scope->escaped = false;
  scope->handle_ptr = NULL;

  jerryx_handle_scope_current = scope;
  ++jerryx_handle_scope_pool.count;
  return (jerryx_handle_scope_t *) scope;
} /* jerryx_handle_scope_alloc */

/**
 * Deannounce a previously claimed handle scope, return it to pool
 * or free the allocated memory block.
 *
 * @param scope - the one to be freed.
 */
void
jerryx_handle_scope_free (jerryx_handle_scope_t *scope)
{
  if (scope == &jerryx_handle_scope_root)
  {
    return;
  }

  --jerryx_handle_scope_pool.count;
  if (scope == jerryx_handle_scope_current)
  {
    jerryx_handle_scope_current = jerryx_handle_scope_get_parent (scope);
  }

  if (!jerryx_handle_scope_is_in_prelist (scope))
  {
    jerryx_handle_scope_dynamic_t *dy_scope = (jerryx_handle_scope_dynamic_t *) scope;
    if (dy_scope == jerryx_handle_scope_pool.start)
    {
      jerryx_handle_scope_pool.start = dy_scope->child;
    }
    else if (dy_scope->parent != NULL)
    {
      dy_scope->parent->child = dy_scope->child;
    }
    jerry_heap_free (dy_scope, sizeof (jerryx_handle_scope_dynamic_t));
    return;
  }
  /**
   * Nothing to do with scopes in prelist
   */
} /* jerryx_handle_scope_free */


#include <string.h>


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


#include <string.h>



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


#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>



jerry_value_t
jerryx_source_parse_script (const char *path_p)
{
  jerry_size_t source_size;
  jerry_char_t *source_p = jerry_port_source_read (path_p, &source_size);

  if (source_p == NULL)
  {
    jerry_log (JERRY_LOG_LEVEL_ERROR, "Failed to open file: %s\n", path_p);
    return jerry_throw_sz (JERRY_ERROR_SYNTAX, "Source file not found");
  }

  if (!jerry_validate_string (source_p, source_size, JERRY_ENCODING_UTF8))
  {
    jerry_port_source_free (source_p);
    return jerry_throw_sz (JERRY_ERROR_SYNTAX, "Input is not a valid UTF-8 encoded string.");
  }

  jerry_parse_options_t parse_options;
  parse_options.options = JERRY_PARSE_HAS_SOURCE_NAME;
  parse_options.source_name =
    jerry_string ((const jerry_char_t *) path_p, (jerry_size_t) strlen (path_p), JERRY_ENCODING_UTF8);

  jerry_value_t result = jerry_parse (source_p, source_size, &parse_options);

  jerry_value_free (parse_options.source_name);
  jerry_port_source_free (source_p);

  return result;
} /* jerryx_source_parse_script */

jerry_value_t
jerryx_source_exec_script (const char *path_p)
{
  jerry_value_t result = jerryx_source_parse_script (path_p);

  if (!jerry_value_is_exception (result))
  {
    jerry_value_t script = result;
    result = jerry_run (script);
    jerry_value_free (script);
  }

  return result;
} /* jerryx_source_exec_script */

jerry_value_t
jerryx_source_exec_module (const char *path_p)
{
  jerry_value_t specifier =
    jerry_string ((const jerry_char_t *) path_p, (jerry_size_t) strlen (path_p), JERRY_ENCODING_UTF8);
  jerry_value_t referrer = jerry_undefined ();

  jerry_value_t module = jerry_module_resolve (specifier, referrer, NULL);

  jerry_value_free (referrer);
  jerry_value_free (specifier);

  if (jerry_value_is_exception (module))
  {
    return module;
  }

  if (jerry_module_state (module) == JERRY_MODULE_STATE_UNLINKED)
  {
    jerry_value_t link_result = jerry_module_link (module, NULL, NULL);

    if (jerry_value_is_exception (link_result))
    {
      jerry_value_free (module);
      return link_result;
    }

    jerry_value_free (link_result);
  }

  jerry_value_t result = jerry_module_evaluate (module);
  jerry_value_free (module);

  jerry_module_cleanup (jerry_undefined ());
  return result;
} /* jerryx_source_exec_module */

jerry_value_t
jerryx_source_exec_snapshot (const char *path_p, size_t function_index)
{
  jerry_size_t source_size;
  jerry_char_t *source_p = jerry_port_source_read (path_p, &source_size);

  if (source_p == NULL)
  {
    jerry_log (JERRY_LOG_LEVEL_ERROR, "Failed to open file: %s\n", path_p);
    return jerry_throw_sz (JERRY_ERROR_SYNTAX, "Snapshot file not found");
  }

  jerry_value_t result =
    jerry_exec_snapshot ((uint32_t *) source_p, source_size, function_index, JERRY_SNAPSHOT_EXEC_COPY_DATA, NULL);

  jerry_port_source_free (source_p);
  return result;
} /* jerryx_source_exec_snapshot */

jerry_value_t
jerryx_source_exec_stdin (void)
{
  jerry_char_t *source_p = NULL;
  jerry_size_t source_size = 0;

  while (true)
  {
    jerry_size_t line_size;
    jerry_char_t *line_p = jerry_port_line_read (&line_size);

    if (line_p == NULL)
    {
      break;
    }

    jerry_size_t new_size = source_size + line_size;
    source_p = realloc (source_p, new_size);

    memcpy (source_p + source_size, line_p, line_size);
    jerry_port_line_free (line_p);
    source_size = new_size;
  }

  if (!jerry_validate_string (source_p, source_size, JERRY_ENCODING_UTF8))
  {
    free (source_p);
    return jerry_throw_sz (JERRY_ERROR_SYNTAX, "Input is not a valid UTF-8 encoded string.");
  }

  jerry_value_t result = jerry_parse (source_p, source_size, NULL);
  free (source_p);

  if (jerry_value_is_exception (result))
  {
    return result;
  }

  jerry_value_t script = result;
  result = jerry_run (script);
  jerry_value_free (script);

  return result;
} /* jerryx_source_exec_stdin */


#include <assert.h>
#include <string.h>



/**
 * Register a method for the $262 object.
 */
static void
jerryx_test262_register_function (jerry_value_t test262_obj, /** $262 object */
                                  const char *name_p, /**< name of the function */
                                  jerry_external_handler_t handler_p) /**< function callback */
{
  jerry_value_t function_val = jerry_function_external (handler_p);
  jerry_value_t result_val = jerry_object_set_sz (test262_obj, name_p, function_val);
  jerry_value_free (function_val);

  assert (!jerry_value_is_exception (result_val));
  jerry_value_free (result_val);
} /* jerryx_test262_register_function */

/**
 * $262.detachArrayBuffer
 *
 * A function which implements the DetachArrayBuffer abstract operation
 *
 * @return null value - if success
 *         value marked with error flag - otherwise
 */
static jerry_value_t
jerryx_test262_detach_array_buffer (const jerry_call_info_t *call_info_p, /**< call information */
                                    const jerry_value_t args_p[], /**< function arguments */
                                    const jerry_length_t args_cnt) /**< number of function arguments */
{
  (void) call_info_p; /* unused */

  if (args_cnt < 1 || !jerry_value_is_arraybuffer (args_p[0]))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "Expected an ArrayBuffer object");
  }

  /* TODO: support the optional 'key' argument */

  return jerry_arraybuffer_detach (args_p[0]);
} /* jerryx_test262_detach_array_buffer */

/**
 * $262.evalScript
 *
 * A function which accepts a string value as its first argument and executes it
 *
 * @return completion of the script parsing and execution.
 */
static jerry_value_t
jerryx_test262_eval_script (const jerry_call_info_t *call_info_p, /**< call information */
                            const jerry_value_t args_p[], /**< function arguments */
                            const jerry_length_t args_cnt) /**< number of function arguments */
{
  (void) call_info_p; /* unused */

  if (args_cnt < 1 || !jerry_value_is_string (args_p[0]))
  {
    return jerry_throw_sz (JERRY_ERROR_TYPE, "Expected a string");
  }

  jerry_value_t ret_value = jerry_parse_value (args_p[0], NULL);

  if (!jerry_value_is_exception (ret_value))
  {
    jerry_value_t func_val = ret_value;
    ret_value = jerry_run (func_val);
    jerry_value_free (func_val);
  }

  return ret_value;
} /* jerryx_test262_eval_script */

static jerry_value_t jerryx_test262_create (jerry_value_t global_obj);

/**
 * $262.createRealm
 *
 * A function which creates a new realm object, and returns a newly created $262 object
 *
 * @return a new $262 object
 */
static jerry_value_t
jerryx_test262_create_realm (const jerry_call_info_t *call_info_p, /**< call information */
                             const jerry_value_t args_p[], /**< function arguments */
                             const jerry_length_t args_cnt) /**< number of function arguments */
{
  (void) call_info_p; /* unused */
  (void) args_p; /* unused */
  (void) args_cnt; /* unused */

  jerry_value_t realm_object = jerry_realm ();
  jerry_value_t previous_realm = jerry_set_realm (realm_object);
  assert (!jerry_value_is_exception (previous_realm));

  jerry_value_t test262_object = jerryx_test262_create (realm_object);
  jerry_set_realm (previous_realm);
  jerry_value_free (realm_object);

  return test262_object;
} /* jerryx_test262_create_realm */

/**
 * Create a new $262 object
 *
 * @return a new $262 object
 */
static jerry_value_t
jerryx_test262_create (jerry_value_t global_obj) /**< global object */
{
  jerry_value_t test262_object = jerry_object ();

  jerryx_test262_register_function (test262_object, "detachArrayBuffer", jerryx_test262_detach_array_buffer);
  jerryx_test262_register_function (test262_object, "evalScript", jerryx_test262_eval_script);
  jerryx_test262_register_function (test262_object, "createRealm", jerryx_test262_create_realm);
  jerryx_test262_register_function (test262_object, "gc", jerryx_handler_gc);

  jerry_value_t result = jerry_object_set_sz (test262_object, "global", global_obj);
  assert (!jerry_value_is_exception (result));
  jerry_value_free (result);

  return test262_object;
} /* create_test262 */

/**
 * Add a new test262 object to the current global object.
 */
void
jerryx_test262_register (void)
{
  jerry_value_t global_obj = jerry_current_realm ();
  jerry_value_t test262_obj = jerryx_test262_create (global_obj);

  jerry_value_t result = jerry_object_set_sz (global_obj, "$262", test262_obj);
  assert (!jerry_value_is_exception (result));

  jerry_value_free (result);
  jerry_value_free (test262_obj);
  jerry_value_free (global_obj);
} /* jerryx_test262_register */


#ifndef DEBUGGER_SHA1_H
#define DEBUGGER_SHA1_H


#if defined(JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)

/* JerryScript debugger protocol is a simplified version of RFC-6455 (WebSockets). */

void jerryx_debugger_compute_sha1 (const uint8_t *input1,
                                   size_t input1_len,
                                   const uint8_t *input2,
                                   size_t input2_len,
                                   uint8_t output[20]);

#endif /* defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1) */

#endif /* !DEBUGGER_SHA1_H */

#if defined(JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)

/* JerryScript debugger protocol is a simplified version of RFC-6455 (WebSockets). */

/**
 * Last fragment of a Websocket package.
 */
#define JERRYX_DEBUGGER_WEBSOCKET_FIN_BIT 0x80

/**
 * Masking-key is available.
 */
#define JERRYX_DEBUGGER_WEBSOCKET_MASK_BIT 0x80

/**
 * Opcode type mask.
 */
#define JERRYX_DEBUGGER_WEBSOCKET_OPCODE_MASK 0x0fu

/**
 * Packet length mask.
 */
#define JERRYX_DEBUGGER_WEBSOCKET_LENGTH_MASK 0x7fu

/**
 * Size of websocket header size.
 */
#define JERRYX_DEBUGGER_WEBSOCKET_HEADER_SIZE 2

/**
 * Payload mask size in bytes of a websocket package.
 */
#define JERRYX_DEBUGGER_WEBSOCKET_MASK_SIZE 4

/**
 * Maximum message size with 1 byte size field.
 */
#define JERRYX_DEBUGGER_WEBSOCKET_ONE_BYTE_LEN_MAX 125

/**
 * WebSocket opcode types.
 */
typedef enum
{
  JERRYX_DEBUGGER_WEBSOCKET_TEXT_FRAME = 1, /**< text frame */
  JERRYX_DEBUGGER_WEBSOCKET_BINARY_FRAME = 2, /**< binary frame */
  JERRYX_DEBUGGER_WEBSOCKET_CLOSE_CONNECTION = 8, /**< close connection */
  JERRYX_DEBUGGER_WEBSOCKET_PING = 9, /**< ping (keep alive) frame */
  JERRYX_DEBUGGER_WEBSOCKET_PONG = 10, /**< reply to ping frame */
} jerryx_websocket_opcode_type_t;

/**
 * Header for incoming packets.
 */
typedef struct
{
  uint8_t ws_opcode; /**< websocket opcode */
  uint8_t size; /**< size of the message */
  uint8_t mask[4]; /**< mask bytes */
} jerryx_websocket_receive_header_t;

/**
 * Convert a 6-bit value to a Base64 character.
 *
 * @return Base64 character
 */
static uint8_t
jerryx_to_base64_character (uint8_t value) /**< 6-bit value */
{
  if (value < 26)
  {
    return (uint8_t) (value + 'A');
  }

  if (value < 52)
  {
    return (uint8_t) (value - 26 + 'a');
  }

  if (value < 62)
  {
    return (uint8_t) (value - 52 + '0');
  }

  if (value == 62)
  {
    return (uint8_t) '+';
  }

  return (uint8_t) '/';
} /* jerryx_to_base64_character */

/**
 * Encode a byte sequence into Base64 string.
 */
static void
jerryx_to_base64 (const uint8_t *source_p, /**< source data */
                  uint8_t *destination_p, /**< destination buffer */
                  size_t length) /**< length of source, must be divisible by 3 */
{
  while (length >= 3)
  {
    uint8_t value = (source_p[0] >> 2);
    destination_p[0] = jerryx_to_base64_character (value);

    value = (uint8_t) (((source_p[0] << 4) | (source_p[1] >> 4)) & 0x3f);
    destination_p[1] = jerryx_to_base64_character (value);

    value = (uint8_t) (((source_p[1] << 2) | (source_p[2] >> 6)) & 0x3f);
    destination_p[2] = jerryx_to_base64_character (value);

    value = (uint8_t) (source_p[2] & 0x3f);
    destination_p[3] = jerryx_to_base64_character (value);

    source_p += 3;
    destination_p += 4;
    length -= 3;
  }
} /* jerryx_to_base64 */

/**
 * Process WebSocket handshake.
 *
 * @return true - if the handshake was completed successfully
 *         false - otherwise
 */
static bool
jerryx_process_handshake (uint8_t *request_buffer_p) /**< temporary buffer */
{
  size_t request_buffer_size = 1024;
  uint8_t *request_end_p = request_buffer_p;

  /* Buffer request text until the double newlines are received. */
  while (true)
  {
    jerry_debugger_transport_receive_context_t context;
    if (!jerry_debugger_transport_receive (&context))
    {
      JERRYX_ASSERT (!jerry_debugger_transport_is_connected ());
      return false;
    }

    if (context.message_p == NULL)
    {
      jerry_debugger_transport_sleep ();
      continue;
    }

    size_t length = request_buffer_size - 1u - (size_t) (request_end_p - request_buffer_p);

    if (length < context.message_length)
    {
      JERRYX_ERROR_MSG ("Handshake buffer too small.\n");
      return false;
    }

    /* Both stream and datagram packets are supported. */
    memcpy (request_end_p, context.message_p, context.message_length);

    jerry_debugger_transport_receive_completed (&context);

    request_end_p += (size_t) context.message_length;
    *request_end_p = 0;

    if (request_end_p > request_buffer_p + 4 && memcmp (request_end_p - 4, "\r\n\r\n", 4) == 0)
    {
      break;
    }
  }

  /* Check protocol. */
  const char get_text[] = "GET /jerry-debugger";
  size_t text_len = sizeof (get_text) - 1;

  if ((size_t) (request_end_p - request_buffer_p) < text_len || memcmp (request_buffer_p, get_text, text_len) != 0)
  {
    JERRYX_ERROR_MSG ("Invalid handshake format.\n");
    return false;
  }

  uint8_t *websocket_key_p = request_buffer_p + text_len;

  const char key_text[] = "Sec-WebSocket-Key:";
  text_len = sizeof (key_text) - 1;

  while (true)
  {
    if ((size_t) (request_end_p - websocket_key_p) < text_len)
    {
      JERRYX_ERROR_MSG ("Sec-WebSocket-Key not found.\n");
      return false;
    }

    if (websocket_key_p[0] == 'S' && websocket_key_p[-1] == '\n' && websocket_key_p[-2] == '\r'
        && memcmp (websocket_key_p, key_text, text_len) == 0)
    {
      websocket_key_p += text_len;
      break;
    }

    websocket_key_p++;
  }

  /* String terminated by double newlines. */

  while (*websocket_key_p == ' ')
  {
    websocket_key_p++;
  }

  uint8_t *websocket_key_end_p = websocket_key_p;

  while (*websocket_key_end_p > ' ')
  {
    websocket_key_end_p++;
  }

  /* Since the request_buffer_p is not needed anymore it can
   * be reused for storing the SHA-1 key and Base64 string. */

  const size_t sha1_length = 20;

  jerryx_debugger_compute_sha1 (websocket_key_p,
                                (size_t) (websocket_key_end_p - websocket_key_p),
                                (const uint8_t *) "258EAFA5-E914-47DA-95CA-C5AB0DC85B11",
                                36,
                                request_buffer_p);

  /* The SHA-1 key is 20 bytes long but jerryx_to_base64 expects
   * a length divisible by 3 so an extra 0 is appended at the end. */
  request_buffer_p[sha1_length] = 0;

  jerryx_to_base64 (request_buffer_p, request_buffer_p + sha1_length + 1, sha1_length + 1);

  /* Last value must be replaced by equal sign. */

  const uint8_t response_prefix[] =
    "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";

  if (!jerry_debugger_transport_send (response_prefix, sizeof (response_prefix) - 1)
      || !jerry_debugger_transport_send (request_buffer_p + sha1_length + 1, 27))
  {
    return false;
  }

  const uint8_t response_suffix[] = "=\r\n\r\n";
  return jerry_debugger_transport_send (response_suffix, sizeof (response_suffix) - 1);
} /* jerryx_process_handshake */

/**
 * Close a tcp connection.
 */
static void
jerryx_debugger_ws_close (jerry_debugger_transport_header_t *header_p) /**< tcp implementation */
{
  JERRYX_ASSERT (!jerry_debugger_transport_is_connected ());

  jerry_heap_free ((void *) header_p, sizeof (jerry_debugger_transport_header_t));
} /* jerryx_debugger_ws_close */

/**
 * Send data over a websocket connection.
 *
 * @return true - if the data has been sent successfully
 *         false - otherwise
 */
static bool
jerryx_debugger_ws_send (jerry_debugger_transport_header_t *header_p, /**< tcp implementation */
                         uint8_t *message_p, /**< message to be sent */
                         size_t message_length) /**< message length in bytes */
{
  JERRYX_ASSERT (message_length <= JERRYX_DEBUGGER_WEBSOCKET_ONE_BYTE_LEN_MAX);

  message_p[-2] = JERRYX_DEBUGGER_WEBSOCKET_FIN_BIT | JERRYX_DEBUGGER_WEBSOCKET_BINARY_FRAME;
  message_p[-1] = (uint8_t) message_length;

  return header_p->next_p->send (header_p->next_p, message_p - 2, message_length + 2);
} /* jerryx_debugger_ws_send */

/**
 * Receive data from a websocket connection.
 */
static bool
jerryx_debugger_ws_receive (jerry_debugger_transport_header_t *header_p, /**< tcp implementation */
                            jerry_debugger_transport_receive_context_t *receive_context_p) /**< receive context */
{
  if (!header_p->next_p->receive (header_p->next_p, receive_context_p))
  {
    return false;
  }

  if (receive_context_p->message_p == NULL)
  {
    return true;
  }

  size_t message_total_length = receive_context_p->message_total_length;

  if (message_total_length == 0)
  {
    /* Byte stream. */
    if (receive_context_p->message_length < sizeof (jerryx_websocket_receive_header_t))
    {
      receive_context_p->message_p = NULL;
      return true;
    }
  }
  else
  {
    /* Datagram packet. */
    JERRYX_ASSERT (receive_context_p->message_length >= sizeof (jerryx_websocket_receive_header_t));
  }

  uint8_t *message_p = receive_context_p->message_p;

  if ((message_p[0] & ~JERRYX_DEBUGGER_WEBSOCKET_OPCODE_MASK) != JERRYX_DEBUGGER_WEBSOCKET_FIN_BIT
      || (message_p[1] & JERRYX_DEBUGGER_WEBSOCKET_LENGTH_MASK) > JERRYX_DEBUGGER_WEBSOCKET_ONE_BYTE_LEN_MAX
      || !(message_p[1] & JERRYX_DEBUGGER_WEBSOCKET_MASK_BIT))
  {
    JERRYX_ERROR_MSG ("Unsupported Websocket message.\n");
    jerry_debugger_transport_close ();
    return false;
  }

  if ((message_p[0] & JERRYX_DEBUGGER_WEBSOCKET_OPCODE_MASK) != JERRYX_DEBUGGER_WEBSOCKET_BINARY_FRAME)
  {
    JERRYX_ERROR_MSG ("Unsupported Websocket opcode.\n");
    jerry_debugger_transport_close ();
    return false;
  }

  size_t message_length = (size_t) (message_p[1] & JERRYX_DEBUGGER_WEBSOCKET_LENGTH_MASK);

  if (message_total_length == 0)
  {
    size_t new_total_length = message_length + sizeof (jerryx_websocket_receive_header_t);

    /* Byte stream. */
    if (receive_context_p->message_length < new_total_length)
    {
      receive_context_p->message_p = NULL;
      return true;
    }

    receive_context_p->message_total_length = new_total_length;
  }
  else
  {
    /* Datagram packet. */
    JERRYX_ASSERT (receive_context_p->message_length == (message_length + sizeof (jerryx_websocket_receive_header_t)));
  }

  message_p += sizeof (jerryx_websocket_receive_header_t);

  receive_context_p->message_p = message_p;
  receive_context_p->message_length = message_length;

  /* Unmask data bytes. */
  const uint8_t *mask_p = message_p - JERRYX_DEBUGGER_WEBSOCKET_MASK_SIZE;
  const uint8_t *mask_end_p = message_p;
  const uint8_t *message_end_p = message_p + message_length;

  while (message_p < message_end_p)
  {
    /* Invert certain bits with xor operation. */
    *message_p = *message_p ^ *mask_p;

    message_p++;
    mask_p++;

    if (JERRY_UNLIKELY (mask_p >= mask_end_p))
    {
      mask_p -= JERRYX_DEBUGGER_WEBSOCKET_MASK_SIZE;
    }
  }

  return true;
} /* jerryx_debugger_ws_receive */

/**
 * Initialize the websocket transportation layer.
 *
 * @return true - if the connection succeeded
 *         false - otherwise
 */
bool
jerryx_debugger_ws_create (void)
{
  bool is_handshake_ok = false;

  const jerry_size_t buffer_size = 1024;
  uint8_t *request_buffer_p = (uint8_t *) jerry_heap_alloc (buffer_size);

  if (!request_buffer_p)
  {
    return false;
  }

  is_handshake_ok = jerryx_process_handshake (request_buffer_p);

  jerry_heap_free ((void *) request_buffer_p, buffer_size);

  if (!is_handshake_ok && jerry_debugger_transport_is_connected ())
  {
    return false;
  }

  const jerry_size_t interface_size = sizeof (jerry_debugger_transport_header_t);
  jerry_debugger_transport_header_t *header_p;
  header_p = (jerry_debugger_transport_header_t *) jerry_heap_alloc (interface_size);

  if (!header_p)
  {
    return false;
  }

  header_p->close = jerryx_debugger_ws_close;
  header_p->send = jerryx_debugger_ws_send;
  header_p->receive = jerryx_debugger_ws_receive;

  jerry_debugger_transport_add (header_p,
                                JERRYX_DEBUGGER_WEBSOCKET_HEADER_SIZE,
                                JERRYX_DEBUGGER_WEBSOCKET_ONE_BYTE_LEN_MAX,
                                JERRYX_DEBUGGER_WEBSOCKET_HEADER_SIZE + JERRYX_DEBUGGER_WEBSOCKET_MASK_SIZE,
                                JERRYX_DEBUGGER_WEBSOCKET_ONE_BYTE_LEN_MAX);

  return true;
} /* jerryx_debugger_ws_create */

#else /* !(defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)) */

/**
 * Dummy function when debugger is disabled.
 *
 * @return false
 */
bool
jerryx_debugger_ws_create (void)
{
  return false;
} /* jerryx_debugger_ws_create */

#endif /* defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1) */



#if defined(JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)

#include <errno.h>

#ifdef _WIN32
#include <BaseTsd.h>

#include <WS2tcpip.h>
#include <winsock2.h>

/* On Windows the WSAEWOULDBLOCK value can be returned for non-blocking operations */
#define JERRYX_EWOULDBLOCK WSAEWOULDBLOCK

/* On Windows the invalid socket's value of INVALID_SOCKET */
#define JERRYX_SOCKET_INVALID INVALID_SOCKET

/*
 * On Windows, socket functions have the following signatures:
 * int send(SOCKET s, const char *buf, int len, int flags);
 * int recv(SOCKET s, char *buf, int len, int flags);
 * int setsockopt(SOCKET s, int level, int optname, const char *optval, int optlen);
 */
typedef int jerryx_socket_ssize_t;
typedef SOCKET jerryx_socket_t;
typedef char jerryx_socket_void_t;
typedef int jerryx_socket_size_t;
#else /* !_WIN32 */
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

/* On *nix the EWOULDBLOCK errno value can be returned for non-blocking operations */
#define JERRYX_EWOULDBLOCK    EWOULDBLOCK

/* On *nix the invalid socket has a value of -1 */
#define JERRYX_SOCKET_INVALID (-1)

/*
 * On *nix, socket functions have the following signatures:
 * ssize_t send(int sockfd, const void *buf, size_t len, int flags);
 * ssize_t recv(int sockfd, void *buf, size_t len, int flags);
 * int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
 */
typedef ssize_t jerryx_socket_ssize_t;
typedef int jerryx_socket_t;
typedef void jerryx_socket_void_t;
typedef size_t jerryx_socket_size_t;
#endif /* _WIN32 */

/**
 * Implementation of transport over tcp/ip.
 */
typedef struct
{
  jerry_debugger_transport_header_t header; /**< transport header */
  jerryx_socket_t tcp_socket; /**< tcp socket */
} jerryx_debugger_transport_tcp_t;

/**
 * Get the network error value.
 *
 * On Windows this returns the result of the `WSAGetLastError ()` call and
 * on any other system the `errno` value.
 *
 *
 * @return last error value.
 */
static inline int
jerryx_debugger_tcp_get_errno (void)
{
#ifdef _WIN32
  return WSAGetLastError ();
#else /* !_WIN32 */
  return errno;
#endif /* _WIN32 */
} /* jerryx_debugger_tcp_get_errno */

/**
 * Correctly close a single socket.
 */
static inline void
jerryx_debugger_tcp_close_socket (jerryx_socket_t socket_id) /**< socket to close */
{
#ifdef _WIN32
  closesocket (socket_id);
#else /* !_WIN32 */
  close (socket_id);
#endif /* _WIN32 */
} /* jerryx_debugger_tcp_close_socket */

/**
 * Log tcp error message.
 */
static void
jerryx_debugger_tcp_log_error (int errno_value) /**< error value to log */
{
  if (errno_value == 0)
  {
    return;
  }

#ifdef _WIN32
  char *error_message = NULL;
  FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL,
                 (DWORD) errno_value,
                 MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                 (LPTSTR) &error_message,
                 0,
                 NULL);
  JERRYX_ERROR_MSG ("TCP Error: %s\n", error_message);
  LocalFree (error_message);
#else /* !_WIN32 */
  JERRYX_ERROR_MSG ("TCP Error: %s\n", strerror (errno_value));
#endif /* _WIN32 */
} /* jerryx_debugger_tcp_log_error */

/**
 * Close a tcp connection.
 */
static void
jerryx_debugger_tcp_close (jerry_debugger_transport_header_t *header_p) /**< tcp implementation */
{
  JERRYX_ASSERT (!jerry_debugger_transport_is_connected ());

  jerryx_debugger_transport_tcp_t *tcp_p = (jerryx_debugger_transport_tcp_t *) header_p;

  JERRYX_DEBUG_MSG ("TCP connection closed.\n");

  jerryx_debugger_tcp_close_socket (tcp_p->tcp_socket);

  jerry_heap_free ((void *) header_p, sizeof (jerryx_debugger_transport_tcp_t));
} /* jerryx_debugger_tcp_close */

/**
 * Send data over a tcp connection.
 *
 * @return true - if the data has been sent successfully
 *         false - otherwise
 */
static bool
jerryx_debugger_tcp_send (jerry_debugger_transport_header_t *header_p, /**< tcp implementation */
                          uint8_t *message_p, /**< message to be sent */
                          size_t message_length) /**< message length in bytes */
{
  JERRYX_ASSERT (jerry_debugger_transport_is_connected ());

  jerryx_debugger_transport_tcp_t *tcp_p = (jerryx_debugger_transport_tcp_t *) header_p;
  jerryx_socket_size_t remaining_bytes = (jerryx_socket_size_t) message_length;

  do
  {
#ifdef __linux__
    ssize_t is_err = recv (tcp_p->tcp_socket, NULL, 0, MSG_PEEK);

    if (is_err == 0 && errno != JERRYX_EWOULDBLOCK)
    {
      int err_val = errno;
      jerry_debugger_transport_close ();
      jerryx_debugger_tcp_log_error (err_val);
      return false;
    }
#endif /* __linux__ */

    jerryx_socket_ssize_t sent_bytes = send (tcp_p->tcp_socket, (jerryx_socket_void_t *) message_p, remaining_bytes, 0);

    if (sent_bytes < 0)
    {
      int err_val = jerryx_debugger_tcp_get_errno ();

      if (err_val == JERRYX_EWOULDBLOCK)
      {
        continue;
      }

      jerry_debugger_transport_close ();
      jerryx_debugger_tcp_log_error (err_val);
      return false;
    }

    message_p += sent_bytes;
    remaining_bytes -= (jerryx_socket_size_t) sent_bytes;
  } while (remaining_bytes > 0);

  return true;
} /* jerryx_debugger_tcp_send */

/**
 * Receive data from a tcp connection.
 */
static bool
jerryx_debugger_tcp_receive (jerry_debugger_transport_header_t *header_p, /**< tcp implementation */
                             jerry_debugger_transport_receive_context_t *receive_context_p) /**< receive context */
{
  jerryx_debugger_transport_tcp_t *tcp_p = (jerryx_debugger_transport_tcp_t *) header_p;

  jerryx_socket_void_t *buffer_p =
    (jerryx_socket_void_t *) (receive_context_p->buffer_p + receive_context_p->received_length);
  jerryx_socket_size_t buffer_size =
    (jerryx_socket_size_t) (JERRY_DEBUGGER_TRANSPORT_MAX_BUFFER_SIZE - receive_context_p->received_length);

  jerryx_socket_ssize_t length = recv (tcp_p->tcp_socket, buffer_p, buffer_size, 0);

  if (length <= 0)
  {
    int err_val = jerryx_debugger_tcp_get_errno ();

    if (err_val != JERRYX_EWOULDBLOCK || length == 0)
    {
      jerry_debugger_transport_close ();
      jerryx_debugger_tcp_log_error (err_val);
      return false;
    }
    length = 0;
  }

  receive_context_p->received_length += (size_t) length;

  if (receive_context_p->received_length > 0)
  {
    receive_context_p->message_p = receive_context_p->buffer_p;
    receive_context_p->message_length = receive_context_p->received_length;
  }

  return true;
} /* jerryx_debugger_tcp_receive */

/**
 * Utility method to prepare the server socket to accept connections.
 *
 * The following steps are performed:
 *  * Configure address re-use.
 *  * Bind the socket to the given port
 *  * Start listening on the socket.
 *
 * @return true if everything is ok
 *         false if there was an error
 */
static bool
jerryx_debugger_tcp_configure_socket (jerryx_socket_t server_socket, /** < socket to configure */
                                      uint16_t port) /** < port number to be used for the socket */
{
  struct sockaddr_in addr;

  addr.sin_family = AF_INET;
  addr.sin_port = htons (port);
  addr.sin_addr.s_addr = INADDR_ANY;

  const int opt_value = 1;

  if (setsockopt (server_socket, SOL_SOCKET, SO_REUSEADDR, (const jerryx_socket_void_t *) &opt_value, sizeof (int))
      != 0)
  {
    return false;
  }

  if (bind (server_socket, (struct sockaddr *) &addr, sizeof (struct sockaddr_in)) != 0)
  {
    return false;
  }

  if (listen (server_socket, 1) != 0)
  {
    return false;
  }

  return true;
} /* jerryx_debugger_tcp_configure_socket */

/**
 * Create a tcp connection.
 *
 * @return true if successful,
 *         false otherwise
 */
bool
jerryx_debugger_tcp_create (uint16_t port) /**< listening port */
{
#ifdef _WIN32
  WSADATA wsaData;
  int wsa_init_status = WSAStartup (MAKEWORD (2, 2), &wsaData);
  if (wsa_init_status != NO_ERROR)
  {
    JERRYX_ERROR_MSG ("WSA Error: %d\n", wsa_init_status);
    return false;
  }
#endif /* _WIN32*/

  jerryx_socket_t server_socket = socket (AF_INET, SOCK_STREAM, 0);
  if (server_socket == JERRYX_SOCKET_INVALID)
  {
    jerryx_debugger_tcp_log_error (jerryx_debugger_tcp_get_errno ());
    return false;
  }

  if (!jerryx_debugger_tcp_configure_socket (server_socket, port))
  {
    int error = jerryx_debugger_tcp_get_errno ();
    jerryx_debugger_tcp_close_socket (server_socket);
    jerryx_debugger_tcp_log_error (error);
    return false;
  }

  JERRYX_DEBUG_MSG ("Waiting for client connection\n");

  struct sockaddr_in addr;
  socklen_t sin_size = sizeof (struct sockaddr_in);

  jerryx_socket_t tcp_socket = accept (server_socket, (struct sockaddr *) &addr, &sin_size);

  jerryx_debugger_tcp_close_socket (server_socket);

  if (tcp_socket == JERRYX_SOCKET_INVALID)
  {
    jerryx_debugger_tcp_log_error (jerryx_debugger_tcp_get_errno ());
    return false;
  }

  /* Set non-blocking mode. */
#ifdef _WIN32
  u_long nonblocking_enabled = 1;
  if (ioctlsocket (tcp_socket, (long) FIONBIO, &nonblocking_enabled) != NO_ERROR)
  {
    jerryx_debugger_tcp_close_socket (tcp_socket);
    return false;
  }
#else /* !_WIN32 */
  int socket_flags = fcntl (tcp_socket, F_GETFL, 0);

  if (socket_flags < 0)
  {
    close (tcp_socket);
    return false;
  }

  if (fcntl (tcp_socket, F_SETFL, socket_flags | O_NONBLOCK) == -1)
  {
    close (tcp_socket);
    return false;
  }
#endif /* _WIN32 */

  JERRYX_DEBUG_MSG ("Connected from: %s\n", inet_ntoa (addr.sin_addr));

  jerry_size_t size = sizeof (jerryx_debugger_transport_tcp_t);

  jerry_debugger_transport_header_t *header_p;
  header_p = (jerry_debugger_transport_header_t *) jerry_heap_alloc (size);

  if (!header_p)
  {
    jerryx_debugger_tcp_close_socket (tcp_socket);
    return false;
  }

  header_p->close = jerryx_debugger_tcp_close;
  header_p->send = jerryx_debugger_tcp_send;
  header_p->receive = jerryx_debugger_tcp_receive;

  ((jerryx_debugger_transport_tcp_t *) header_p)->tcp_socket = tcp_socket;

  jerry_debugger_transport_add (header_p,
                                0,
                                JERRY_DEBUGGER_TRANSPORT_MAX_BUFFER_SIZE,
                                0,
                                JERRY_DEBUGGER_TRANSPORT_MAX_BUFFER_SIZE);

  return true;
} /* jerryx_debugger_tcp_create */

#else /* !(defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)) */

/**
 * Dummy function when debugger is disabled.
 *
 * @return false
 */
bool
jerryx_debugger_tcp_create (uint16_t port)
{
  JERRYX_UNUSED (port);
  return false;
} /* jerryx_debugger_tcp_create */

#endif /* defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1) */

/*
 *  FIPS-180-1 compliant SHA-1 implementation
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

/*
 *  The SHA-1 standard was published by NIST in 1993.
 *
 *  http://www.itl.nist.gov/fipspubs/fip180-1.htm
 */



#if defined(JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)

/**
 * SHA-1 context structure.
 */
typedef struct
{
  uint32_t total[2]; /**< number of bytes processed */
  uint32_t state[5]; /**< intermediate digest state */
  uint8_t buffer[64]; /**< data block being processed */
} jerryx_sha1_context;

/* 32-bit integer manipulation macros (big endian). */

#define JERRYX_SHA1_GET_UINT32_BE(n, b, i)                                                                         \
  {                                                                                                                \
    (n) = (((uint32_t) (b)[(i) + 0]) << 24) | (((uint32_t) (b)[(i) + 1]) << 16) | (((uint32_t) (b)[(i) + 2]) << 8) \
          | ((uint32_t) (b)[(i) + 3]);                                                                             \
  }

#define JERRYX_SHA1_PUT_UINT32_BE(n, b, i) \
  {                                        \
    (b)[(i) + 0] = (uint8_t) ((n) >> 24);  \
    (b)[(i) + 1] = (uint8_t) ((n) >> 16);  \
    (b)[(i) + 2] = (uint8_t) ((n) >> 8);   \
    (b)[(i) + 3] = (uint8_t) ((n));        \
  }

/**
 * Initialize SHA-1 context.
 */
static void
jerryx_sha1_init (jerryx_sha1_context *sha1_context_p) /**< SHA-1 context */
{
  memset (sha1_context_p, 0, sizeof (jerryx_sha1_context));

  sha1_context_p->total[0] = 0;
  sha1_context_p->total[1] = 0;

  sha1_context_p->state[0] = 0x67452301;
  sha1_context_p->state[1] = 0xEFCDAB89;
  sha1_context_p->state[2] = 0x98BADCFE;
  sha1_context_p->state[3] = 0x10325476;
  sha1_context_p->state[4] = 0xC3D2E1F0;
} /* jerryx_sha1_init */

#define JERRYX_SHA1_P(a, b, c, d, e, x)                              \
  do                                                                 \
  {                                                                  \
    e += JERRYX_SHA1_SHIFT (a, 5) + JERRYX_SHA1_F (b, c, d) + K + x; \
    b = JERRYX_SHA1_SHIFT (b, 30);                                   \
  } while (0)

/**
 * Update SHA-1 internal buffer status.
 */
static void
jerryx_sha1_process (jerryx_sha1_context *sha1_context_p, /**< SHA-1 context */
                     const uint8_t data[64]) /**< data buffer */
{
  uint32_t temp, W[16], A, B, C, D, E;

  JERRYX_SHA1_GET_UINT32_BE (W[0], data, 0);
  JERRYX_SHA1_GET_UINT32_BE (W[1], data, 4);
  JERRYX_SHA1_GET_UINT32_BE (W[2], data, 8);
  JERRYX_SHA1_GET_UINT32_BE (W[3], data, 12);
  JERRYX_SHA1_GET_UINT32_BE (W[4], data, 16);
  JERRYX_SHA1_GET_UINT32_BE (W[5], data, 20);
  JERRYX_SHA1_GET_UINT32_BE (W[6], data, 24);
  JERRYX_SHA1_GET_UINT32_BE (W[7], data, 28);
  JERRYX_SHA1_GET_UINT32_BE (W[8], data, 32);
  JERRYX_SHA1_GET_UINT32_BE (W[9], data, 36);
  JERRYX_SHA1_GET_UINT32_BE (W[10], data, 40);
  JERRYX_SHA1_GET_UINT32_BE (W[11], data, 44);
  JERRYX_SHA1_GET_UINT32_BE (W[12], data, 48);
  JERRYX_SHA1_GET_UINT32_BE (W[13], data, 52);
  JERRYX_SHA1_GET_UINT32_BE (W[14], data, 56);
  JERRYX_SHA1_GET_UINT32_BE (W[15], data, 60);

#define JERRYX_SHA1_SHIFT(x, n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define JERRYX_SHA1_R(t)                                                            \
  (temp = W[(t - 3) & 0x0F] ^ W[(t - 8) & 0x0F] ^ W[(t - 14) & 0x0F] ^ W[t & 0x0F], \
   W[t & 0x0F] = JERRYX_SHA1_SHIFT (temp, 1))

  A = sha1_context_p->state[0];
  B = sha1_context_p->state[1];
  C = sha1_context_p->state[2];
  D = sha1_context_p->state[3];
  E = sha1_context_p->state[4];

  uint32_t K = 0x5A827999;

#define JERRYX_SHA1_F(x, y, z) (z ^ (x & (y ^ z)))

  JERRYX_SHA1_P (A, B, C, D, E, W[0]);
  JERRYX_SHA1_P (E, A, B, C, D, W[1]);
  JERRYX_SHA1_P (D, E, A, B, C, W[2]);
  JERRYX_SHA1_P (C, D, E, A, B, W[3]);
  JERRYX_SHA1_P (B, C, D, E, A, W[4]);
  JERRYX_SHA1_P (A, B, C, D, E, W[5]);
  JERRYX_SHA1_P (E, A, B, C, D, W[6]);
  JERRYX_SHA1_P (D, E, A, B, C, W[7]);
  JERRYX_SHA1_P (C, D, E, A, B, W[8]);
  JERRYX_SHA1_P (B, C, D, E, A, W[9]);
  JERRYX_SHA1_P (A, B, C, D, E, W[10]);
  JERRYX_SHA1_P (E, A, B, C, D, W[11]);
  JERRYX_SHA1_P (D, E, A, B, C, W[12]);
  JERRYX_SHA1_P (C, D, E, A, B, W[13]);
  JERRYX_SHA1_P (B, C, D, E, A, W[14]);
  JERRYX_SHA1_P (A, B, C, D, E, W[15]);
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (16));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (17));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (18));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (19));

#undef JERRYX_SHA1_F

  K = 0x6ED9EBA1;

#define JERRYX_SHA1_F(x, y, z) (x ^ y ^ z)

  JERRYX_SHA1_P (A, B, C, D, E, JERRYX_SHA1_R (20));
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (21));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (22));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (23));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (24));
  JERRYX_SHA1_P (A, B, C, D, E, JERRYX_SHA1_R (25));
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (26));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (27));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (28));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (29));
  JERRYX_SHA1_P (A, B, C, D, E, JERRYX_SHA1_R (30));
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (31));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (32));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (33));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (34));
  JERRYX_SHA1_P (A, B, C, D, E, JERRYX_SHA1_R (35));
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (36));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (37));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (38));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (39));

#undef JERRYX_SHA1_F

  K = 0x8F1BBCDC;

#define JERRYX_SHA1_F(x, y, z) ((x & y) | (z & (x | y)))

  JERRYX_SHA1_P (A, B, C, D, E, JERRYX_SHA1_R (40));
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (41));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (42));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (43));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (44));
  JERRYX_SHA1_P (A, B, C, D, E, JERRYX_SHA1_R (45));
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (46));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (47));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (48));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (49));
  JERRYX_SHA1_P (A, B, C, D, E, JERRYX_SHA1_R (50));
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (51));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (52));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (53));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (54));
  JERRYX_SHA1_P (A, B, C, D, E, JERRYX_SHA1_R (55));
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (56));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (57));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (58));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (59));

#undef JERRYX_SHA1_F

  K = 0xCA62C1D6;

#define JERRYX_SHA1_F(x, y, z) (x ^ y ^ z)

  JERRYX_SHA1_P (A, B, C, D, E, JERRYX_SHA1_R (60));
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (61));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (62));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (63));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (64));
  JERRYX_SHA1_P (A, B, C, D, E, JERRYX_SHA1_R (65));
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (66));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (67));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (68));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (69));
  JERRYX_SHA1_P (A, B, C, D, E, JERRYX_SHA1_R (70));
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (71));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (72));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (73));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (74));
  JERRYX_SHA1_P (A, B, C, D, E, JERRYX_SHA1_R (75));
  JERRYX_SHA1_P (E, A, B, C, D, JERRYX_SHA1_R (76));
  JERRYX_SHA1_P (D, E, A, B, C, JERRYX_SHA1_R (77));
  JERRYX_SHA1_P (C, D, E, A, B, JERRYX_SHA1_R (78));
  JERRYX_SHA1_P (B, C, D, E, A, JERRYX_SHA1_R (79));

#undef JERRYX_SHA1_F

  sha1_context_p->state[0] += A;
  sha1_context_p->state[1] += B;
  sha1_context_p->state[2] += C;
  sha1_context_p->state[3] += D;
  sha1_context_p->state[4] += E;

#undef JERRYX_SHA1_SHIFT
#undef JERRYX_SHA1_R
} /* jerryx_sha1_process */

#undef JERRYX_SHA1_P

/**
 * SHA-1 update buffer.
 */
static void
jerryx_sha1_update (jerryx_sha1_context *sha1_context_p, /**< SHA-1 context */
                    const uint8_t *source_p, /**< source buffer */
                    size_t source_length) /**< length of source buffer */
{
  size_t fill;
  uint32_t left;

  if (source_length == 0)
  {
    return;
  }

  left = sha1_context_p->total[0] & 0x3F;
  fill = 64 - left;

  sha1_context_p->total[0] += (uint32_t) source_length;

  /* Check overflow. */
  if (sha1_context_p->total[0] < (uint32_t) source_length)
  {
    sha1_context_p->total[1]++;
  }

  if (left && source_length >= fill)
  {
    memcpy ((void *) (sha1_context_p->buffer + left), source_p, fill);
    jerryx_sha1_process (sha1_context_p, sha1_context_p->buffer);
    source_p += fill;
    source_length -= fill;
    left = 0;
  }

  while (source_length >= 64)
  {
    jerryx_sha1_process (sha1_context_p, source_p);
    source_p += 64;
    source_length -= 64;
  }

  if (source_length > 0)
  {
    memcpy ((void *) (sha1_context_p->buffer + left), source_p, source_length);
  }
} /* jerryx_sha1_update */

/**
 * SHA-1 final digest.
 */
static void
jerryx_sha1_finish (jerryx_sha1_context *sha1_context_p, /**< SHA-1 context */
                    uint8_t destination_p[20]) /**< result */
{
  uint8_t buffer[16];

  uint32_t high = (sha1_context_p->total[0] >> 29) | (sha1_context_p->total[1] << 3);
  uint32_t low = (sha1_context_p->total[0] << 3);

  uint32_t last = sha1_context_p->total[0] & 0x3F;
  uint32_t padn = (last < 56) ? (56 - last) : (120 - last);

  memset (buffer, 0, sizeof (buffer));
  buffer[0] = 0x80;

  while (padn > sizeof (buffer))
  {
    jerryx_sha1_update (sha1_context_p, buffer, sizeof (buffer));
    buffer[0] = 0;
    padn -= (uint32_t) sizeof (buffer);
  }

  jerryx_sha1_update (sha1_context_p, buffer, padn);

  JERRYX_SHA1_PUT_UINT32_BE (high, buffer, 0);
  JERRYX_SHA1_PUT_UINT32_BE (low, buffer, 4);

  jerryx_sha1_update (sha1_context_p, buffer, 8);

  JERRYX_SHA1_PUT_UINT32_BE (sha1_context_p->state[0], destination_p, 0);
  JERRYX_SHA1_PUT_UINT32_BE (sha1_context_p->state[1], destination_p, 4);
  JERRYX_SHA1_PUT_UINT32_BE (sha1_context_p->state[2], destination_p, 8);
  JERRYX_SHA1_PUT_UINT32_BE (sha1_context_p->state[3], destination_p, 12);
  JERRYX_SHA1_PUT_UINT32_BE (sha1_context_p->state[4], destination_p, 16);
} /* jerryx_sha1_finish */

#undef JERRYX_SHA1_GET_UINT32_BE
#undef JERRYX_SHA1_PUT_UINT32_BE

/**
 * Computes the SHA-1 value of the combination of the two input buffers.
 */
void
jerryx_debugger_compute_sha1 (const uint8_t *source1_p, /**< first part of the input */
                              size_t source1_length, /**< length of the first part */
                              const uint8_t *source2_p, /**< second part of the input */
                              size_t source2_length, /**< length of the second part */
                              uint8_t destination_p[20]) /**< result */
{
  jerryx_sha1_context sha1_context;

  jerryx_sha1_init (&sha1_context);
  jerryx_sha1_update (&sha1_context, source1_p, source1_length);
  jerryx_sha1_update (&sha1_context, source2_p, source2_length);
  jerryx_sha1_finish (&sha1_context, destination_p);
} /* jerryx_debugger_compute_sha1 */

#endif /* defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1) */



#if (defined(JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)) && !defined _WIN32

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/* Max size of configuration string */
#define CONFIG_SIZE (255)

/**
 * Implementation of transport over serial connection.
 */
typedef struct
{
  jerry_debugger_transport_header_t header; /**< transport header */
  int fd; /**< file descriptor */
} jerryx_debugger_transport_serial_t;

/**
 * Configure parameters for a serial port.
 */
typedef struct
{
  char *device_id;
  uint32_t baud_rate; /**< specify the rate at which bits are transmitted for the serial interface */
  uint32_t data_bits; /**< specify the number of data bits to transmit over the serial interface */
  char parity; /**< specify how you want to check parity bits in the data bits transmitted via the serial port */
  uint32_t stop_bits; /**< specify the number of bits used to indicate the end of a byte. */
} jerryx_debugger_transport_serial_config_t;

/**
 * Correctly close a file descriptor.
 */
static inline void
jerryx_debugger_serial_close_fd (int fd) /**< file descriptor to close */
{
  if (close (fd) != 0)
  {
    JERRYX_ERROR_MSG ("Error while closing the file descriptor: %d\n", errno);
  }
} /* jerryx_debugger_serial_close_fd */

/**
 * Set a file descriptor to blocking or non-blocking mode.
 *
 * @return true if everything is ok
 *         false if there was an error
 **/
static bool
jerryx_debugger_serial_set_blocking (int fd, bool blocking)
{
  /* Save the current flags */
  int flags = fcntl (fd, F_GETFL, 0);
  if (flags == -1)
  {
    JERRYX_ERROR_MSG ("Error %d during get flags from file descriptor\n", errno);
    return false;
  }

  if (blocking)
  {
    flags &= ~O_NONBLOCK;
  }
  else
  {
    flags |= O_NONBLOCK;
  }

  if (fcntl (fd, F_SETFL, flags) == -1)
  {
    JERRYX_ERROR_MSG ("Error %d during set flags from file descriptor\n", errno);
    return false;
  }

  return true;
} /* jerryx_debugger_serial_set_blocking */

/**
 * Configure the file descriptor used by the serial communcation.
 *
 * @return true if everything is ok
 *         false if there was an error
 */
static inline bool
jerryx_debugger_serial_configure_attributes (int fd, jerryx_debugger_transport_serial_config_t serial_config)
{
  struct termios options;
  memset (&options, 0, sizeof (options));

  /* Get the parameters associated with the file descriptor */
  if (tcgetattr (fd, &options) != 0)
  {
    JERRYX_ERROR_MSG ("Error %d from tggetattr\n", errno);
    return false;
  }

  /* Set the input and output baud rates */
  cfsetispeed (&options, serial_config.baud_rate);
  cfsetospeed (&options, serial_config.baud_rate);

  /* Set the control modes */
  options.c_cflag &= (uint32_t) ~CSIZE; // character size mask
  options.c_cflag |= (CLOCAL | CREAD); // ignore modem control lines and enable the receiver

  switch (serial_config.data_bits)
  {
    case 5:
    {
      options.c_cflag |= CS5; // set character size mask to 5-bit chars
      break;
    }
    case 6:
    {
      options.c_cflag |= CS6; // set character size mask to 6-bit chars
      break;
    }
    case 7:
    {
      options.c_cflag |= CS7; // set character size mask to 7-bit chars
      break;
    }
    case 8:
    {
      options.c_cflag |= CS8; // set character size mask to 8-bit chars
      break;
    }
    default:
    {
      JERRYX_ERROR_MSG ("Unsupported data bits: %d\n", serial_config.data_bits);
      return false;
    }
  }

  switch (serial_config.parity)
  {
    case 'N':
    {
      options.c_cflag &= (unsigned int) ~(PARENB | PARODD);
      break;
    }
    case 'O':
    {
      options.c_cflag |= PARENB;
      options.c_cflag |= PARODD;
      break;
    }
    case 'E':
    {
      options.c_cflag |= PARENB;
      options.c_cflag |= PARODD;
      break;
    }
    default:
    {
      JERRYX_ERROR_MSG ("Unsupported parity: %c\n", serial_config.parity);
      return false;
    }
  }

  switch (serial_config.stop_bits)
  {
    case 1:
    {
      options.c_cflag &= (uint32_t) ~CSTOPB; // set 1 stop bits
      break;
    }
    case 2:
    {
      options.c_cflag |= CSTOPB; // set 2 stop bits
      break;
    }
    default:
    {
      JERRYX_ERROR_MSG ("Unsupported stop bits: %d\n", serial_config.stop_bits);
      return false;
    }
  }

  /* Set the input modes */
  options.c_iflag &= (uint32_t) ~IGNBRK; // disable break processing
  options.c_iflag &= (uint32_t) ~(IXON | IXOFF | IXANY); // disable xon/xoff ctrl

  /* Set the output modes: no remapping, no delays */
  options.c_oflag = 0;

  /* Set the local modes: no signaling chars, no echo, no canoncial processing */
  options.c_lflag = 0;

  /* Read returns when at least one byte of data is available. */
  options.c_cc[VMIN] = 1; // read block
  options.c_cc[VTIME] = 5; // 0.5 seconds read timeout

  /* Set the parameters associated with the file descriptor */
  if (tcsetattr (fd, TCSANOW, &options) != 0)
  {
    JERRYX_ERROR_MSG ("Error %d from tcsetattr", errno);
    return false;
  }

  /* Flushes both data received but not read, and data written but not transmitted */
  if (tcflush (fd, TCIOFLUSH) != 0)
  {
    JERRYX_ERROR_MSG ("Error %d in tcflush() :%s\n", errno, strerror (errno));
    jerryx_debugger_serial_close_fd (fd);
    return false;
  }

  return true;
} /* jerryx_debugger_serial_configure_attributes */

/**
 * Close a serial connection.
 */
static void
jerryx_debugger_serial_close (jerry_debugger_transport_header_t *header_p) /**< serial implementation */
{
  JERRYX_ASSERT (!jerry_debugger_transport_is_connected ());

  jerryx_debugger_transport_serial_t *serial_p = (jerryx_debugger_transport_serial_t *) header_p;

  JERRYX_DEBUG_MSG ("Serial connection closed.\n");

  jerryx_debugger_serial_close_fd (serial_p->fd);

  jerry_heap_free ((void *) header_p, sizeof (jerryx_debugger_transport_serial_t));
} /* jerryx_debugger_serial_close */

/**
 * Send data over a serial connection.
 *
 * @return true - if the data has been sent successfully
 *         false - otherwise
 */
static bool
jerryx_debugger_serial_send (jerry_debugger_transport_header_t *header_p, /**< serial implementation */
                             uint8_t *message_p, /**< message to be sent */
                             size_t message_length) /**< message length in bytes */
{
  JERRYX_ASSERT (jerry_debugger_transport_is_connected ());

  jerryx_debugger_transport_serial_t *serial_p = (jerryx_debugger_transport_serial_t *) header_p;

  do
  {
    ssize_t sent_bytes = write (serial_p->fd, message_p, message_length);

    if (sent_bytes < 0)
    {
      if (errno == EWOULDBLOCK)
      {
        continue;
      }

      JERRYX_ERROR_MSG ("Error: write to file descriptor: %d\n", errno);
      jerry_debugger_transport_close ();
      return false;
    }

    message_p += sent_bytes;
    message_length -= (size_t) sent_bytes;
  } while (message_length > 0);

  return true;
} /* jerryx_debugger_serial_send */

/**
 * Receive data from a serial connection.
 */
static bool
jerryx_debugger_serial_receive (jerry_debugger_transport_header_t *header_p, /**< serial implementation */
                                jerry_debugger_transport_receive_context_t *receive_context_p) /**< receive context */
{
  jerryx_debugger_transport_serial_t *serial_p = (jerryx_debugger_transport_serial_t *) header_p;

  uint8_t *buffer_p = receive_context_p->buffer_p + receive_context_p->received_length;
  size_t buffer_size = JERRY_DEBUGGER_TRANSPORT_MAX_BUFFER_SIZE - receive_context_p->received_length;

  ssize_t length = read (serial_p->fd, buffer_p, buffer_size);

  if (length <= 0)
  {
    if (errno != EWOULDBLOCK || length == 0)
    {
      jerry_debugger_transport_close ();
      return false;
    }
    length = 0;
  }

  receive_context_p->received_length += (size_t) length;

  if (receive_context_p->received_length > 0)
  {
    receive_context_p->message_p = receive_context_p->buffer_p;
    receive_context_p->message_length = receive_context_p->received_length;
  }

  return true;
} /* jerryx_debugger_serial_receive */

/**
 * Create a serial connection.
 *
 * @return true if successful,
 *         false otherwise
 */
bool
jerryx_debugger_serial_create (const char *config) /**< specify the configuration */
{
  /* Parse the configuration string */
  char tmp_config[CONFIG_SIZE];
  strncpy (tmp_config, config, CONFIG_SIZE);
  jerryx_debugger_transport_serial_config_t serial_config;

  char *token = strtok (tmp_config, ",");
  serial_config.device_id = token ? token : "/dev/ttyS0";
  serial_config.baud_rate = (token = strtok (NULL, ",")) ? (uint32_t) strtoul (token, NULL, 10) : 115200;
  serial_config.data_bits = (token = strtok (NULL, ",")) ? (uint32_t) strtoul (token, NULL, 10) : 8;
  serial_config.parity = (token = strtok (NULL, ",")) ? token[0] : 'N';
  serial_config.stop_bits = (token = strtok (NULL, ",")) ? (uint32_t) strtoul (token, NULL, 10) : 1;

  int fd = open (serial_config.device_id, O_RDWR);

  if (fd < 0)
  {
    JERRYX_ERROR_MSG ("Error %d opening %s: %s", errno, serial_config.device_id, strerror (errno));
    return false;
  }

  if (!jerryx_debugger_serial_configure_attributes (fd, serial_config))
  {
    jerryx_debugger_serial_close_fd (fd);
    return false;
  }

  JERRYX_DEBUG_MSG ("Waiting for client connection\n");

  /* Client will sent a 'c' char to initiate the connection. */
  uint8_t conn_char;
  ssize_t t = read (fd, &conn_char, 1);
  if (t != 1 || conn_char != 'c' || !jerryx_debugger_serial_set_blocking (fd, false))
  {
    return false;
  }

  JERRYX_DEBUG_MSG ("Client connected\n");

  jerry_size_t size = sizeof (jerryx_debugger_transport_serial_t);

  jerry_debugger_transport_header_t *header_p;
  header_p = (jerry_debugger_transport_header_t *) jerry_heap_alloc (size);

  if (!header_p)
  {
    jerryx_debugger_serial_close_fd (fd);
    return false;
  }

  header_p->close = jerryx_debugger_serial_close;
  header_p->send = jerryx_debugger_serial_send;
  header_p->receive = jerryx_debugger_serial_receive;

  ((jerryx_debugger_transport_serial_t *) header_p)->fd = fd;

  jerry_debugger_transport_add (header_p,
                                0,
                                JERRY_DEBUGGER_TRANSPORT_MAX_BUFFER_SIZE,
                                0,
                                JERRY_DEBUGGER_TRANSPORT_MAX_BUFFER_SIZE);

  return true;
} /* jerryx_debugger_serial_create */

#else /* !(defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)) || _WIN32 */
/**
 * Dummy function when debugger is disabled.
 *
 * @return false
 */
bool
jerryx_debugger_serial_create (const char *config)
{
  JERRYX_UNUSED (config);
  return false;
} /* jerryx_debugger_serial_create */

#endif /* (defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)) && !defined _WIN32 */


#if defined(JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)

/* A simplified transmission layer. */

/**
 * Size of the raw packet header.
 */
#define JERRYX_DEBUGGER_RAWPACKET_HEADER_SIZE 1
/**
 * Maximum message size with 1 byte size field.
 */
#define JERRYX_DEBUGGER_RAWPACKET_ONE_BYTE_LEN_MAX 255

/**
 * Header for incoming packets.
 */
typedef struct
{
  uint8_t size; /**< size of the message */
} jerryx_rawpacket_receive_header_t;

/**
 * Close a tcp connection.
 */
static void
jerryx_debugger_rp_close (jerry_debugger_transport_header_t *header_p) /**< header for the transport interface */
{
  JERRYX_ASSERT (!jerry_debugger_transport_is_connected ());

  jerry_heap_free ((void *) header_p, sizeof (jerry_debugger_transport_header_t));
} /* jerryx_debugger_rp_close */

/**
 * Send data over a simple raw packet connection.
 *
 * @return true - if the data has been sent successfully
 *         false - otherwise
 */
static bool
jerryx_debugger_rp_send (jerry_debugger_transport_header_t *header_p, /**< header for the transport interface */
                         uint8_t *message_p, /**< message to be sent */
                         size_t message_length) /**< message length in bytes */
{
  JERRYX_ASSERT (message_length <= JERRYX_DEBUGGER_RAWPACKET_ONE_BYTE_LEN_MAX);

  message_p[-1] = (uint8_t) message_length;

  return header_p->next_p->send (header_p->next_p, message_p - 1, message_length + 1);
} /* jerryx_debugger_rp_send */

/**
 * Receive data from a rawpacket connection.
 *
 * @return true - if data has been received successfully
 *         false - otherwise
 */
static bool
jerryx_debugger_rp_receive (jerry_debugger_transport_header_t *header_p, /**< header for the transport interface */
                            jerry_debugger_transport_receive_context_t *receive_context_p) /**< receive context */
{
  if (!header_p->next_p->receive (header_p->next_p, receive_context_p))
  {
    return false;
  }

  if (receive_context_p->message_p == NULL)
  {
    return true;
  }

  size_t message_total_length = receive_context_p->message_total_length;

  if (message_total_length == 0)
  {
    /* Byte stream. */
    if (receive_context_p->message_length < sizeof (jerryx_rawpacket_receive_header_t))
    {
      receive_context_p->message_p = NULL;
      return true;
    }
  }
  else
  {
    /* Datagram packet. */
    JERRYX_ASSERT (receive_context_p->message_length >= sizeof (jerryx_rawpacket_receive_header_t));
  }

  uint8_t *message_p = receive_context_p->message_p;
  size_t message_length = (size_t) (message_p[0]);

  if (message_total_length == 0)
  {
    size_t new_total_length = message_length + sizeof (jerryx_rawpacket_receive_header_t);

    /* Byte stream. */
    if (receive_context_p->message_length < new_total_length)
    {
      receive_context_p->message_p = NULL;
      return true;
    }

    receive_context_p->message_total_length = new_total_length;
  }
  else
  {
    /* Datagram packet. */
    JERRYX_ASSERT (receive_context_p->message_length == (message_length + sizeof (jerryx_rawpacket_receive_header_t)));
  }

  receive_context_p->message_p = message_p + sizeof (jerryx_rawpacket_receive_header_t);
  receive_context_p->message_length = message_length;

  return true;
} /* jerryx_debugger_rp_receive */

/**
 * Initialize a simple raw packet transmission layer.
 *
 * @return true - if the connection succeeded
 *         false - otherwise
 */
bool
jerryx_debugger_rp_create (void)
{
  const jerry_size_t interface_size = sizeof (jerry_debugger_transport_header_t);
  jerry_debugger_transport_header_t *header_p;
  header_p = (jerry_debugger_transport_header_t *) jerry_heap_alloc (interface_size);

  if (!header_p)
  {
    return false;
  }

  header_p->close = jerryx_debugger_rp_close;
  header_p->send = jerryx_debugger_rp_send;
  header_p->receive = jerryx_debugger_rp_receive;

  jerry_debugger_transport_add (header_p,
                                JERRYX_DEBUGGER_RAWPACKET_HEADER_SIZE,
                                JERRYX_DEBUGGER_RAWPACKET_ONE_BYTE_LEN_MAX,
                                JERRYX_DEBUGGER_RAWPACKET_HEADER_SIZE,
                                JERRYX_DEBUGGER_RAWPACKET_ONE_BYTE_LEN_MAX);

  return true;
} /* jerryx_debugger_rp_create */

#else /* !(defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)) */

/**
 * Dummy function when debugger is disabled.
 *
 * @return false
 */
bool
jerryx_debugger_rp_create (void)
{
  return false;
} /* jerryx_debugger_rp_create */

#endif /* defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1) */

#include <assert.h>


/**
 * Must be called after the connection has been initialized.
 */
void
jerryx_debugger_after_connect (bool success) /**< tells whether the connection
                                              *   has been successfully established */
{
#if defined(JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)
  if (success)
  {
    jerry_debugger_transport_start ();
  }
  else
  {
    jerry_debugger_transport_close ();
  }
#else /* !(defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1)) */
  JERRYX_UNUSED (success);
#endif /* defined (JERRY_DEBUGGER) && (JERRY_DEBUGGER == 1) */
} /* jerryx_debugger_after_connect */

/**
 * Check that value contains the reset abort value.
 *
 * Note: if the value is the reset abort value, the value is released.
 *
 * return true, if reset abort
 *        false, otherwise
 */
bool
jerryx_debugger_is_reset (jerry_value_t value) /**< jerry value */
{
  if (!jerry_value_is_abort (value))
  {
    return false;
  }

  jerry_value_t abort_value = jerry_exception_value (value, false);

  if (!jerry_value_is_string (abort_value))
  {
    jerry_value_free (abort_value);
    return false;
  }

  static const char restart_str[] = "r353t";

  jerry_size_t str_size = jerry_string_size (abort_value, JERRY_ENCODING_CESU8);
  bool is_reset = false;

  if (str_size == sizeof (restart_str) - 1)
  {
    JERRY_VLA (jerry_char_t, str_buf, str_size);
    jerry_string_to_buffer (abort_value, JERRY_ENCODING_CESU8, str_buf, str_size);

    is_reset = memcmp (restart_str, (char *) (str_buf), str_size) == 0;

    if (is_reset)
    {
      jerry_value_free (value);
    }
  }

  jerry_value_free (abort_value);
  return is_reset;
} /* jerryx_debugger_is_reset */
