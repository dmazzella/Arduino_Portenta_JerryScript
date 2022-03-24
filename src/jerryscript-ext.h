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

#ifndef JERRYX_ARG_H
#define JERRYX_ARG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


JERRY_C_API_BEGIN

/**
 * The forward declaration of jerryx_arg_t.
 */
typedef struct jerryx_arg_t jerryx_arg_t;

/**
 * The forward declaration of jerryx_arg_js_iterator_t
 */
typedef struct jerryx_arg_js_iterator_t jerryx_arg_js_iterator_t;

/**
 * Signature of the transform function.
 */
typedef jerry_value_t (*jerryx_arg_transform_func_t) (jerryx_arg_js_iterator_t *js_arg_iter_p, /**< available JS args */
                                                      const jerryx_arg_t *c_arg_p); /**< native arg */

/**
 * The structure used in jerryx_arg_object_properties
 */
typedef struct
{
  const jerry_char_t **name_p; /**< property name list of the JS object */
  jerry_length_t name_cnt; /**< count of the name list */
  const jerryx_arg_t *c_arg_p; /**< points to the array of transformation steps */
  jerry_length_t c_arg_cnt; /**< the count of the `c_arg_p` array */
} jerryx_arg_object_props_t;

/**
 * The structure used in jerryx_arg_array
 */
typedef struct
{
  const jerryx_arg_t *c_arg_p; /**< points to the array of transformation steps */
  jerry_length_t c_arg_cnt; /**< the count of the `c_arg_p` array */
} jerryx_arg_array_items_t;

/**
 * The structure defining a single validation & transformation step.
 */
struct jerryx_arg_t
{
  jerryx_arg_transform_func_t func; /**< the transform function */
  void *dest; /**< pointer to destination where func should store the result */
  uintptr_t extra_info; /**< extra information, specific to func */
};

jerry_value_t jerryx_arg_transform_this_and_args (const jerry_value_t this_val,
                                                  const jerry_value_t *js_arg_p,
                                                  const jerry_length_t js_arg_cnt,
                                                  const jerryx_arg_t *c_arg_p,
                                                  jerry_length_t c_arg_cnt);

jerry_value_t jerryx_arg_transform_args (const jerry_value_t *js_arg_p,
                                         const jerry_length_t js_arg_cnt,
                                         const jerryx_arg_t *c_arg_p,
                                         jerry_length_t c_arg_cnt);

jerry_value_t jerryx_arg_transform_object_properties (const jerry_value_t obj_val,
                                                      const jerry_char_t **name_p,
                                                      const jerry_length_t name_cnt,
                                                      const jerryx_arg_t *c_arg_p,
                                                      jerry_length_t c_arg_cnt);
jerry_value_t
jerryx_arg_transform_array (const jerry_value_t array_val, const jerryx_arg_t *c_arg_p, jerry_length_t c_arg_cnt);

/**
 * Indicates whether an argument is allowed to be coerced into the expected JS type.
 */
typedef enum
{
  JERRYX_ARG_COERCE, /**< the transform inside will invoke toNumber, toBoolean or toString */
  JERRYX_ARG_NO_COERCE /**< the type coercion is not allowed. */
} jerryx_arg_coerce_t;

/**
 * Indicates whether an argument is optional or required.
 */
typedef enum
{
  /**
   * The argument is optional. If the argument is `undefined` the transform is
   * successful and `c_arg_p->dest` remains untouched.
   */
  JERRYX_ARG_OPTIONAL,
  /**
   * The argument is required. If the argument is `undefined` the transform
   * will fail and `c_arg_p->dest` remains untouched.
   */
  JERRYX_ARG_REQUIRED
} jerryx_arg_optional_t;

/**
 * Indicates the rounding policy which will be chosen to transform an integer.
 */
typedef enum
{
  JERRYX_ARG_ROUND, /**< round */
  JERRYX_ARG_FLOOR, /**< floor */
  JERRYX_ARG_CEIL /**< ceil */
} jerryx_arg_round_t;

/**
 * Indicates the clamping policy which will be chosen to transform an integer.
 * If the policy is NO_CLAMP, and the number is out of range,
 * then the transformer will throw a range error.
 */
typedef enum
{
  JERRYX_ARG_CLAMP, /**< clamp the number when it is out of range */
  JERRYX_ARG_NO_CLAMP /**< throw a range error */
} jerryx_arg_clamp_t;

/* Inline functions for initializing jerryx_arg_t */

#define JERRYX_ARG_INTEGER(type)                                                 \
  static inline jerryx_arg_t jerryx_arg_##type (type##_t *dest,                  \
                                                jerryx_arg_round_t round_flag,   \
                                                jerryx_arg_clamp_t clamp_flag,   \
                                                jerryx_arg_coerce_t coerce_flag, \
                                                jerryx_arg_optional_t opt_flag);

JERRYX_ARG_INTEGER (uint8)
JERRYX_ARG_INTEGER (int8)
JERRYX_ARG_INTEGER (uint16)
JERRYX_ARG_INTEGER (int16)
JERRYX_ARG_INTEGER (uint32)
JERRYX_ARG_INTEGER (int32)

#undef JERRYX_ARG_INTEGER

static inline jerryx_arg_t
jerryx_arg_number (double *dest, jerryx_arg_coerce_t coerce_flag, jerryx_arg_optional_t opt_flag);
static inline jerryx_arg_t
jerryx_arg_boolean (bool *dest, jerryx_arg_coerce_t coerce_flag, jerryx_arg_optional_t opt_flag);
static inline jerryx_arg_t
jerryx_arg_string (char *dest, uint32_t size, jerryx_arg_coerce_t coerce_flag, jerryx_arg_optional_t opt_flag);
static inline jerryx_arg_t
jerryx_arg_utf8_string (char *dest, uint32_t size, jerryx_arg_coerce_t coerce_flag, jerryx_arg_optional_t opt_flag);
static inline jerryx_arg_t jerryx_arg_function (jerry_value_t *dest, jerryx_arg_optional_t opt_flag);
static inline jerryx_arg_t
jerryx_arg_native_pointer (void **dest, const jerry_object_native_info_t *info_p, jerryx_arg_optional_t opt_flag);
static inline jerryx_arg_t jerryx_arg_ignore (void);
static inline jerryx_arg_t jerryx_arg_custom (void *dest, uintptr_t extra_info, jerryx_arg_transform_func_t func);
static inline jerryx_arg_t jerryx_arg_object_properties (const jerryx_arg_object_props_t *object_props_p,
                                                         jerryx_arg_optional_t opt_flag);
static inline jerryx_arg_t jerryx_arg_array (const jerryx_arg_array_items_t *array_items_p,
                                             jerryx_arg_optional_t opt_flag);

jerry_value_t jerryx_arg_transform_optional (jerryx_arg_js_iterator_t *js_arg_iter_p,
                                             const jerryx_arg_t *c_arg_p,
                                             jerryx_arg_transform_func_t func);

/* Helper functions for transform functions. */
jerry_value_t jerryx_arg_js_iterator_pop (jerryx_arg_js_iterator_t *js_arg_iter_p);
jerry_value_t jerryx_arg_js_iterator_restore (jerryx_arg_js_iterator_t *js_arg_iter_p);
jerry_value_t jerryx_arg_js_iterator_peek (jerryx_arg_js_iterator_t *js_arg_iter_p);
jerry_length_t jerryx_arg_js_iterator_index (jerryx_arg_js_iterator_t *js_arg_iter_p);


#ifndef JERRYX_ARG_IMPL_H
#define JERRYX_ARG_IMPL_H

/* transform functions for each type. */

#define JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL(type)                                                               \
  jerry_value_t jerryx_arg_transform_##type (jerryx_arg_js_iterator_t *js_arg_iter_p, const jerryx_arg_t *c_arg_p); \
  jerry_value_t jerryx_arg_transform_##type##_optional (jerryx_arg_js_iterator_t *js_arg_iter_p,                    \
                                                        const jerryx_arg_t *c_arg_p);

#define JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL_AND_STRICT(type) \
  JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL (type)                 \
  JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL (type##_strict)

JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL_AND_STRICT (uint8)
JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL_AND_STRICT (int8)
JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL_AND_STRICT (uint16)
JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL_AND_STRICT (int16)
JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL_AND_STRICT (uint32)
JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL_AND_STRICT (int32)
JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL_AND_STRICT (number)
JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL_AND_STRICT (string)
JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL_AND_STRICT (utf8_string)
JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL_AND_STRICT (boolean)

JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL (function)
JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL (native_pointer)
JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL (object_props)
JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL (array_items)

jerry_value_t jerryx_arg_transform_ignore (jerryx_arg_js_iterator_t *js_arg_iter_p, const jerryx_arg_t *c_arg_p);

#undef JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL
#undef JERRYX_ARG_TRANSFORM_FUNC_WITH_OPTIONAL_AND_STRICT

/**
 * The structure indicates the options used to transform integer argument.
 * It will be passed into jerryx_arg_t's  extra_info field.
 */
typedef struct
{
  uint8_t round; /**< rounding policy */
  uint8_t clamp; /**< clamping policy */
} jerryx_arg_int_option_t;

/**
 * The macro used to generate jerryx_arg_xxx for int type.
 */
#define JERRYX_ARG_INT(type)                                                                  \
  static inline jerryx_arg_t jerryx_arg_##type (type##_t *dest,                               \
                                                jerryx_arg_round_t round_flag,                \
                                                jerryx_arg_clamp_t clamp_flag,                \
                                                jerryx_arg_coerce_t coerce_flag,              \
                                                jerryx_arg_optional_t opt_flag)               \
  {                                                                                           \
    jerryx_arg_transform_func_t func;                                                         \
    if (coerce_flag == JERRYX_ARG_NO_COERCE)                                                  \
    {                                                                                         \
      if (opt_flag == JERRYX_ARG_OPTIONAL)                                                    \
      {                                                                                       \
        func = jerryx_arg_transform_##type##_strict_optional;                                 \
      }                                                                                       \
      else                                                                                    \
      {                                                                                       \
        func = jerryx_arg_transform_##type##_strict;                                          \
      }                                                                                       \
    }                                                                                         \
    else                                                                                      \
    {                                                                                         \
      if (opt_flag == JERRYX_ARG_OPTIONAL)                                                    \
      {                                                                                       \
        func = jerryx_arg_transform_##type##_optional;                                        \
      }                                                                                       \
      else                                                                                    \
      {                                                                                       \
        func = jerryx_arg_transform_##type;                                                   \
      }                                                                                       \
    }                                                                                         \
    union                                                                                     \
    {                                                                                         \
      jerryx_arg_int_option_t int_option;                                                     \
      uintptr_t extra_info;                                                                   \
    } u = { .int_option = { .round = (uint8_t) round_flag, .clamp = (uint8_t) clamp_flag } }; \
    return (jerryx_arg_t){ .func = func, .dest = (void *) dest, .extra_info = u.extra_info }; \
  }

JERRYX_ARG_INT (uint8)
JERRYX_ARG_INT (int8)
JERRYX_ARG_INT (uint16)
JERRYX_ARG_INT (int16)
JERRYX_ARG_INT (uint32)
JERRYX_ARG_INT (int32)

#undef JERRYX_ARG_INT

/**
 * Create a validation/transformation step (`jerryx_arg_t`) that expects to
 * consume one `number` JS argument and stores it into a C `double`.
 *
 * @return a jerryx_arg_t instance.
 */
static inline jerryx_arg_t
jerryx_arg_number (double *dest, /**< pointer to the double where the result should be stored */
                   jerryx_arg_coerce_t coerce_flag, /**< whether type coercion is allowed */
                   jerryx_arg_optional_t opt_flag) /**< whether the argument is optional */
{
  jerryx_arg_transform_func_t func;

  if (coerce_flag == JERRYX_ARG_NO_COERCE)
  {
    if (opt_flag == JERRYX_ARG_OPTIONAL)
    {
      func = jerryx_arg_transform_number_strict_optional;
    }
    else
    {
      func = jerryx_arg_transform_number_strict;
    }
  }
  else
  {
    if (opt_flag == JERRYX_ARG_OPTIONAL)
    {
      func = jerryx_arg_transform_number_optional;
    }
    else
    {
      func = jerryx_arg_transform_number;
    }
  }

  return (jerryx_arg_t){ .func = func, .dest = (void *) dest };
} /* jerryx_arg_number */

/**
 * Create a validation/transformation step (`jerryx_arg_t`) that expects to
 * consume one `boolean` JS argument and stores it into a C `bool`.
 *
 * @return a jerryx_arg_t instance.
 */
static inline jerryx_arg_t
jerryx_arg_boolean (bool *dest, /**< points to the native bool */
                    jerryx_arg_coerce_t coerce_flag, /**< whether type coercion is allowed */
                    jerryx_arg_optional_t opt_flag) /**< whether the argument is optional */
{
  jerryx_arg_transform_func_t func;

  if (coerce_flag == JERRYX_ARG_NO_COERCE)
  {
    if (opt_flag == JERRYX_ARG_OPTIONAL)
    {
      func = jerryx_arg_transform_boolean_strict_optional;
    }
    else
    {
      func = jerryx_arg_transform_boolean_strict;
    }
  }
  else
  {
    if (opt_flag == JERRYX_ARG_OPTIONAL)
    {
      func = jerryx_arg_transform_boolean_optional;
    }
    else
    {
      func = jerryx_arg_transform_boolean;
    }
  }

  return (jerryx_arg_t){ .func = func, .dest = (void *) dest };
} /* jerryx_arg_boolean */

/**
 * Create a validation/transformation step (`jerryx_arg_t`) that expects to
 * consume one `string` JS argument and stores it into a C `char` array.
 *
 * @return a jerryx_arg_t instance.
 */
static inline jerryx_arg_t
jerryx_arg_string (char *dest, /**< pointer to the native char array where the result should be stored */
                   uint32_t size, /**< the size of native char array */
                   jerryx_arg_coerce_t coerce_flag, /**< whether type coercion is allowed */
                   jerryx_arg_optional_t opt_flag) /**< whether the argument is optional */
{
  jerryx_arg_transform_func_t func;

  if (coerce_flag == JERRYX_ARG_NO_COERCE)
  {
    if (opt_flag == JERRYX_ARG_OPTIONAL)
    {
      func = jerryx_arg_transform_string_strict_optional;
    }
    else
    {
      func = jerryx_arg_transform_string_strict;
    }
  }
  else
  {
    if (opt_flag == JERRYX_ARG_OPTIONAL)
    {
      func = jerryx_arg_transform_string_optional;
    }
    else
    {
      func = jerryx_arg_transform_string;
    }
  }

  return (jerryx_arg_t){ .func = func, .dest = (void *) dest, .extra_info = (uintptr_t) size };
} /* jerryx_arg_string */

/**
 * Create a validation/transformation step (`jerryx_arg_t`) that expects to
 * consume one `string` JS argument and stores it into a C utf8 `char` array.
 *
 * @return a jerryx_arg_t instance.
 */
static inline jerryx_arg_t
jerryx_arg_utf8_string (char *dest, /**< [out] pointer to the native char array where the result should be stored */
                        uint32_t size, /**< the size of native char array */
                        jerryx_arg_coerce_t coerce_flag, /**< whether type coercion is allowed */
                        jerryx_arg_optional_t opt_flag) /**< whether the argument is optional */
{
  jerryx_arg_transform_func_t func;

  if (coerce_flag == JERRYX_ARG_NO_COERCE)
  {
    if (opt_flag == JERRYX_ARG_OPTIONAL)
    {
      func = jerryx_arg_transform_utf8_string_strict_optional;
    }
    else
    {
      func = jerryx_arg_transform_utf8_string_strict;
    }
  }
  else
  {
    if (opt_flag == JERRYX_ARG_OPTIONAL)
    {
      func = jerryx_arg_transform_utf8_string_optional;
    }
    else
    {
      func = jerryx_arg_transform_utf8_string;
    }
  }

  return (jerryx_arg_t){ .func = func, .dest = (void *) dest, .extra_info = (uintptr_t) size };
} /* jerryx_arg_utf8_string */

/**
 * Create a validation/transformation step (`jerryx_arg_t`) that expects to
 * consume one `function` JS argument and stores it into a C `jerry_value_t`.
 *
 * @return a jerryx_arg_t instance.
 */
static inline jerryx_arg_t
jerryx_arg_function (jerry_value_t *dest, /**< pointer to the jerry_value_t where the result should be stored */
                     jerryx_arg_optional_t opt_flag) /**< whether the argument is optional */
{
  jerryx_arg_transform_func_t func;

  if (opt_flag == JERRYX_ARG_OPTIONAL)
  {
    func = jerryx_arg_transform_function_optional;
  }
  else
  {
    func = jerryx_arg_transform_function;
  }

  return (jerryx_arg_t){ .func = func, .dest = (void *) dest };
} /* jerryx_arg_function */

/**
 * Create a validation/transformation step (`jerryx_arg_t`) that expects to
 * consume one `object` JS argument that is 'backed' with a native pointer with
 * a given type info. In case the native pointer info matches, the transform
 * will succeed and the object's native pointer will be assigned to *dest.
 *
 * @return a jerryx_arg_t instance.
 */
static inline jerryx_arg_t
jerryx_arg_native_pointer (void **dest, /**< pointer to where the resulting native pointer should be stored */
                           const jerry_object_native_info_t *info_p, /**< expected the type info */
                           jerryx_arg_optional_t opt_flag) /**< whether the argument is optional */
{
  jerryx_arg_transform_func_t func;

  if (opt_flag == JERRYX_ARG_OPTIONAL)
  {
    func = jerryx_arg_transform_native_pointer_optional;
  }
  else
  {
    func = jerryx_arg_transform_native_pointer;
  }

  return (jerryx_arg_t){ .func = func, .dest = (void *) dest, .extra_info = (uintptr_t) info_p };
} /* jerryx_arg_native_pointer */

/**
 * Create a jerryx_arg_t instance for ignored argument.
 *
 * @return a jerryx_arg_t instance.
 */
static inline jerryx_arg_t
jerryx_arg_ignore (void)
{
  return (jerryx_arg_t){ .func = jerryx_arg_transform_ignore };
} /* jerryx_arg_ignore */

/**
 * Create a jerryx_arg_t instance with custom transform.
 *
 * @return a jerryx_arg_t instance.
 */
static inline jerryx_arg_t
jerryx_arg_custom (void *dest, /**< pointer to the native argument where the result should be stored */
                   uintptr_t extra_info, /**< the extra parameter, specific to the transform function */
                   jerryx_arg_transform_func_t func) /**< the custom transform function */
{
  return (jerryx_arg_t){ .func = func, .dest = dest, .extra_info = extra_info };
} /* jerryx_arg_custom */

/**
 * Create a jerryx_arg_t instance for object properties.
 *
 * @return a jerryx_arg_t instance.
 */
static inline jerryx_arg_t
jerryx_arg_object_properties (const jerryx_arg_object_props_t *object_props, /**< pointer to object property mapping */
                              jerryx_arg_optional_t opt_flag) /**< whether the argument is optional */
{
  jerryx_arg_transform_func_t func;

  if (opt_flag == JERRYX_ARG_OPTIONAL)
  {
    func = jerryx_arg_transform_object_props_optional;
  }
  else
  {
    func = jerryx_arg_transform_object_props;
  }

  return (jerryx_arg_t){ .func = func, .dest = NULL, .extra_info = (uintptr_t) object_props };
} /* jerryx_arg_object_properties */

/**
 * Create a jerryx_arg_t instance for array.
 *
 * @return a jerryx_arg_t instance.
 */
static inline jerryx_arg_t
jerryx_arg_array (const jerryx_arg_array_items_t *array_items_p, /**< pointer to array items mapping */
                  jerryx_arg_optional_t opt_flag) /**< whether the argument is optional */
{
  jerryx_arg_transform_func_t func;

  if (opt_flag == JERRYX_ARG_OPTIONAL)
  {
    func = jerryx_arg_transform_array_items_optional;
  }
  else
  {
    func = jerryx_arg_transform_array_items;
  }

  return (jerryx_arg_t){ .func = func, .dest = NULL, .extra_info = (uintptr_t) array_items_p };
} /* jerryx_arg_array */

#endif /* !JERRYX_ARG_IMPL_H */

JERRY_C_API_END

#endif /* !JERRYX_ARG_H */

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

#ifndef JERRYX_HANDLERS_H
#define JERRYX_HANDLERS_H


JERRY_C_API_BEGIN

jerry_value_t jerryx_handler_assert (const jerry_call_info_t *call_info_p,
                                     const jerry_value_t args_p[],
                                     const jerry_length_t args_cnt);
jerry_value_t
jerryx_handler_gc (const jerry_call_info_t *call_info_p, const jerry_value_t args_p[], const jerry_length_t args_cnt);
jerry_value_t jerryx_handler_print (const jerry_call_info_t *call_info_p,
                                    const jerry_value_t args_p[],
                                    const jerry_length_t args_cnt);
jerry_value_t jerryx_handler_source_name (const jerry_call_info_t *call_info_p,
                                          const jerry_value_t args_p[],
                                          const jerry_length_t args_cnt);
jerry_value_t jerryx_handler_create_realm (const jerry_call_info_t *call_info_p,
                                           const jerry_value_t args_p[],
                                           const jerry_length_t args_cnt);
void jerryx_handler_promise_reject (jerry_promise_event_type_t event_type,
                                    const jerry_value_t object,
                                    const jerry_value_t value,
                                    void *user_p);
jerry_value_t jerryx_handler_source_received (const jerry_char_t *source_name_p,
                                              size_t source_name_size,
                                              const jerry_char_t *source_p,
                                              size_t source_size,
                                              void *user_p);
JERRY_C_API_END

#endif /* !JERRYX_HANDLERS_H */

#ifndef JERRYX_HANDLE_SCOPE_H
#define JERRYX_HANDLE_SCOPE_H


JERRY_C_API_BEGIN

#ifndef JERRYX_HANDLE_PRELIST_SIZE
#define JERRYX_HANDLE_PRELIST_SIZE 20
#endif /* !defined(JERRYX_HANDLE_PRELIST_SIZE) */

#ifndef JERRYX_SCOPE_PRELIST_SIZE
#define JERRYX_SCOPE_PRELIST_SIZE 20
#endif /* !defined(JERRYX_SCOPE_PRELIST_SIZE) */

typedef struct jerryx_handle_t jerryx_handle_t;
/**
 * Dynamically allocated handle in the scopes.
 * Scopes has it's own size-limited linear storage of handles. Still there
 * might be not enough space left for new handles, dynamically allocated
 * `jerryx_handle_t` could ease the pre-allocated linear memory burden.
 */
struct jerryx_handle_t
{
  jerry_value_t jval; /**< jerry value of the handle bound to */
  jerryx_handle_t *sibling; /**< next sibling the the handle */
};

#define JERRYX_HANDLE_SCOPE_FIELDS                          \
  jerry_value_t handle_prelist[JERRYX_HANDLE_PRELIST_SIZE]; \
  uint8_t prelist_handle_count;                             \
  bool escaped;                                             \
  jerryx_handle_t *handle_ptr

typedef struct jerryx_handle_scope_s jerryx_handle_scope_t;
typedef jerryx_handle_scope_t *jerryx_handle_scope;
typedef jerryx_handle_scope_t *jerryx_escapable_handle_scope;
/**
 * Inlined simple handle scope type.
 */
struct jerryx_handle_scope_s
{
  JERRYX_HANDLE_SCOPE_FIELDS; /**< common handle scope fields */
};

typedef struct jerryx_handle_scope_dynamic_s jerryx_handle_scope_dynamic_t;
/**
 * Dynamically allocated handle scope type.
 */
struct jerryx_handle_scope_dynamic_s
{
  JERRYX_HANDLE_SCOPE_FIELDS; /**< common handle scope fields */
  jerryx_handle_scope_dynamic_t *child; /**< child dynamically allocated handle scope */
  jerryx_handle_scope_dynamic_t *parent; /**< parent dynamically allocated handle scope */
};

#undef JERRYX_HANDLE_SCOPE_FIELDS

typedef enum
{
  jerryx_handle_scope_ok = 0,

  jerryx_escape_called_twice,
  jerryx_handle_scope_mismatch,
} jerryx_handle_scope_status;

jerryx_handle_scope_status jerryx_open_handle_scope (jerryx_handle_scope *result);

jerryx_handle_scope_status jerryx_close_handle_scope (jerryx_handle_scope scope);

jerryx_handle_scope_status jerryx_open_escapable_handle_scope (jerryx_handle_scope *result);

jerryx_handle_scope_status jerryx_close_escapable_handle_scope (jerryx_handle_scope scope);

jerryx_handle_scope_status
jerryx_escape_handle (jerryx_escapable_handle_scope scope, jerry_value_t escapee, jerry_value_t *result);

/**
 * Completely escape a handle from handle scope,
 * leave life time management totally up to user.
 */
jerryx_handle_scope_status
jerryx_remove_handle (jerryx_escapable_handle_scope scope, jerry_value_t escapee, jerry_value_t *result);

jerry_value_t jerryx_create_handle (jerry_value_t jval);

jerry_value_t jerryx_create_handle_in_scope (jerry_value_t jval, jerryx_handle_scope scope);

/** MARK: - handle-scope-allocator.c */
jerryx_handle_scope_t *jerryx_handle_scope_get_current (void);

jerryx_handle_scope_t *jerryx_handle_scope_get_root (void);
/** MARK: - END handle-scope-allocator.c */

JERRY_C_API_END

#endif /* !JERRYX_HANDLE_SCOPE_H */

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

#ifndef JERRYX_MODULE_H
#define JERRYX_MODULE_H


JERRY_C_API_BEGIN

/**
 * Declare the signature for the module initialization function.
 */
typedef jerry_value_t (*jerryx_native_module_on_resolve_t) (void);

/**
 * Declare the structure used to define a module. One should only make use of this structure via the
 * JERRYX_NATIVE_MODULE macro declared below.
 */
typedef struct jerryx_native_module_t
{
  const jerry_char_t *name_p; /**< name of the module */
  const jerryx_native_module_on_resolve_t on_resolve_p; /**< function that returns a new instance of the module */
  struct jerryx_native_module_t *next_p; /**< pointer to next module in the list */
} jerryx_native_module_t;

/**
 * Declare the constructor and destructor attributes. These evaluate to nothing if this extension is built without
 * library constructor/destructor support.
 */
#ifdef ENABLE_INIT_FINI
#ifdef _MSC_VER
/**
 * Only Visual Studio 2008 and upper version support for __pragma keyword
 * refer to https://msdn.microsoft.com/en-us/library/d9x1s805(v=vs.90).aspx
 */
#if _MSC_VER >= 1500
#ifdef _WIN64
#define JERRYX_MSVC_INCLUDE_SYM(s) comment (linker, "/include:" #s)
#else /* !_WIN64 */
#define JERRYX_MSVC_INCLUDE_SYM(s) comment (linker, "/include:_" #s)
#endif /* _WIN64 */

#ifdef __cplusplus
#define JERRYX_MSCV_EXTERN_C extern "C"
#else /* !__cplusplus */
#define JERRYX_MSCV_EXTERN_C
#endif /* __cplusplus */

#pragma section(".CRT$XCU", read)
#pragma section(".CRT$XTU", read)

#define JERRYX_MSVC_FUNCTION_ON_SECTION(sec_name, f)                               \
  static void f (void);                                                            \
  __pragma (JERRYX_MSVC_INCLUDE_SYM (f##_section)) __declspec(allocate (sec_name)) \
    JERRYX_MSCV_EXTERN_C void (*f##_section) (void) = f;                           \
  static void f (void)

#define JERRYX_MODULE_CONSTRUCTOR(f) JERRYX_MSVC_FUNCTION_ON_SECTION (".CRT$XCU", f)
#define JERRYX_MODULE_DESTRUCTOR(f)  JERRYX_MSVC_FUNCTION_ON_SECTION (".CRT$XTU", f)
#else /* !(_MSC_VER >= 1500) */
#error "Only Visual Studio 2008 and upper version are supported."
#endif /* _MSC_VER >= 1500 */
#elif defined(__GNUC__)
#define JERRYX_MODULE_CONSTRUCTOR(f)                  \
  static void f (void) __attribute__ ((constructor)); \
  static void f (void)

#define JERRYX_MODULE_DESTRUCTOR(f)                  \
  static void f (void) __attribute__ ((destructor)); \
  static void f (void)
#else /* __GNUC__ */
#error "`FEATURE_INIT_FINI` build flag isn't supported on this compiler"
#endif /* _MSC_VER */
#else /* !ENABLE_INIT_FINI */
#define JERRYX_MODULE_CONSTRUCTOR(f) \
  void f (void);                     \
  void f (void)

#define JERRYX_MODULE_DESTRUCTOR(f) \
  void f (void);                    \
  void f (void)
#endif /* ENABLE_INIT_FINI */

/**
 * Having two levels of macros allows strings to be used unquoted.
 */
#define JERRYX_NATIVE_MODULE(module_name, on_resolve_cb) JERRYX_NATIVE_MODULE_IMPLEM (module_name, on_resolve_cb)

#define JERRYX_NATIVE_MODULE_IMPLEM(module_name, on_resolve_cb)                                          \
  static jerryx_native_module_t _##module_name##_definition = { .name_p = (jerry_char_t *) #module_name, \
                                                                .on_resolve_p = (on_resolve_cb),         \
                                                                .next_p = NULL };                        \
                                                                                                         \
  JERRYX_MODULE_CONSTRUCTOR (module_name##_register)                                                     \
  {                                                                                                      \
    jerryx_native_module_register (&_##module_name##_definition);                                        \
  }                                                                                                      \
                                                                                                         \
  JERRYX_MODULE_DESTRUCTOR (module_name##_unregister)                                                    \
  {                                                                                                      \
    jerryx_native_module_unregister (&_##module_name##_definition);                                      \
  }

/**
 * Register a native module. This makes it available for loading via jerryx_module_resolve, when
 * jerryx_module_native_resolver is passed in as a possible resolver.
 */
void jerryx_native_module_register (jerryx_native_module_t *module_p);

/**
 * Unregister a native module. This removes the module from the list of available native modules, meaning that
 * subsequent calls to jerryx_module_resolve with jerryx_module_native_resolver will not be able to find it.
 */
void jerryx_native_module_unregister (jerryx_native_module_t *module_p);

/**
 * Declare the function pointer type for canonical name resolution.
 */
typedef jerry_value_t (*jerryx_module_get_canonical_name_t) (const jerry_value_t name); /**< The name for which to
                                                                                         *   compute the canonical
                                                                                         *   name */

/**
 * Declare the function pointer type for module resolution.
 */
typedef bool (*jerryx_module_resolve_t) (const jerry_value_t canonical_name, /**< The module's canonical name */
                                         jerry_value_t *result); /**< The resulting module, if the function returns
                                                                  *   true */

/**
 * Declare the structure for module resolvers.
 */
typedef struct
{
  jerryx_module_get_canonical_name_t get_canonical_name_p; /**< function pointer to establish the canonical name of a
                                                            *   module */
  jerryx_module_resolve_t resolve_p; /**< function pointer to resolve a module */
} jerryx_module_resolver_t;

/**
 * Declare the JerryScript module resolver so that it may be added to an array of jerryx_module_resolver_t items and
 * thus passed to jerryx_module_resolve.
 */
extern jerryx_module_resolver_t jerryx_module_native_resolver;

/**
 * Load a copy of a module into the current context using the provided module resolvers, or return one that was already
 * loaded if it is found.
 */
jerry_value_t
jerryx_module_resolve (const jerry_value_t name, const jerryx_module_resolver_t **resolvers, size_t count);

/**
 * Delete a module from the cache or, if name has the JavaScript value of undefined, clear the entire cache.
 */
void jerryx_module_clear_cache (const jerry_value_t name, const jerryx_module_resolver_t **resolvers, size_t count);

JERRY_C_API_END

#endif /* !JERRYX_MODULE_H */

#ifndef JERRYX_PRINT_H
#define JERRYX_PRINT_H


JERRY_C_API_BEGIN

jerry_value_t jerryx_print_value (const jerry_value_t value);
void jerryx_print_byte (jerry_char_t ch);
void jerryx_print_buffer (const jerry_char_t *buffer_p, jerry_size_t buffer_size);
void jerryx_print_string (const char *str_p);
void jerryx_print_backtrace (unsigned depth);
void jerryx_print_unhandled_exception (jerry_value_t exception);
void jerryx_print_unhandled_rejection (jerry_value_t exception);

JERRY_C_API_END

#endif /* !JERRYX_PRINT_H */

#ifndef JERRYX_PROPERTIES_H
#define JERRYX_PROPERTIES_H


JERRY_C_API_BEGIN

/*
 * Handler registration helper
 */

bool jerryx_register_global (const char *name_p, jerry_external_handler_t handler_p);

/**
 * Struct used by the `jerryx_set_functions` method to
 * register multiple methods for a given object.
 */
typedef struct
{
  const char *name; /**< name of the property to add */
  jerry_value_t value; /**< value of the property */
} jerryx_property_entry;

#define JERRYX_PROPERTY_NUMBER(NAME, NUMBER) \
  (jerryx_property_entry)                    \
  {                                          \
    NAME, jerry_number (NUMBER)              \
  }
#define JERRYX_PROPERTY_STRING(NAME, STR, SIZE)                                \
  (jerryx_property_entry)                                                      \
  {                                                                            \
    NAME, jerry_string ((const jerry_char_t *) STR, SIZE, JERRY_ENCODING_UTF8) \
  }
#define JERRYX_PROPERTY_STRING_SZ(NAME, STR) \
  (jerryx_property_entry)                    \
  {                                          \
    NAME, jerry_string_sz (STR)              \
  }
#define JERRYX_PROPERTY_BOOLEAN(NAME, VALUE) \
  (jerryx_property_entry)                    \
  {                                          \
    NAME, jerry_boolean (VALUE)              \
  }
#define JERRYX_PROPERTY_FUNCTION(NAME, FUNC) \
  (jerryx_property_entry)                    \
  {                                          \
    NAME, jerry_function_external (FUNC)     \
  }
#define JERRYX_PROPERTY_UNDEFINED(NAME) \
  (jerryx_property_entry)               \
  {                                     \
    NAME, jerry_undefined ()            \
  }
#define JERRYX_PROPERTY_LIST_END() \
  (jerryx_property_entry)          \
  {                                \
    NULL, 0                        \
  }

/**
 * Stores the result of property register operation.
 */
typedef struct
{
  jerry_value_t result; /**< result of property registration (undefined or error object) */
  uint32_t registered; /**< number of successfully registered methods */
} jerryx_register_result;

jerryx_register_result jerryx_set_properties (const jerry_value_t target_object, const jerryx_property_entry entries[]);

void jerryx_release_property_entry (const jerryx_property_entry entries[],
                                    const jerryx_register_result register_result);

JERRY_C_API_END

#endif /* !JERRYX_PROPERTIES_H */

#ifndef JERRYX_REPL_H
#define JERRYX_REPL_H


JERRY_C_API_BEGIN

void jerryx_repl (const char* prompt_p);

JERRY_C_API_END

#endif /* !JERRYX_REPL_H */

#ifndef JERRYX_SOURCES_H
#define JERRYX_SOURCES_H


JERRY_C_API_BEGIN

jerry_value_t jerryx_source_parse_script (const char* path);
jerry_value_t jerryx_source_exec_script (const char* path);
jerry_value_t jerryx_source_exec_module (const char* path);
jerry_value_t jerryx_source_exec_snapshot (const char* path, size_t function_index);
jerry_value_t jerryx_source_exec_stdin (void);

JERRY_C_API_END

#endif /* !JERRYX_EXEC_H */

#ifndef JERRYX_TEST262_H
#define JERRYX_TEST262_H


JERRY_C_API_BEGIN

void jerryx_test262_register (void);

JERRY_C_API_END

#endif /* !JERRYX_TEST262_H */
