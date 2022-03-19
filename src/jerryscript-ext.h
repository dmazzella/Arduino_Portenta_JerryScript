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

#ifndef JERRYX_EXT_H
#define JERRYX_EXT_H

#include "jerryscript.h"

JERRY_C_API_BEGIN

/* !JEXT_COMMON_H */

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
/* !JEXT_COMMON_H */

/* !JERRYX_PRINT_H */
jerry_value_t jerryx_print_value (const jerry_value_t value);
void jerryx_print_byte (jerry_char_t ch);
void jerryx_print_buffer (const jerry_char_t *buffer_p, jerry_size_t buffer_size);
void jerryx_print_string (const char *str_p);
void jerryx_print_backtrace (unsigned depth);
void jerryx_print_unhandled_exception (jerry_value_t exception);
void jerryx_print_unhandled_rejection (jerry_value_t exception);
/* !JERRYX_PRINT_H */

/* !JERRYX_HANDLERS_H */
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
/* !JERRYX_HANDLERS_H */


/* !JERRYX_PROPERTIES_H */
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
/* !JERRYX_PROPERTIES_H */

/* !JERRYX_MODULE_H */
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
/* !JERRYX_MODULE_H */

/* !JERRYX_REPL_H */
void jerryx_repl (const char* prompt_p);
/* !JERRYX_REPL_H */

JERRY_C_API_END

#endif /* !JERRYX_EXT_H */