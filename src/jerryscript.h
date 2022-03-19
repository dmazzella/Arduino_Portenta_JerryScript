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
#include "jerryscript-config.h"

#ifndef JERRYSCRIPT_H
#define JERRYSCRIPT_H

/**
 * Major version of JerryScript API.
 */
#define JERRY_API_MAJOR_VERSION 3

/**
 * Minor version of JerryScript API.
 */
#define JERRY_API_MINOR_VERSION 0

/**
 * Patch version of JerryScript API.
 */
#define JERRY_API_PATCH_VERSION 0


#ifndef JERRYSCRIPT_CORE_H
#define JERRYSCRIPT_CORE_H


#ifndef JERRYSCRIPT_TYPES_H
#define JERRYSCRIPT_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#ifndef JERRYSCRIPT_COMPILER_H
#define JERRYSCRIPT_COMPILER_H

#ifdef __cplusplus
#define JERRY_C_API_BEGIN extern "C" {
#define JERRY_C_API_END   }
#else /* !__cplusplus */
#define JERRY_C_API_BEGIN
#define JERRY_C_API_END
#endif /* __cplusplus */

JERRY_C_API_BEGIN

/** \addtogroup jerry-compiler Jerry compiler compatibility components
 * @{
 */

#ifdef __GNUC__

/*
 * Compiler-specific macros relevant for GCC.
 */
#define JERRY_ATTR_ALIGNED(ALIGNMENT) __attribute__ ((aligned (ALIGNMENT)))
#define JERRY_ATTR_ALWAYS_INLINE      __attribute__ ((always_inline))
#define JERRY_ATTR_CONST              __attribute__ ((const))
#define JERRY_ATTR_DEPRECATED         __attribute__ ((deprecated))
#define JERRY_ATTR_FORMAT(...)        __attribute__ ((format (__VA_ARGS__)))
#define JERRY_ATTR_HOT                __attribute__ ((hot))
#define JERRY_ATTR_NOINLINE           __attribute__ ((noinline))
#define JERRY_ATTR_NORETURN           __attribute__ ((noreturn))
#define JERRY_ATTR_PURE               __attribute__ ((pure))
#define JERRY_ATTR_WARN_UNUSED_RESULT __attribute__ ((warn_unused_result))
#define JERRY_ATTR_WEAK               __attribute__ ((weak))

#define JERRY_WEAK_SYMBOL_SUPPORT

#ifndef JERRY_LIKELY
#define JERRY_LIKELY(x) __builtin_expect (!!(x), 1)
#endif /* !JERRY_LIKELY */

#ifndef JERRY_UNLIKELY
#define JERRY_UNLIKELY(x) __builtin_expect (!!(x), 0)
#endif /* !JERRY_UNLIKELY */

#endif /* __GNUC__ */

#ifdef _MSC_VER

/*
 * Compiler-specific macros relevant for Microsoft Visual C/C++ Compiler.
 */
#define JERRY_ATTR_DEPRECATED __declspec(deprecated)
#define JERRY_ATTR_NOINLINE   __declspec(noinline)
#define JERRY_ATTR_NORETURN   __declspec(noreturn)

/*
 * Microsoft Visual C/C++ Compiler doesn't support for VLA, using _alloca
 * instead.
 */
void *__cdecl _alloca (size_t _Size);
#define JERRY_VLA(type, name, size) type *name = (type *) (_alloca (sizeof (type) * (size)))

#endif /* _MSC_VER */

/*
 * Default empty definitions for all compiler-specific macros. Define any of
 * these in a guarded block above (e.g., as for GCC) to fine tune compilation
 * for your own compiler. */

/**
 * Function attribute to align function to given number of bytes.
 */
#ifndef JERRY_ATTR_ALIGNED
#define JERRY_ATTR_ALIGNED(ALIGNMENT)
#endif /* !JERRY_ATTR_ALIGNED */

/**
 * Function attribute to inline function to all call sites.
 */
#ifndef JERRY_ATTR_ALWAYS_INLINE
#define JERRY_ATTR_ALWAYS_INLINE
#endif /* !JERRY_ATTR_ALWAYS_INLINE */

/**
 * Function attribute to declare that function has no effect except the return
 * value and it only depends on parameters.
 */
#ifndef JERRY_ATTR_CONST
#define JERRY_ATTR_CONST
#endif /* !JERRY_ATTR_CONST */

/**
 * Function attribute to trigger warning if deprecated function is called.
 */
#ifndef JERRY_ATTR_DEPRECATED
#define JERRY_ATTR_DEPRECATED
#endif /* !JERRY_ATTR_DEPRECATED */

/**
 * Function attribute to declare that function is variadic and takes a format
 * string and some arguments as parameters.
 */
#ifndef JERRY_ATTR_FORMAT
#define JERRY_ATTR_FORMAT(...)
#endif /* !JERRY_ATTR_FORMAT */

/**
 * Function attribute to predict that function is a hot spot, and therefore
 * should be optimized aggressively.
 */
#ifndef JERRY_ATTR_HOT
#define JERRY_ATTR_HOT
#endif /* !JERRY_ATTR_HOT */

/**
 * Function attribute not to inline function ever.
 */
#ifndef JERRY_ATTR_NOINLINE
#define JERRY_ATTR_NOINLINE
#endif /* !JERRY_ATTR_NOINLINE */

/**
 * Function attribute to declare that function never returns.
 */
#ifndef JERRY_ATTR_NORETURN
#define JERRY_ATTR_NORETURN
#endif /* !JERRY_ATTR_NORETURN */

/**
 * Function attribute to declare that function has no effect except the return
 * value and it only depends on parameters and global variables.
 */
#ifndef JERRY_ATTR_PURE
#define JERRY_ATTR_PURE
#endif /* !JERRY_ATTR_PURE */

/**
 * Function attribute to trigger warning if function's caller doesn't use (e.g.,
 * check) the return value.
 */
#ifndef JERRY_ATTR_WARN_UNUSED_RESULT
#define JERRY_ATTR_WARN_UNUSED_RESULT
#endif /* !JERRY_ATTR_WARN_UNUSED_RESULT */

/**
 * Function attribute to declare a function a weak symbol
 */
#ifndef JERRY_ATTR_WEAK
#define JERRY_ATTR_WEAK
#endif /* !JERRY_ATTR_WEAK */

/**
 * Helper to predict that a condition is likely.
 */
#ifndef JERRY_LIKELY
#define JERRY_LIKELY(x) (x)
#endif /* !JERRY_LIKELY */

/**
 * Helper to predict that a condition is unlikely.
 */
#ifndef JERRY_UNLIKELY
#define JERRY_UNLIKELY(x) (x)
#endif /* !JERRY_UNLIKELY */

/**
 * Helper to declare (or mimic) a C99 variable-length array.
 */
#ifndef JERRY_VLA
#define JERRY_VLA(type, name, size) type name[size]
#endif /* !JERRY_VLA */

/**
 * @}
 */

JERRY_C_API_END

#endif /* !JERRYSCRIPT_COMPILER_H */

JERRY_C_API_BEGIN

/**
 * @defgroup jerry-api-types JerryScript public API types
 * @{
 */

/**
 * JerryScript init flags.
 */
typedef enum
{
  JERRY_INIT_EMPTY = (0u), /**< empty flag set */
  JERRY_INIT_SHOW_OPCODES = (1u << 0), /**< dump byte-code to log after parse */
  JERRY_INIT_SHOW_REGEXP_OPCODES = (1u << 1), /**< dump regexp byte-code to log after compilation */
  JERRY_INIT_MEM_STATS = (1u << 2), /**< dump memory statistics */
} jerry_init_flag_t;

/**
 * Jerry log levels. The levels are in severity order
 * where the most serious levels come first.
 */
typedef enum
{
  JERRY_LOG_LEVEL_ERROR = 0u, /**< the engine will terminate after the message is printed */
  JERRY_LOG_LEVEL_WARNING = 1u, /**< a request is aborted, but the engine continues its operation */
  JERRY_LOG_LEVEL_DEBUG = 2u, /**< debug messages from the engine, low volume */
  JERRY_LOG_LEVEL_TRACE = 3u /**< detailed info about engine internals, potentially high volume */
} jerry_log_level_t;

/**
 * JerryScript API Error object types.
 */
typedef enum
{
  JERRY_ERROR_NONE = 0, /**< No Error */

  JERRY_ERROR_COMMON, /**< Error */
  JERRY_ERROR_EVAL, /**< EvalError */
  JERRY_ERROR_RANGE, /**< RangeError */
  JERRY_ERROR_REFERENCE, /**< ReferenceError */
  JERRY_ERROR_SYNTAX, /**< SyntaxError */
  JERRY_ERROR_TYPE, /**< TypeError */
  JERRY_ERROR_URI, /**< URIError */
  JERRY_ERROR_AGGREGATE /**< AggregateError */
} jerry_error_t;

/**
 * JerryScript feature types.
 */
typedef enum
{
  JERRY_FEATURE_CPOINTER_32_BIT, /**< 32 bit compressed pointers */
  JERRY_FEATURE_ERROR_MESSAGES, /**< error messages */
  JERRY_FEATURE_JS_PARSER, /**< js-parser */
  JERRY_FEATURE_HEAP_STATS, /**< memory statistics */
  JERRY_FEATURE_PARSER_DUMP, /**< parser byte-code dumps */
  JERRY_FEATURE_REGEXP_DUMP, /**< regexp byte-code dumps */
  JERRY_FEATURE_SNAPSHOT_SAVE, /**< saving snapshot files */
  JERRY_FEATURE_SNAPSHOT_EXEC, /**< executing snapshot files */
  JERRY_FEATURE_DEBUGGER, /**< debugging */
  JERRY_FEATURE_VM_EXEC_STOP, /**< stopping ECMAScript execution */
  JERRY_FEATURE_VM_THROW, /**< capturing ECMAScript throws */
  JERRY_FEATURE_JSON, /**< JSON support */
  JERRY_FEATURE_PROMISE, /**< promise support */
  JERRY_FEATURE_TYPEDARRAY, /**< Typedarray support */
  JERRY_FEATURE_DATE, /**< Date support */
  JERRY_FEATURE_REGEXP, /**< Regexp support */
  JERRY_FEATURE_LINE_INFO, /**< line info available */
  JERRY_FEATURE_LOGGING, /**< logging */
  JERRY_FEATURE_SYMBOL, /**< symbol support */
  JERRY_FEATURE_DATAVIEW, /**< DataView support */
  JERRY_FEATURE_PROXY, /**< Proxy support */
  JERRY_FEATURE_MAP, /**< Map support */
  JERRY_FEATURE_SET, /**< Set support */
  JERRY_FEATURE_WEAKMAP, /**< WeakMap support */
  JERRY_FEATURE_WEAKSET, /**< WeakSet support */
  JERRY_FEATURE_BIGINT, /**< BigInt support */
  JERRY_FEATURE_REALM, /**< realm support */
  JERRY_FEATURE_GLOBAL_THIS, /**< GlobalThisValue support */
  JERRY_FEATURE_PROMISE_CALLBACK, /**< Promise callback support */
  JERRY_FEATURE_MODULE, /**< Module support */
  JERRY_FEATURE_WEAKREF, /**< WeakRef support */
  JERRY_FEATURE_FUNCTION_TO_STRING, /**< function toString support */
  JERRY_FEATURE__COUNT /**< number of features. NOTE: must be at the end of the list */
} jerry_feature_t;

/**
 * GC operational modes.
 */
typedef enum
{
  JERRY_GC_PRESSURE_LOW, /**< free unused objects, but keep memory
                          *   allocated for performance improvements
                          *   such as property hash tables for large objects */
  JERRY_GC_PRESSURE_HIGH /**< free as much memory as possible */
} jerry_gc_mode_t;

/**
 * Jerry regexp flags.
 */
typedef enum
{
  JERRY_REGEXP_FLAG_GLOBAL = (1u << 1), /**< Globally scan string */
  JERRY_REGEXP_FLAG_IGNORE_CASE = (1u << 2), /**< Ignore case */
  JERRY_REGEXP_FLAG_MULTILINE = (1u << 3), /**< Multiline string scan */
  JERRY_REGEXP_FLAG_STICKY = (1u << 4), /**< ECMAScript v11, 21.2.5.14 */
  JERRY_REGEXP_FLAG_UNICODE = (1u << 5), /**< ECMAScript v11, 21.2.5.17 */
  JERRY_REGEXP_FLAG_DOTALL = (1u << 6) /**< ECMAScript v11, 21.2.5.3 */
} jerry_regexp_flags_t;

/**
 * Character type of JerryScript.
 */
typedef uint8_t jerry_char_t;

/**
 * Size type of JerryScript.
 */
typedef uint32_t jerry_size_t;

/**
 * Length type of JerryScript.
 */
typedef uint32_t jerry_length_t;

/**
 * Description of a JerryScript value.
 */
typedef uint32_t jerry_value_t;

/**
 * Option bits for jerry_parse_options_t.
 */
typedef enum
{
  JERRY_PARSE_NO_OPTS = 0, /**< no options passed */
  JERRY_PARSE_STRICT_MODE = (1 << 0), /**< enable strict mode */
  JERRY_PARSE_MODULE = (1 << 1), /**< parse source as an ECMAScript module */
  JERRY_PARSE_HAS_ARGUMENT_LIST = (1 << 2), /**< argument_list field is valid,
                                             * this also means that function parsing will be done */
  JERRY_PARSE_HAS_SOURCE_NAME = (1 << 3), /**< source_name field is valid */
  JERRY_PARSE_HAS_START = (1 << 4), /**< start_line and start_column fields are valid */
  JERRY_PARSE_HAS_USER_VALUE = (1 << 5), /**< user_value field is valid */
} jerry_parse_option_enable_feature_t;

/**
 * Various configuration options for parsing functions such as jerry_parse or jerry_parse_function.
 */
typedef struct
{
  uint32_t options; /**< combination of jerry_parse_option_enable_feature_t values */
  jerry_value_t argument_list; /**< function argument list if JERRY_PARSE_HAS_ARGUMENT_LIST is set in options
                                *   Note: must be string value */
  jerry_value_t source_name; /**< source name string (usually a file name)
                              *   if JERRY_PARSE_HAS_SOURCE_NAME is set in options
                              *   Note: must be string value */
  uint32_t start_line; /**< start line of the source code if JERRY_PARSE_HAS_START is set in options */
  uint32_t start_column; /**< start column of the source code if JERRY_PARSE_HAS_START is set in options */
  jerry_value_t user_value; /**< user value assigned to all functions created by this script including eval
                             *   calls executed by the script if JERRY_PARSE_HAS_USER_VALUE is set in options */
} jerry_parse_options_t;

/**
 * Description of ECMA property descriptor.
 */
typedef enum
{
  JERRY_PROP_NO_OPTS = (0), /**< empty property descriptor */
  JERRY_PROP_IS_CONFIGURABLE = (1 << 0), /**< [[Configurable]] */
  JERRY_PROP_IS_ENUMERABLE = (1 << 1), /**< [[Enumerable]] */
  JERRY_PROP_IS_WRITABLE = (1 << 2), /**< [[Writable]] */

  JERRY_PROP_IS_CONFIGURABLE_DEFINED = (1 << 3), /**< is [[Configurable]] defined? */
  JERRY_PROP_IS_ENUMERABLE_DEFINED = (1 << 4), /**< is [[Enumerable]] defined? */
  JERRY_PROP_IS_WRITABLE_DEFINED = (1 << 5), /**< is [[Writable]] defined? */

  JERRY_PROP_IS_VALUE_DEFINED = (1 << 6), /**< is [[Value]] defined? */
  JERRY_PROP_IS_GET_DEFINED = (1 << 7), /**< is [[Get]] defined? */
  JERRY_PROP_IS_SET_DEFINED = (1 << 8), /**< is [[Set]] defined? */

  JERRY_PROP_SHOULD_THROW = (1 << 9), /**< should throw on error, instead of returning with false */
} jerry_property_descriptor_flags_t;

/**
 * Description of ECMA property descriptor.
 */
typedef struct
{
  uint16_t flags; /**< any combination of jerry_property_descriptor_flags_t bits */
  jerry_value_t value; /**< [[Value]] */
  jerry_value_t getter; /**< [[Get]] */
  jerry_value_t setter; /**< [[Set]] */
} jerry_property_descriptor_t;

/**
 * JerryScript object property filter options.
 */
typedef enum
{
  JERRY_PROPERTY_FILTER_ALL = 0, /**< List all property keys independently
                                  *   from key type or property value attributes
                                  *   (equivalent to Reflect.ownKeys call)  */
  JERRY_PROPERTY_FILTER_TRAVERSE_PROTOTYPE_CHAIN = (1 << 0), /**< Include keys from the objects's
                                                              *   prototype chain as well */
  JERRY_PROPERTY_FILTER_EXCLUDE_NON_CONFIGURABLE = (1 << 1), /**< Exclude property key if
                                                              *   the property is non-configurable */
  JERRY_PROPERTY_FILTER_EXCLUDE_NON_ENUMERABLE = (1 << 2), /**< Exclude property key if
                                                            *   the property is non-enumerable */
  JERRY_PROPERTY_FILTER_EXCLUDE_NON_WRITABLE = (1 << 3), /**< Exclude property key if
                                                          *   the property is non-writable */
  JERRY_PROPERTY_FILTER_EXCLUDE_STRINGS = (1 << 4), /**< Exclude property key if it is a string */
  JERRY_PROPERTY_FILTER_EXCLUDE_SYMBOLS = (1 << 5), /**< Exclude property key if it is a symbol */
  JERRY_PROPERTY_FILTER_EXCLUDE_INTEGER_INDICES = (1 << 6), /**< Exclude property key if it is an integer index */
  JERRY_PROPERTY_FILTER_INTEGER_INDICES_AS_NUMBER = (1 << 7), /**< By default integer index property keys are
                                                               *   converted to string. Enabling this flags keeps
                                                               *   integer index property keys as numbers. */
} jerry_property_filter_t;

/**
 * String encoding.
 */
typedef enum
{
  JERRY_ENCODING_CESU8, /**< cesu-8 encoding */
  JERRY_ENCODING_UTF8, /**< utf-8 encoding */
} jerry_encoding_t;

/**
 * Description of JerryScript heap memory stats.
 * It is for memory profiling.
 */
typedef struct
{
  size_t version; /**< the version of the stats struct */
  size_t size; /**< heap total size */
  size_t allocated_bytes; /**< currently allocated bytes */
  size_t peak_allocated_bytes; /**< peak allocated bytes */
  size_t reserved[4]; /**< padding for future extensions */
} jerry_heap_stats_t;

/**
 * Call related information passed to jerry_external_handler_t.
 */
typedef struct jerry_call_info_t
{
  jerry_value_t function; /**< invoked function object */
  jerry_value_t this_value; /**< this value passed to the function  */
  jerry_value_t new_target; /**< current new target value, undefined for non-constructor calls */
} jerry_call_info_t;

/**
 * Type of an external function handler.
 */
typedef jerry_value_t (*jerry_external_handler_t) (const jerry_call_info_t *call_info_p,
                                                   const jerry_value_t args_p[],
                                                   const jerry_length_t args_count);

/**
 * Native free callback of generic value types.
 */
typedef void (*jerry_value_free_cb_t) (void *native_p);

/**
 * Forward definition of jerry_object_native_info_t.
 */
struct jerry_object_native_info_t;

/**
 * Native free callback of an object.
 */
typedef void (*jerry_object_native_free_cb_t) (void *native_p, struct jerry_object_native_info_t *info_p);

/**
 * Free callback for external strings.
 */
typedef void (*jerry_external_string_free_cb_t) (jerry_char_t *string_p, jerry_size_t string_size, void *user_p);

/**
 * Decorator callback for Error objects. The decorator can create
 * or update any properties of the newly created Error object.
 */
typedef void (*jerry_error_object_created_cb_t) (const jerry_value_t error_object, void *user_p);

/**
 * Callback which tells whether the ECMAScript execution should be stopped.
 *
 * As long as the function returns with undefined the execution continues.
 * When a non-undefined value is returned the execution stops and the value
 * is thrown by the engine as an exception.
 *
 * Note: if the function returns with a non-undefined value it
 *       must return with the same value for future calls.
 */
typedef jerry_value_t (*jerry_halt_cb_t) (void *user_p);

/**
 * Callback function which is called when an exception is thrown in an ECMAScript code.
 * The callback should not change the exception_value. The callback is not called again
 * until the value is caught.
 *
 * Note: the engine considers exceptions thrown by external functions as never caught.
 */
typedef void (*jerry_throw_cb_t) (const jerry_value_t exception_value, void *user_p);

/**
 * Function type applied to each unit of encoding when iterating over a string.
 */
typedef void (*jerry_string_iterate_cb_t) (uint32_t value, void *user_p);

/**
 * Function type applied for each data property of an object.
 */
typedef bool (*jerry_object_property_foreach_cb_t) (const jerry_value_t property_name,
                                                    const jerry_value_t property_value,
                                                    void *user_data_p);

/**
 * Function type applied for each object in the engine.
 */
typedef bool (*jerry_foreach_live_object_cb_t) (const jerry_value_t object, void *user_data_p);

/**
 * Function type applied for each matching object in the engine.
 */
typedef bool (*jerry_foreach_live_object_with_info_cb_t) (const jerry_value_t object,
                                                          void *object_data_p,
                                                          void *user_data_p);

/**
 * User context item manager
 */
typedef struct
{
  /**
   * Callback responsible for initializing a context item, or NULL to zero out the memory. This is called lazily, the
   * first time jerry_context_data () is called with this manager.
   *
   * @param [in] data The buffer that JerryScript allocated for the manager. The buffer is zeroed out. The size is
   * determined by the bytes_needed field. The buffer is kept alive until jerry_cleanup () is called.
   */
  void (*init_cb) (void *data);

  /**
   * Callback responsible for deinitializing a context item, or NULL. This is called as part of jerry_cleanup (),
   * right *before* the VM has been cleaned up. This is a good place to release strong references to jerry_value_t's
   * that the manager may be holding.
   * Note: because the VM has not been fully cleaned up yet, jerry_object_native_info_t free_cb's can still get called
   * *after* all deinit_cb's have been run. See finalize_cb for a callback that is guaranteed to run *after* all
   * free_cb's have been run.
   *
   * @param [in] data The buffer that JerryScript allocated for the manager.
   */
  void (*deinit_cb) (void *data);

  /**
   * Callback responsible for finalizing a context item, or NULL. This is called as part of jerry_cleanup (),
   * right *after* the VM has been cleaned up and destroyed and jerry_... APIs cannot be called any more. At this point,
   * all values in the VM have been cleaned up. This is a good place to clean up native state that can only be cleaned
   * up at the very end when there are no more VM values around that may need to access that state.
   *
   * @param [in] data The buffer that JerryScript allocated for the manager. After returning from this callback,
   * the data pointer may no longer be used.
   */
  void (*finalize_cb) (void *data);

  /**
   * Number of bytes to allocate for this manager. This is the size of the buffer that JerryScript will allocate on
   * behalf of the manager. The pointer to this buffer is passed into init_cb, deinit_cb and finalize_cb. It is also
   * returned from the jerry_context_data () API.
   */
  size_t bytes_needed;
} jerry_context_data_manager_t;

/**
 * Function type for allocating buffer for JerryScript context.
 */
typedef void *(*jerry_context_alloc_cb_t) (size_t size, void *cb_data_p);

/**
 * Type information of a native pointer.
 */
typedef struct jerry_object_native_info_t
{
  jerry_object_native_free_cb_t free_cb; /**< the free callback of the native pointer */
  uint16_t number_of_references; /**< the number of value references which are marked by the garbage collector */
  uint16_t offset_of_references; /**< byte offset indicating the start offset of value
                                  *   references in the user allocated buffer */
} jerry_object_native_info_t;

/**
 * An opaque declaration of the JerryScript context structure.
 */
typedef struct jerry_context_t jerry_context_t;

/**
 * Enum that contains the supported binary operation types
 */
typedef enum
{
  JERRY_BIN_OP_EQUAL = 0u, /**< equal comparison (==) */
  JERRY_BIN_OP_STRICT_EQUAL, /**< strict equal comparison (===) */
  JERRY_BIN_OP_LESS, /**< less relation (<) */
  JERRY_BIN_OP_LESS_EQUAL, /**< less or equal relation (<=) */
  JERRY_BIN_OP_GREATER, /**< greater relation (>) */
  JERRY_BIN_OP_GREATER_EQUAL, /**< greater or equal relation (>=)*/
  JERRY_BIN_OP_INSTANCEOF, /**< instanceof operation */
  JERRY_BIN_OP_ADD, /**< addition operator (+) */
  JERRY_BIN_OP_SUB, /**< subtraction operator (-) */
  JERRY_BIN_OP_MUL, /**< multiplication operator (*) */
  JERRY_BIN_OP_DIV, /**< division operator (/) */
  JERRY_BIN_OP_REM, /**< remainder operator (%) */
} jerry_binary_op_t;

/**
 * Backtrace related types.
 */

/**
 * List of backtrace frame types returned by jerry_frame_type.
 */
typedef enum
{
  JERRY_BACKTRACE_FRAME_JS, /**< indicates that the frame is created for a JavaScript function/method */
} jerry_frame_type_t;

/**
 * Location info retrieved by jerry_frame_location.
 */
typedef struct
{
  jerry_value_t source_name; /**< source name */
  jerry_size_t line; /**< line index */
  jerry_size_t column; /**< column index */
} jerry_frame_location_t;

/*
 * Internal data structure for jerry_frame_t definition.
 */
struct jerry_frame_internal_t;

/**
 * Backtrace frame data passed to the jerry_backtrace_cb_t handler.
 */
typedef struct jerry_frame_internal_t jerry_frame_t;

/**
 * Callback function which is called by jerry_backtrace for each stack frame.
 */
typedef bool (*jerry_backtrace_cb_t) (jerry_frame_t *frame_p, void *user_p);

/**
 * Detailed value type related types.
 */

/**
 * JerryScript API value type information.
 */
typedef enum
{
  JERRY_TYPE_NONE = 0u, /**< no type information */
  JERRY_TYPE_UNDEFINED, /**< undefined type */
  JERRY_TYPE_NULL, /**< null type */
  JERRY_TYPE_BOOLEAN, /**< boolean type */
  JERRY_TYPE_NUMBER, /**< number type */
  JERRY_TYPE_STRING, /**< string type */
  JERRY_TYPE_OBJECT, /**< object type */
  JERRY_TYPE_FUNCTION, /**< function type */
  JERRY_TYPE_EXCEPTION, /**< exception/abort type */
  JERRY_TYPE_SYMBOL, /**< symbol type */
  JERRY_TYPE_BIGINT, /**< bigint type */
} jerry_type_t;

/**
 * JerryScript object type information.
 */
typedef enum
{
  JERRY_OBJECT_TYPE_NONE = 0u, /**< Non object type */
  JERRY_OBJECT_TYPE_GENERIC, /**< Generic JavaScript object without any internal property */
  JERRY_OBJECT_TYPE_MODULE_NAMESPACE, /**< Namespace object */
  JERRY_OBJECT_TYPE_ARRAY, /**< Array object */
  JERRY_OBJECT_TYPE_PROXY, /**< Proxy object */
  JERRY_OBJECT_TYPE_SCRIPT, /**< Script object (see jerry_parse) */
  JERRY_OBJECT_TYPE_MODULE, /**< Module object (see jerry_parse) */
  JERRY_OBJECT_TYPE_PROMISE, /**< Promise object */
  JERRY_OBJECT_TYPE_DATAVIEW, /**< Dataview object */
  JERRY_OBJECT_TYPE_FUNCTION, /**< Function object (see jerry_function_type) */
  JERRY_OBJECT_TYPE_TYPEDARRAY, /**< %TypedArray% object (see jerry_typedarray_type) */
  JERRY_OBJECT_TYPE_ITERATOR, /**< Iterator object (see jerry_iterator_type) */
  JERRY_OBJECT_TYPE_CONTAINER, /**< Container object (see jerry_container_get_type) */
  JERRY_OBJECT_TYPE_ERROR, /**< Error object */
  JERRY_OBJECT_TYPE_ARRAYBUFFER, /**< Array buffer object */
  JERRY_OBJECT_TYPE_SHARED_ARRAY_BUFFER, /**< Shared Array Buffer object */

  JERRY_OBJECT_TYPE_ARGUMENTS, /**< Arguments object */
  JERRY_OBJECT_TYPE_BOOLEAN, /**< Boolean object */
  JERRY_OBJECT_TYPE_DATE, /**< Date object */
  JERRY_OBJECT_TYPE_NUMBER, /**< Number object */
  JERRY_OBJECT_TYPE_REGEXP, /**< RegExp object */
  JERRY_OBJECT_TYPE_STRING, /**< String object */
  JERRY_OBJECT_TYPE_SYMBOL, /**< Symbol object */
  JERRY_OBJECT_TYPE_GENERATOR, /**< Generator object */
  JERRY_OBJECT_TYPE_BIGINT, /**< BigInt object */
  JERRY_OBJECT_TYPE_WEAKREF, /**< WeakRef object */
} jerry_object_type_t;

/**
 * JerryScript function object type information.
 */
typedef enum
{
  JERRY_FUNCTION_TYPE_NONE = 0u, /**< Non function type */
  JERRY_FUNCTION_TYPE_GENERIC, /**< Generic JavaScript function */
  JERRY_FUNCTION_TYPE_ACCESSOR, /**< Accessor function */
  JERRY_FUNCTION_TYPE_BOUND, /**< Bound function */
  JERRY_FUNCTION_TYPE_ARROW, /**< Arrow function */
  JERRY_FUNCTION_TYPE_GENERATOR, /**< Generator function */
} jerry_function_type_t;

/**
 * JerryScript iterator object type information.
 */
typedef enum
{
  JERRY_ITERATOR_TYPE_NONE = 0u, /**< Non iterator type */
  JERRY_ITERATOR_TYPE_ARRAY, /**< Array iterator */
  JERRY_ITERATOR_TYPE_STRING, /**< String iterator */
  JERRY_ITERATOR_TYPE_MAP, /**< Map iterator */
  JERRY_ITERATOR_TYPE_SET, /**< Set iterator */
} jerry_iterator_type_t;

/**
 * Module related types.
 */

/**
 * An enum representing the current status of a module
 */
typedef enum
{
  JERRY_MODULE_STATE_INVALID = 0, /**< return value for jerry_module_state when its argument is not a module */
  JERRY_MODULE_STATE_UNLINKED = 1, /**< module is currently unlinked */
  JERRY_MODULE_STATE_LINKING = 2, /**< module is currently being linked */
  JERRY_MODULE_STATE_LINKED = 3, /**< module has been linked (its dependencies has been resolved) */
  JERRY_MODULE_STATE_EVALUATING = 4, /**< module is currently being evaluated */
  JERRY_MODULE_STATE_EVALUATED = 5, /**< module has been evaluated (its source code has been executed) */
  JERRY_MODULE_STATE_ERROR = 6, /**< an error has been encountered before the evaluated state is reached */
} jerry_module_state_t;

/**
 * Callback which is called by jerry_module_link to get the referenced module.
 */
typedef jerry_value_t (*jerry_module_resolve_cb_t) (const jerry_value_t specifier,
                                                    const jerry_value_t referrer,
                                                    void *user_p);

/**
 * Callback which is called when an import is resolved dynamically to get the referenced module.
 */
typedef jerry_value_t (*jerry_module_import_cb_t) (const jerry_value_t specifier,
                                                   const jerry_value_t user_value,
                                                   void *user_p);

/**
 * Callback which is called after the module enters into linked, evaluated or error state.
 */
typedef void (*jerry_module_state_changed_cb_t) (jerry_module_state_t new_state,
                                                 const jerry_value_t module,
                                                 const jerry_value_t value,
                                                 void *user_p);

/**
 * Callback which is called when an import.meta expression of a module is evaluated the first time.
 */
typedef void (*jerry_module_import_meta_cb_t) (const jerry_value_t module,
                                               const jerry_value_t meta_object,
                                               void *user_p);

/**
 * Callback which is called by jerry_module_evaluate to evaluate the native module.
 */
typedef jerry_value_t (*jerry_native_module_evaluate_cb_t) (const jerry_value_t native_module);

/**
 * Proxy related types.
 */

/**
 * JerryScript special Proxy object options.
 */
typedef enum
{
  JERRY_PROXY_SKIP_RESULT_VALIDATION = (1u << 0), /**< skip result validation for [[GetPrototypeOf]],
                                                   *   [[SetPrototypeOf]], [[IsExtensible]],
                                                   *   [[PreventExtensions]], [[GetOwnProperty]],
                                                   *   [[DefineOwnProperty]], [[HasProperty]], [[Get]],
                                                   *   [[Set]], [[Delete]] and [[OwnPropertyKeys]] */
} jerry_proxy_custom_behavior_t;

/**
 * Promise related types.
 */

/**
 * Enum values representing various Promise states.
 */
typedef enum
{
  JERRY_PROMISE_STATE_NONE = 0u, /**< Invalid/Unknown state (possibly called on a non-promise object). */
  JERRY_PROMISE_STATE_PENDING, /**< Promise is in "Pending" state. */
  JERRY_PROMISE_STATE_FULFILLED, /**< Promise is in "Fulfilled" state. */
  JERRY_PROMISE_STATE_REJECTED, /**< Promise is in "Rejected" state. */
} jerry_promise_state_t;

/**
 * Event types for jerry_promise_event_cb_t callback function.
 * The description of the 'object' and 'value' arguments are provided for each type.
 */
typedef enum
{
  JERRY_PROMISE_EVENT_CREATE = 0u, /**< a new Promise object is created
                                    *   object: the new Promise object
                                    *   value: parent Promise for `then` chains, undefined otherwise */
  JERRY_PROMISE_EVENT_RESOLVE, /**< called when a Promise is about to be resolved
                                *   object: the Promise object
                                *   value: value for resolving */
  JERRY_PROMISE_EVENT_REJECT, /**< called when a Promise is about to be rejected
                               *   object: the Promise object
                               *   value: value for rejecting */
  JERRY_PROMISE_EVENT_RESOLVE_FULFILLED, /**< called when a resolve is called on a fulfilled Promise
                                          *   object: the Promise object
                                          *   value: value for resolving */
  JERRY_PROMISE_EVENT_REJECT_FULFILLED, /**< called when a reject is called on a fulfilled Promise
                                         *  object: the Promise object
                                         *  value: value for rejecting */
  JERRY_PROMISE_EVENT_REJECT_WITHOUT_HANDLER, /**< called when a Promise is rejected without a handler
                                               *   object: the Promise object
                                               *   value: value for rejecting */
  JERRY_PROMISE_EVENT_CATCH_HANDLER_ADDED, /**< called when a catch handler is added to a rejected
                                            *   Promise which did not have a catch handler before
                                            *   object: the Promise object
                                            *   value: undefined */
  JERRY_PROMISE_EVENT_BEFORE_REACTION_JOB, /**< called before executing a Promise reaction job
                                            *   object: the Promise object
                                            *   value: undefined */
  JERRY_PROMISE_EVENT_AFTER_REACTION_JOB, /**< called after a Promise reaction job is completed
                                           *   object: the Promise object
                                           *   value: undefined */
  JERRY_PROMISE_EVENT_ASYNC_AWAIT, /**< called when an async function awaits the result of a Promise object
                                    *   object: internal object representing the execution status
                                    *   value: the Promise object */
  JERRY_PROMISE_EVENT_ASYNC_BEFORE_RESOLVE, /**< called when an async function is continued with resolve
                                             *   object: internal object representing the execution status
                                             *   value: value for resolving */
  JERRY_PROMISE_EVENT_ASYNC_BEFORE_REJECT, /**< called when an async function is continued with reject
                                            *   object: internal object representing the execution status
                                            *   value: value for rejecting */
  JERRY_PROMISE_EVENT_ASYNC_AFTER_RESOLVE, /**< called when an async function resolve is completed
                                            *   object: internal object representing the execution status
                                            *   value: value for resolving */
  JERRY_PROMISE_EVENT_ASYNC_AFTER_REJECT, /**< called when an async function reject is completed
                                           *   object: internal object representing the execution status
                                           *   value: value for rejecting */
} jerry_promise_event_type_t;

/**
 * Filter types for jerry_promise_on_event callback function.
 * The callback is only called for those events which are enabled by the filters.
 */
typedef enum
{
  JERRY_PROMISE_EVENT_FILTER_DISABLE = 0, /**< disable reporting of all events */
  JERRY_PROMISE_EVENT_FILTER_CREATE = (1 << 0), /**< enables the following event:
                                                 *   JERRY_PROMISE_EVENT_CREATE */
  JERRY_PROMISE_EVENT_FILTER_RESOLVE = (1 << 1), /**< enables the following event:
                                                  *   JERRY_PROMISE_EVENT_RESOLVE */
  JERRY_PROMISE_EVENT_FILTER_REJECT = (1 << 2), /**< enables the following event:
                                                 *   JERRY_PROMISE_EVENT_REJECT */
  JERRY_PROMISE_EVENT_FILTER_ERROR = (1 << 3), /**< enables the following events:
                                                *   JERRY_PROMISE_EVENT_RESOLVE_FULFILLED
                                                *   JERRY_PROMISE_EVENT_REJECT_FULFILLED
                                                *   JERRY_PROMISE_EVENT_REJECT_WITHOUT_HANDLER
                                                *   JERRY_PROMISE_EVENT_CATCH_HANDLER_ADDED */
  JERRY_PROMISE_EVENT_FILTER_REACTION_JOB = (1 << 4), /**< enables the following events:
                                                       *   JERRY_PROMISE_EVENT_BEFORE_REACTION_JOB
                                                       *   JERRY_PROMISE_EVENT_AFTER_REACTION_JOB */
  JERRY_PROMISE_EVENT_FILTER_ASYNC_MAIN = (1 << 5), /**< enables the following event:
                                                     *   JERRY_PROMISE_EVENT_ASYNC_AWAIT */
  JERRY_PROMISE_EVENT_FILTER_ASYNC_REACTION_JOB = (1 << 6), /**< enables the following events:
                                                             *   JERRY_PROMISE_EVENT_ASYNC_BEFORE_RESOLVE
                                                             *   JERRY_PROMISE_EVENT_ASYNC_BEFORE_REJECT
                                                             *   JERRY_PROMISE_EVENT_ASYNC_AFTER_RESOLVE
                                                             *   JERRY_PROMISE_EVENT_ASYNC_AFTER_REJECT */
} jerry_promise_event_filter_t;

/**
 * Notification callback for tracking Promise and async function operations.
 */
typedef void (*jerry_promise_event_cb_t) (jerry_promise_event_type_t event_type,
                                          const jerry_value_t object,
                                          const jerry_value_t value,
                                          void *user_p);

/**
 * Symbol related types.
 */

/**
 * List of well-known symbols.
 */
typedef enum
{
  JERRY_SYMBOL_ASYNC_ITERATOR, /**< @@asyncIterator well-known symbol */
  JERRY_SYMBOL_HAS_INSTANCE, /**< @@hasInstance well-known symbol */
  JERRY_SYMBOL_IS_CONCAT_SPREADABLE, /**< @@isConcatSpreadable well-known symbol */
  JERRY_SYMBOL_ITERATOR, /**< @@iterator well-known symbol */
  JERRY_SYMBOL_MATCH, /**< @@match well-known symbol */
  JERRY_SYMBOL_REPLACE, /**< @@replace well-known symbol */
  JERRY_SYMBOL_SEARCH, /**< @@search well-known symbol */
  JERRY_SYMBOL_SPECIES, /**< @@species well-known symbol */
  JERRY_SYMBOL_SPLIT, /**< @@split well-known symbol */
  JERRY_SYMBOL_TO_PRIMITIVE, /**< @@toPrimitive well-known symbol */
  JERRY_SYMBOL_TO_STRING_TAG, /**< @@toStringTag well-known symbol */
  JERRY_SYMBOL_UNSCOPABLES, /**< @@unscopables well-known symbol */
  JERRY_SYMBOL_MATCH_ALL, /**< @@matchAll well-known symbol */
} jerry_well_known_symbol_t;

/**
 * TypedArray related types.
 */

/**
 * TypedArray types.
 */
typedef enum
{
  JERRY_TYPEDARRAY_INVALID = 0,
  JERRY_TYPEDARRAY_UINT8,
  JERRY_TYPEDARRAY_UINT8CLAMPED,
  JERRY_TYPEDARRAY_INT8,
  JERRY_TYPEDARRAY_UINT16,
  JERRY_TYPEDARRAY_INT16,
  JERRY_TYPEDARRAY_UINT32,
  JERRY_TYPEDARRAY_INT32,
  JERRY_TYPEDARRAY_FLOAT32,
  JERRY_TYPEDARRAY_FLOAT64,
  JERRY_TYPEDARRAY_BIGINT64,
  JERRY_TYPEDARRAY_BIGUINT64,
} jerry_typedarray_type_t;

/**
 * Container types.
 */
typedef enum
{
  JERRY_CONTAINER_TYPE_INVALID = 0, /**< Invalid container */
  JERRY_CONTAINER_TYPE_MAP, /**< Map type */
  JERRY_CONTAINER_TYPE_SET, /**< Set type */
  JERRY_CONTAINER_TYPE_WEAKMAP, /**< WeakMap type */
  JERRY_CONTAINER_TYPE_WEAKSET, /**< WeakSet type */
} jerry_container_type_t;

/**
 * Container operations
 */
typedef enum
{
  JERRY_CONTAINER_OP_ADD, /**< Set/WeakSet add operation */
  JERRY_CONTAINER_OP_GET, /**< Map/WeakMap get operation */
  JERRY_CONTAINER_OP_SET, /**< Map/WeakMap set operation */
  JERRY_CONTAINER_OP_HAS, /**< Set/WeakSet/Map/WeakMap has operation */
  JERRY_CONTAINER_OP_DELETE, /**< Set/WeakSet/Map/WeakMap delete operation */
  JERRY_CONTAINER_OP_SIZE, /**< Set/WeakSet/Map/WeakMap size operation */
  JERRY_CONTAINER_OP_CLEAR, /**< Set/Map clear operation */
} jerry_container_op_t;

/**
 * Miscellaneous types.
 */

/**
 * Enabled fields of jerry_source_info_t.
 */
typedef enum
{
  JERRY_SOURCE_INFO_HAS_SOURCE_CODE = (1 << 0), /**< source_code field is valid */
  JERRY_SOURCE_INFO_HAS_FUNCTION_ARGUMENTS = (1 << 1), /**< function_arguments field is valid */
  JERRY_SOURCE_INFO_HAS_SOURCE_RANGE = (1 << 2), /**< both source_range_start and source_range_length
                                                  *   fields are valid */
} jerry_source_info_enabled_fields_t;

/**
 * Source related information of a script/module/function.
 */
typedef struct
{
  uint32_t enabled_fields; /**< combination of jerry_source_info_enabled_fields_t values */
  jerry_value_t source_code; /**< script source code or function body */
  jerry_value_t function_arguments; /**< function arguments */
  uint32_t source_range_start; /**< start position of the function in the source code */
  uint32_t source_range_length; /**< source length of the function in the source code */
} jerry_source_info_t;

/**
 * Array buffer types.
 */

/**
 * Type of an array buffer.
 */
typedef enum
{
  JERRY_ARRAYBUFFER_TYPE_ARRAYBUFFER, /**< the object is an array buffer object */
  JERRY_ARRAYBUFFER_TYPE_SHARED_ARRAYBUFFER, /**< the object is a shared array buffer object */
} jerry_arraybuffer_type_t;

/**
 * Callback for allocating the backing store of array buffer or shared array buffer objects.
 */
typedef uint8_t *(*jerry_arraybuffer_allocate_cb_t) (jerry_arraybuffer_type_t buffer_type,
                                                     uint32_t buffer_size,
                                                     void **arraybuffer_user_p,
                                                     void *user_p);

/**
 * Callback for freeing the backing store of array buffer or shared array buffer objects.
 */
typedef void (*jerry_arraybuffer_free_cb_t) (jerry_arraybuffer_type_t buffer_type,
                                             uint8_t *buffer_p,
                                             uint32_t buffer_size,
                                             void *arraybuffer_user_p,
                                             void *user_p);

/**
 * @}
 */

JERRY_C_API_END

#endif /* !JERRYSCRIPT_TYPES_H */

JERRY_C_API_BEGIN

/**
 * @defgroup jerry-api JerryScript public API
 * @{
 */

/**
 * @defgroup jerry-api-general General functions
 * @{
 */

/**
 * @defgroup jerry-api-general-conext Context management
 * @{
 */
void jerry_init (jerry_init_flag_t flags);
void jerry_cleanup (void);

void *jerry_context_data (const jerry_context_data_manager_t *manager_p);

jerry_value_t jerry_current_realm (void);
jerry_value_t jerry_set_realm (jerry_value_t realm);
/**
 * jerry-api-general-conext @}
 */

/**
 * @defgroup jerry-api-general-heap Heap management
 * @{
 */
void *jerry_heap_alloc (jerry_size_t size);
void jerry_heap_free (void *mem_p, jerry_size_t size);

bool jerry_heap_stats (jerry_heap_stats_t *out_stats_p);
void jerry_heap_gc (jerry_gc_mode_t mode);

bool jerry_foreach_live_object (jerry_foreach_live_object_cb_t callback, void *user_data);
bool jerry_foreach_live_object_with_info (const jerry_object_native_info_t *native_info_p,
                                          jerry_foreach_live_object_with_info_cb_t callback,
                                          void *user_data_p);
/**
 * jerry-api-general-heap @}
 */

/**
 * @defgroup jerry-api-general-misc Miscellaneous
 * @{
 */

void JERRY_ATTR_FORMAT (printf, 2, 3) jerry_log (jerry_log_level_t level, const char *format_p, ...);
void jerry_log_set_level (jerry_log_level_t level);
bool jerry_validate_string (const jerry_char_t *buffer_p, jerry_size_t buffer_size, jerry_encoding_t encoding);
bool JERRY_ATTR_CONST jerry_feature_enabled (const jerry_feature_t feature);
void jerry_register_magic_strings (const jerry_char_t *const *ext_strings_p,
                                   uint32_t count,
                                   const jerry_length_t *str_lengths_p);
/**
 * jerry-api-general-misc @}
 */

/**
 * jerry-api-general @}
 */

/**
 * @defgroup jerry-api-code Scripts and Executables
 * @{
 */

/**
 * @defgroup jerry-api-code-parse Parsing
 * @{
 */
jerry_value_t jerry_parse (const jerry_char_t *source_p, size_t source_size, const jerry_parse_options_t *options_p);
jerry_value_t jerry_parse_value (const jerry_value_t source, const jerry_parse_options_t *options_p);
/**
 * jerry-api-code-parse @}
 */

/**
 * @defgroup jerry-api-code-exec Execution
 * @{
 */
jerry_value_t jerry_eval (const jerry_char_t *source_p, size_t source_size, uint32_t flags);
jerry_value_t jerry_run (const jerry_value_t script);
jerry_value_t jerry_run_jobs (void);
/**
 * jerry-api-code-exec @}
 */

/**
 * @defgroup jerry-api-code-sourceinfo Source information
 * @{
 */
jerry_value_t jerry_source_name (const jerry_value_t value);
jerry_value_t jerry_source_user_value (const jerry_value_t value);
jerry_source_info_t *jerry_source_info (const jerry_value_t value);
void jerry_source_info_free (jerry_source_info_t *source_info_p);
/**
 * jerry-api-code-sourceinfo @}
 */

/**
 * @defgroup jerry-api-code-cb Callbacks
 * @{
 */
void jerry_halt_handler (uint32_t interval, jerry_halt_cb_t callback, void *user_p);
/**
 * jerry-api-code-cb @}
 */

/**
 * jerry-api-code @}
 */

/**
 * @defgroup jerry-api-backtrace Backtraces
 * @{
 */

/**
 * @defgroup jerry-api-backtrace-capture Capturing
 * @{
 */
jerry_value_t jerry_backtrace (uint32_t max_depth);
void jerry_backtrace_capture (jerry_backtrace_cb_t callback, void *user_p);
/**
 * jerry-api-backtrace-capture @}
 */

/**
 * @defgroup jerry-api-backtrace-frame Frames
 * @{
 */
jerry_frame_type_t jerry_frame_type (const jerry_frame_t *frame_p);
const jerry_value_t *jerry_frame_callee (jerry_frame_t *frame_p);
const jerry_value_t *jerry_frame_this (jerry_frame_t *frame_p);
const jerry_frame_location_t *jerry_frame_location (jerry_frame_t *frame_p);
bool jerry_frame_is_strict (jerry_frame_t *frame_p);
/**
 * jerry-api-backtrace-frame @}
 */

/**
 * jerry-api-backtrace @}
 */

/**
 * @defgroup jerry-api-value Values
 * @{
 */

/* Reference management */
jerry_value_t JERRY_ATTR_WARN_UNUSED_RESULT jerry_value_copy (const jerry_value_t value);
void jerry_value_free (jerry_value_t value);

/**
 * @defgroup jerry-api-value-checks Type inspection
 * @{
 */
jerry_type_t jerry_value_type (const jerry_value_t value);
bool jerry_value_is_exception (const jerry_value_t value);
bool jerry_value_is_abort (const jerry_value_t value);

bool jerry_value_is_undefined (const jerry_value_t value);
bool jerry_value_is_null (const jerry_value_t value);
bool jerry_value_is_boolean (const jerry_value_t value);
bool jerry_value_is_true (const jerry_value_t value);
bool jerry_value_is_false (const jerry_value_t value);

bool jerry_value_is_number (const jerry_value_t value);
bool jerry_value_is_bigint (const jerry_value_t value);

bool jerry_value_is_string (const jerry_value_t value);
bool jerry_value_is_symbol (const jerry_value_t value);

bool jerry_value_is_object (const jerry_value_t value);
bool jerry_value_is_array (const jerry_value_t value);
bool jerry_value_is_promise (const jerry_value_t value);
bool jerry_value_is_proxy (const jerry_value_t value);
bool jerry_value_is_arraybuffer (const jerry_value_t value);
bool jerry_value_is_shared_arraybuffer (const jerry_value_t value);
bool jerry_value_is_dataview (const jerry_value_t value);
bool jerry_value_is_typedarray (const jerry_value_t value);

bool jerry_value_is_constructor (const jerry_value_t value);
bool jerry_value_is_function (const jerry_value_t value);
bool jerry_value_is_async_function (const jerry_value_t value);

bool jerry_value_is_error (const jerry_value_t value);
/**
 * jerry-api-value-checks @}
 */

/**
 * @defgroup jerry-api-value-coerce Coercion
 * @{
 */
bool jerry_value_to_boolean (const jerry_value_t value);
jerry_value_t jerry_value_to_number (const jerry_value_t value);
jerry_value_t jerry_value_to_object (const jerry_value_t value);
jerry_value_t jerry_value_to_primitive (const jerry_value_t value);
jerry_value_t jerry_value_to_string (const jerry_value_t value);
jerry_value_t jerry_value_to_bigint (const jerry_value_t value);

double jerry_value_as_number (const jerry_value_t value);
double jerry_value_as_integer (const jerry_value_t value);
int32_t jerry_value_as_int32 (const jerry_value_t value);
uint32_t jerry_value_as_uint32 (const jerry_value_t value);
/**
 * jerry-api-value-coerce @}
 */

/**
 * @defgroup jerry-api-value-op Operations
 * @{
 */
jerry_value_t jerry_binary_op (jerry_binary_op_t operation, const jerry_value_t lhs, const jerry_value_t rhs);

/**
 * jerry-api-value-op @}
 */

/**
 * jerry-api-value @}
 */

/**
 * @defgroup jerry-api-exception Exceptions
 * @{
 */

/**
 * @defgroup jerry-api-exception-ctor Constructors
 * @{
 */
jerry_value_t jerry_throw (jerry_error_t type, const jerry_value_t message);
jerry_value_t jerry_throw_sz (jerry_error_t type, const char *message_p);
jerry_value_t jerry_throw_value (jerry_value_t value, bool take_ownership);
jerry_value_t jerry_throw_abort (jerry_value_t value, bool take_ownership);
/**
 * jerry-api-exception-ctor @}
 */

/**
 * @defgroup jerry-api-exception-op Operations
 * @{
 */
void jerry_exception_allow_capture (jerry_value_t value, bool allow_capture);
/**
 * jerry-api-exception-op @}
 */

/**
 * @defgroup jerry-api-exception-get Getters
 * @{
 */
jerry_value_t jerry_exception_value (jerry_value_t value, bool free_exception);
bool jerry_exception_is_captured (const jerry_value_t value);
/**
 * jerry-api-exception-get @}
 */

/**
 * @defgroup jerry-api-exception-cb Callbacks
 * @{
 */
void jerry_on_throw (jerry_throw_cb_t callback, void *user_p);
/**
 * jerry-api-exception-cb @}
 */

/**
 * jerry-api-error @}
 */

/**
 * @defgroup jerry-api-primitives Primitive types
 * @{
 */

/**
 * @defgroup jerry-api-undefined Undefined
 * @{
 */

/**
 * @defgroup jerry-api-undefined-ctor Constructors
 * @{
 */

jerry_value_t JERRY_ATTR_CONST jerry_undefined (void);

/**
 * jerry-api-undefined-ctor @}
 */

/**
 * jerry-api-undefined @}
 */

/**
 * @defgroup jerry-api-null Null
 * @{
 */

/**
 * @defgroup jerry-api-null-ctor Constructors
 * @{
 */

jerry_value_t JERRY_ATTR_CONST jerry_null (void);

/**
 * jerry-api-null-ctor @}
 */

/**
 * jerry-api-null @}
 */

/**
 * @defgroup jerry-api-boolean Boolean
 * @{
 */

/**
 * @defgroup jerry-api-boolean-ctor Constructors
 * @{
 */

jerry_value_t JERRY_ATTR_CONST jerry_boolean (bool value);

/**
 * jerry-api-boolean-ctor @}
 */

/**
 * jerry-api-boolean @}
 */

/**
 * @defgroup jerry-api-number Number
 * @{
 */

/**
 * @defgroup jerry-api-number-ctor Number
 * @{
 */

jerry_value_t jerry_number (double value);
jerry_value_t jerry_infinity (bool sign);
jerry_value_t jerry_nan (void);

/**
 * jerry-api-number-ctor @}
 */

/**
 * jerry-api-number @}
 */

/**
 * @defgroup jerry-api-bigint BigInt
 * @{
 */

/**
 * @defgroup jerry-api-bigint-ctor Constructors
 * @{
 */
jerry_value_t jerry_bigint (const uint64_t *digits_p, uint32_t digit_count, bool sign);
/**
 * jerry-api-bigint-ctor @}
 */

/**
 * @defgroup jerry-api-bigint-get Getters
 * @{
 */
uint32_t jerry_bigint_digit_count (const jerry_value_t value);
/**
 * jerry-api-bigint-get @}
 */

/**
 * @defgroup jerry-api-bigint-op Operations
 * @{
 */
void jerry_bigint_to_digits (const jerry_value_t value, uint64_t *digits_p, uint32_t digit_count, bool *sign_p);
/**
 * jerry-api-bigint-get @}
 */

/**
 * jerry-api-bigint @}
 */

/**
 * @defgroup jerry-api-string String
 * @{
 */

/**
 * @defgroup jerry-api-string-ctor Constructors
 * @{
 */
jerry_value_t jerry_string (const jerry_char_t *buffer_p, jerry_size_t buffer_size, jerry_encoding_t encoding);
jerry_value_t jerry_string_sz (const char *str_p);
jerry_value_t jerry_string_external (const jerry_char_t *buffer_p, jerry_size_t buffer_size, void *user_p);
jerry_value_t jerry_string_external_sz (const char *str_p, void *user_p);
/**
 * jerry-api-string-cotr @}
 */

/**
 * @defgroup jerry-api-string-get Getters
 * @{
 */
jerry_size_t jerry_string_size (const jerry_value_t value, jerry_encoding_t encoding);
jerry_length_t jerry_string_length (const jerry_value_t value);
void *jerry_string_user_ptr (const jerry_value_t value, bool *is_external);
/**
 * jerry-api-string-get @}
 */

/**
 * @defgroup jerry-api-string-op Operations
 * @{
 */
jerry_size_t jerry_string_substr (const jerry_value_t value, jerry_length_t start, jerry_length_t end);
jerry_size_t jerry_string_to_buffer (const jerry_value_t value,
                                     jerry_encoding_t encoding,
                                     jerry_char_t *buffer_p,
                                     jerry_size_t buffer_size);
void jerry_string_iterate (const jerry_value_t value,
                           jerry_encoding_t encoding,
                           jerry_string_iterate_cb_t callback,
                           void *user_p);
/**
 * jerry-api-string-op @}
 */

/**
 * @defgroup jerry-api-string-cb Callbacks
 * @{
 */
void jerry_string_external_on_free (jerry_external_string_free_cb_t callback);
/**
 * jerry-api-string-cb @}
 */

/**
 * jerry-api-string @}
 */

/**
 * @defgroup jerry-api-symbol Symbol
 * @{
 */

/**
 * @defgroup jerry-api-symbol-ctor Constructors
 * @{
 */
jerry_value_t jerry_symbol (jerry_well_known_symbol_t symbol);
jerry_value_t jerry_symbol_with_description (const jerry_value_t value);
/**
 * jerry-api-symbol-ctor @}
 */

/**
 * @defgroup jerry-api-symbol-get Getters
 * @{
 */
jerry_value_t jerry_symbol_description (const jerry_value_t symbol);
jerry_value_t jerry_symbol_descriptive_string (const jerry_value_t symbol);
/**
 * jerry-api-symbol-get @}
 */

/**
 * jerry-api-symbol @}
 */

/**
 * jerry-api-primitives @}
 */

/**
 * @defgroup jerry-api-objects Objects
 * @{
 */

/**
 * @defgroup jerry-api-object-ctor Constructors
 * @{
 */
jerry_value_t jerry_object (void);
/**
 * jerry-api-object-ctor @}
 */

/**
 * @defgroup jerry-api-object-get Getters
 * @{
 */

jerry_object_type_t jerry_object_type (const jerry_value_t object);
jerry_value_t jerry_object_proto (const jerry_value_t object);
jerry_value_t jerry_object_keys (const jerry_value_t object);
jerry_value_t jerry_object_property_names (const jerry_value_t object, jerry_property_filter_t filter);

/**
 * jerry-api-object-get @}
 */

/**
 * @defgroup jerry-api-object-op Operations
 * @{
 */

jerry_value_t jerry_object_set_proto (jerry_value_t object, const jerry_value_t proto);
bool jerry_object_foreach (const jerry_value_t object, jerry_object_property_foreach_cb_t foreach_p, void *user_data_p);

/**
 * @defgroup jerry-api-object-op-set Set
 * @{
 */
jerry_value_t jerry_object_set (jerry_value_t object, const jerry_value_t key, const jerry_value_t value);
jerry_value_t jerry_object_set_sz (jerry_value_t object, const char *key_p, const jerry_value_t value);
jerry_value_t jerry_object_set_index (jerry_value_t object, uint32_t index, const jerry_value_t value);
jerry_value_t jerry_object_define_own_prop (jerry_value_t object,
                                            const jerry_value_t key,
                                            const jerry_property_descriptor_t *prop_desc_p);
bool jerry_object_set_internal (jerry_value_t object, const jerry_value_t key, const jerry_value_t value);
void jerry_object_set_native_ptr (jerry_value_t object,
                                  const jerry_object_native_info_t *native_info_p,
                                  void *native_pointer_p);
/**
 * jerry-api-object-op-set @}
 */

/**
 * @defgroup jerry-api-object-op-has Has
 * @{
 */
jerry_value_t jerry_object_has (const jerry_value_t object, const jerry_value_t key);
jerry_value_t jerry_object_has_sz (const jerry_value_t object, const char *key_p);
jerry_value_t jerry_object_has_own (const jerry_value_t object, const jerry_value_t key);
bool jerry_object_has_internal (const jerry_value_t object, const jerry_value_t key);
bool jerry_object_has_native_ptr (const jerry_value_t object, const jerry_object_native_info_t *native_info_p);
/**
 * jerry-api-object-op-has @}
 */

/**
 * @defgroup jerry-api-object-op-get Get
 * @{
 */
jerry_value_t jerry_object_get (const jerry_value_t object, const jerry_value_t key);
jerry_value_t jerry_object_get_sz (const jerry_value_t object, const char *key_p);
jerry_value_t jerry_object_get_index (const jerry_value_t object, uint32_t index);
jerry_value_t jerry_object_get_own_prop (const jerry_value_t object,
                                         const jerry_value_t key,
                                         jerry_property_descriptor_t *prop_desc_p);
jerry_value_t jerry_object_get_internal (const jerry_value_t object, const jerry_value_t key);
void *jerry_object_get_native_ptr (const jerry_value_t object, const jerry_object_native_info_t *native_info_p);

jerry_value_t jerry_object_find_own (const jerry_value_t object,
                                     const jerry_value_t key,
                                     const jerry_value_t receiver,
                                     bool *found_p);
/**
 * jerry-api-object-op-get @}
 */

/**
 * @defgroup jerry-api-object-op-del Delete
 * @{
 */
jerry_value_t jerry_object_delete (jerry_value_t object, const jerry_value_t key);
jerry_value_t jerry_object_delete_sz (const jerry_value_t object, const char *key_p);
jerry_value_t jerry_object_delete_index (jerry_value_t object, uint32_t index);
bool jerry_object_delete_internal (jerry_value_t object, const jerry_value_t key);
bool jerry_object_delete_native_ptr (jerry_value_t object, const jerry_object_native_info_t *native_info_p);
/**
 * jerry-api-object-op-del @}
 */

/**
 * jerry-api-object-op @}
 */

/**
 * @defgroup jerry-api-object-prop-desc Property descriptors
 * @{
 */

/**
 * @defgroup jerry-api-object-prop-desc-ctor Constructors
 * @{
 */
jerry_property_descriptor_t jerry_property_descriptor (void);
jerry_value_t jerry_property_descriptor_from_object (const jerry_value_t obj_value,
                                                     jerry_property_descriptor_t *out_prop_desc_p);
/**
 * jerry-api-object-prop-desc-ctor @}
 */

/**
 * @defgroup jerry-api-object-prop-desc-op Operations
 * @{
 */
void jerry_property_descriptor_free (jerry_property_descriptor_t *prop_desc_p);
jerry_value_t jerry_property_descriptor_to_object (const jerry_property_descriptor_t *src_prop_desc_p);
/**
 * jerry-api-object-prop-desc-op @}
 */

/**
 * jerry-api-object-prop-desc @}
 */

/**
 * @defgroup jerry-api-object-native-ptr Native pointers
 * @{
 */

/**
 * @defgroup jerry-api-object-native-ptr-op Operations
 * @{
 */
void jerry_native_ptr_init (void *native_pointer_p, const jerry_object_native_info_t *native_info_p);
void jerry_native_ptr_free (void *native_pointer_p, const jerry_object_native_info_t *native_info_p);
void jerry_native_ptr_set (jerry_value_t *reference_p, const jerry_value_t value);
/**
 * jerry-api-object-native-ptr-op @}
 */

/**
 * jerry-api-object-native-ptr @}
 */

/**
 * @defgroup jerry-api-array Array
 * @{
 */

/**
 * @defgroup jerry-api-array-ctor Constructors
 * @{
 */
jerry_value_t jerry_array (jerry_length_t length);
/**
 * jerry-api-array-ctor @}
 */

/**
 * @defgroup jerry-api-array-get Getters
 * @{
 */
jerry_length_t jerry_array_length (const jerry_value_t value);
/**
 * jerry-api-array-get @}
 */

/**
 * jerry-api-array @}
 */

/**
 * @defgroup jerry-api-arraybuffer ArrayBuffer
 * @{
 */

/**
 * @defgroup jerry-api-arraybuffer-ctor Constructors
 * @{
 */
jerry_value_t jerry_arraybuffer (const jerry_length_t size);
jerry_value_t jerry_arraybuffer_external (uint8_t *buffer_p, jerry_length_t size, void *user_p);
/**
 * jerry-api-arraybuffer-ctor @}
 */

/**
 * @defgroup jerry-api-arraybuffer-get Getters
 * @{
 */
jerry_size_t jerry_arraybuffer_size (const jerry_value_t value);
uint8_t *jerry_arraybuffer_data (const jerry_value_t value);
bool jerry_arraybuffer_is_detachable (const jerry_value_t value);
bool jerry_arraybuffer_has_buffer (const jerry_value_t value);
/**
 * jerry-api-arraybuffer-get @}
 */

/**
 * @defgroup jerry-api-arraybuffer-op Operations
 * @{
 */
jerry_size_t
jerry_arraybuffer_read (const jerry_value_t value, jerry_size_t offset, uint8_t *buffer_p, jerry_size_t buffer_size);
jerry_size_t
jerry_arraybuffer_write (jerry_value_t value, jerry_size_t offset, const uint8_t *buffer_p, jerry_size_t buffer_size);
jerry_value_t jerry_arraybuffer_detach (jerry_value_t value);
void jerry_arraybuffer_heap_allocation_limit (jerry_size_t limit);
/**
 * jerry-api-arraybuffer-op @}
 */

/**
 * @defgroup jerry-api-arraybuffer-cb Callbacks
 * @{
 */
void jerry_arraybuffer_allocator (jerry_arraybuffer_allocate_cb_t allocate_callback,
                                  jerry_arraybuffer_free_cb_t free_callback,
                                  void *user_p);
/**
 * jerry-api-arraybuffer-cb @}
 */

/**
 * jerry-api-arraybuffer @}
 */

/**
 * @defgroup jerry-api-sharedarraybuffer SharedArrayBuffer
 * @{
 */

/**
 * @defgroup jerry-api-sharedarraybuffer-ctor Constructors
 * @{
 */
jerry_value_t jerry_shared_arraybuffer (jerry_size_t size);
jerry_value_t jerry_shared_arraybuffer_external (uint8_t *buffer_p, jerry_size_t buffer_size, void *user_p);
/**
 * jerry-api-sharedarraybuffer-ctor @}
 */

/**
 * jerry-api-sharedarraybuffer @}
 */

/**
 * @defgroup jerry-api-dataview DataView
 * @{
 */

/**
 * @defgroup jerry-api-dataview-ctor Constructors
 * @{
 */
jerry_value_t jerry_dataview (const jerry_value_t value, jerry_size_t byte_offset, jerry_size_t byte_length);
/**
 * jerry-api-dataview-ctr @}
 */

/**
 * @defgroup jerry-api-dataview-get Getters
 * @{
 */
jerry_value_t
jerry_dataview_buffer (const jerry_value_t dataview, jerry_size_t *byte_offset, jerry_size_t *byte_length);
/**
 * jerry-api-dataview-get @}
 */

/**
 * jerry-api-dataview @}
 */

/**
 * @defgroup jerry-api-typedarray TypedArray
 * @{
 */

/**
 * @defgroup jerry-api-typedarray-ctor Constructors
 * @{
 */
jerry_value_t jerry_typedarray (jerry_typedarray_type_t type, jerry_length_t length);
jerry_value_t jerry_typedarray_with_buffer (jerry_typedarray_type_t type, const jerry_value_t arraybuffer);
jerry_value_t jerry_typedarray_with_buffer_span (jerry_typedarray_type_t type,
                                                 const jerry_value_t arraybuffer,
                                                 jerry_size_t byte_offset,
                                                 jerry_size_t byte_length);
/**
 * jerry-api-typedarray-ctor @}
 */

/**
 * @defgroup jerry-api-typedarray-get Getters
 * @{
 */
jerry_typedarray_type_t jerry_typedarray_type (const jerry_value_t value);
jerry_length_t jerry_typedarray_length (const jerry_value_t value);
jerry_value_t jerry_typedarray_buffer (const jerry_value_t value, jerry_size_t *byte_offset, jerry_size_t *byte_length);
/**
 * jerry-api-typedarray-get @}
 */

/**
 * jerry-api-typedarray @}
 */

/**
 * @defgroup jerry-api-iterator Iterator
 * @{
 */

/**
 * @defgroup jerry-api-iterator-get Getters
 * @{
 */
jerry_iterator_type_t jerry_iterator_type (const jerry_value_t value);
/**
 * jerry-api-iterator-get @}
 */

/**
 * jerry-api-iterator @}
 */

/**
 * @defgroup jerry-api-function Function
 * @{
 */

/**
 * @defgroup jerry-api-function-ctor Constructors
 * @{
 */
jerry_value_t jerry_function_external (jerry_external_handler_t handler);
/**
 * jerry-api-function-ctor @}
 */

/**
 * @defgroup jerry-api-function-get Getters
 * @{
 */
jerry_function_type_t jerry_function_type (const jerry_value_t value);
bool jerry_function_is_dynamic (const jerry_value_t value);
/**
 * jerry-api-function-get @}
 */

/**
 * @defgroup jerry-api-function-op Operations
 * @{
 */
jerry_value_t jerry_call (const jerry_value_t function,
                          const jerry_value_t this_value,
                          const jerry_value_t *args_p,
                          jerry_size_t args_count);
jerry_value_t jerry_construct (const jerry_value_t function, const jerry_value_t *args_p, jerry_size_t args_count);
/**
 * jerry-api-function-op @}
 */

/**
 * jerry-api-function @}
 */

/**
 * @defgroup jerry-api-proxy Proxy
 * @{
 */

/**
 * @defgroup jerry-api-proxy-ctor Constructors
 * @{
 */
jerry_value_t jerry_proxy (const jerry_value_t target, const jerry_value_t handler);
jerry_value_t jerry_proxy_custom (const jerry_value_t target, const jerry_value_t handler, uint32_t flags);
/**
 * jerry-api-function-proxy-ctor @}
 */

/**
 * @defgroup jerry-api-proxy-get Getters
 * @{
 */
jerry_value_t jerry_proxy_target (const jerry_value_t value);
jerry_value_t jerry_proxy_handler (const jerry_value_t value);
/**
 * jerry-api-function-proxy-get @}
 */

/**
 * jerry-api-proxy @}
 */

/**
 * @defgroup jerry-api-promise Promise
 * @{
 */

/**
 * @defgroup jerry-api-promise-ctor Constructors
 * @{
 */
jerry_value_t jerry_promise (void);
/**
 * jerry-api-promise-ctor @}
 */

/**
 * @defgroup jerry-api-promise-get Getters
 * @{
 */
jerry_value_t jerry_promise_result (const jerry_value_t promise);
jerry_promise_state_t jerry_promise_state (const jerry_value_t promise);
/**
 * jerry-api-promise-get @}
 */

/**
 * @defgroup jerry-api-promise-op Operations
 * @{
 */
jerry_value_t jerry_promise_resolve (jerry_value_t promise, const jerry_value_t argument);
jerry_value_t jerry_promise_reject (jerry_value_t promise, const jerry_value_t argument);
/**
 * jerry-api-promise-op @}
 */

/**
 * @defgroup jerry-api-promise-cb Callbacks
 * @{
 */
void jerry_promise_on_event (jerry_promise_event_filter_t filters, jerry_promise_event_cb_t callback, void *user_p);
/**
 * jerry-api-promise-cb @}
 */

/**
 * jerry-api-promise @}
 */

/**
 * @defgroup jerry-api-container Map, Set, WeakMap, WeakSet
 * @{
 */

/**
 * @defgroup jerry-api-container-ctor Constructors
 * @{
 */
jerry_value_t jerry_container (jerry_container_type_t container_type,
                               const jerry_value_t *arguments_p,
                               jerry_length_t argument_count);
/**
 * jerry-api-promise-ctor @}
 */

/**
 * @defgroup jerry-api-container-get Getters
 * @{
 */
jerry_container_type_t jerry_container_type (const jerry_value_t value);
/**
 * jerry-api-container-get @}
 */

/**
 * @defgroup jerry-api-container-op Operations
 * @{
 */
jerry_value_t jerry_container_to_array (const jerry_value_t value, bool *is_key_value_p);
jerry_value_t jerry_container_op (jerry_container_op_t operation,
                                  jerry_value_t container,
                                  const jerry_value_t *arguments,
                                  uint32_t argument_count);
/**
 * jerry-api-container-op @}
 */

/**
 * jerry-api-container @}
 */

/**
 * @defgroup jerry-api-regexp RegExp
 * @{
 */

/**
 * @defgroup jerry-api-regexp-ctor Constructors
 * @{
 */
jerry_value_t jerry_regexp (const jerry_value_t pattern, uint16_t flags);
jerry_value_t jerry_regexp_sz (const char *pattern_p, uint16_t flags);
/**
 * jerry-api-regexp-ctor @}
 */

/**
 * jerry-api-regexp @}
 */

/**
 * @defgroup jerry-api-error Error
 * @{
 */

/**
 * @defgroup jerry-api-error-ctor Constructors
 * @{
 */
jerry_value_t jerry_error (jerry_error_t type, const jerry_value_t message);
jerry_value_t jerry_error_sz (jerry_error_t type, const char *message_p);
/**
 * jerry-api-error-ctor @}
 */

/**
 * @defgroup jerry-api-error-get Getters
 * @{
 */
jerry_error_t jerry_error_type (jerry_value_t value);
/**
 * jerry-api-error-get @}
 */

/**
 * @defgroup jerry-api-error-cb Callbacks
 * @{
 */
void jerry_error_on_created (jerry_error_object_created_cb_t callback, void *user_p);
/**
 * jerry-api-error-cb @}
 */

/**
 * jerry-api-error @}
 */

/**
 * jerry-api-objects @}
 */

/**
 * @defgroup jerry-api-json JSON
 * @{
 */

/**
 * @defgroup jerry-api-json-op Operations
 * @{
 */
jerry_value_t jerry_json_parse (const jerry_char_t *string_p, jerry_size_t string_size);
jerry_value_t jerry_json_stringify (const jerry_value_t object);
/**
 * jerry-api-json-op @}
 */

/**
 * jerry-api-json @}
 */

/**
 * @defgroup jerry-api-module Modules
 * @{
 */

/**
 * @defgroup jerry-api-module-get Getters
 * @{
 */
jerry_module_state_t jerry_module_state (const jerry_value_t module);
size_t jerry_module_request_count (const jerry_value_t module);
jerry_value_t jerry_module_request (const jerry_value_t module, size_t request_index);
jerry_value_t jerry_module_namespace (const jerry_value_t module);
/**
 * jerry-api-module-get @}
 */

/**
 * @defgroup jerry-api-module-op Operations
 * @{
 */

/**
 * Resolve and parse a module file
 *
 * @param specifier: module request specifier string.
 * @param referrer: parent module.
 * @param user_p: user specified pointer.
 *
 * @return module object if resolving is successful, error otherwise.
 */
jerry_value_t jerry_module_resolve (const jerry_value_t specifier, const jerry_value_t referrer, void *user_p);

jerry_value_t jerry_module_link (const jerry_value_t module, jerry_module_resolve_cb_t callback, void *user_p);
jerry_value_t jerry_module_evaluate (const jerry_value_t module);

/**
 * Release known modules in the current context. If realm parameter is supplied, cleans up modules native to that realm
 * only. This function should be called by the user application when the module database in the current context is no
 * longer needed.
 *
 * @param realm: release only those modules which realm value is equal to this argument.
 */
void jerry_module_cleanup (const jerry_value_t realm);

/**
 * jerry-api-module-op @}
 */

/**
 * @defgroup jerry-api-module-native Native modules
 * @{
 */
jerry_value_t jerry_native_module (jerry_native_module_evaluate_cb_t callback,
                                   const jerry_value_t *const exports_p,
                                   size_t export_count);
jerry_value_t jerry_native_module_get (const jerry_value_t native_module, const jerry_value_t export_name);
jerry_value_t
jerry_native_module_set (jerry_value_t native_module, const jerry_value_t export_name, const jerry_value_t value);
/**
 * jerry-api-module-native @}
 */

/**
 * @defgroup jerry-api-module-cb Callbacks
 * @{
 */
void jerry_module_on_state_changed (jerry_module_state_changed_cb_t callback, void *user_p);
void jerry_module_on_import_meta (jerry_module_import_meta_cb_t callback, void *user_p);
void jerry_module_on_import (jerry_module_import_cb_t callback, void *user_p);
/**
 * jerry-api-module-cb @}
 */

/**
 * jerry-api-module @}
 */

/**
 * @defgroup jerry-api-realm Realms
 * @{
 */

/**
 * @defgroup jerry-api-realm-ctor Constructors
 * @{
 */
jerry_value_t jerry_realm (void);
/**
 * jerry-api-realm-ctor @}
 */

/**
 * @defgroup jerry-api-realm-get Getters
 * @{
 */
jerry_value_t jerry_realm_this (jerry_value_t realm);
/**
 * jerry-api-realm-ctor @}
 */

/**
 * @defgroup jerry-api-realm-op Operation
 * @{
 */
jerry_value_t jerry_realm_set_this (jerry_value_t realm, jerry_value_t this_value);
/**
 * jerry-api-realm-op @}
 */

/**
 * jerry-api-realm @}
 */

/**
 * jerry-api @}
 */

JERRY_C_API_END

#endif /* !JERRYSCRIPT_CORE_H */

/* vim: set fdm=marker fmr=@{,@}: */

#ifndef JERRYSCRIPT_DEBUGGER_H
#define JERRYSCRIPT_DEBUGGER_H


JERRY_C_API_BEGIN

/** \addtogroup jerry-debugger Jerry engine interface - Debugger feature
 * @{
 */

/**
 * JerryScript debugger protocol version.
 */
#define JERRY_DEBUGGER_VERSION (9)

/**
 * Types for the client source wait and run method.
 */
typedef enum
{
  JERRY_DEBUGGER_SOURCE_RECEIVE_FAILED = 0, /**< source is not received */
  JERRY_DEBUGGER_SOURCE_RECEIVED = 1, /**< a source has been received */
  JERRY_DEBUGGER_SOURCE_END = 2, /**< the end of the sources signal received */
  JERRY_DEBUGGER_CONTEXT_RESET_RECEIVED, /**< the context reset request has been received */
} jerry_debugger_wait_for_source_status_t;

/**
 * Callback for jerry_debugger_wait_and_run_client_source
 *
 * The callback receives the source name, source code and a user pointer.
 *
 * @return this value is passed back by jerry_debugger_wait_and_run_client_source
 */
typedef jerry_value_t (*jerry_debugger_wait_for_source_callback_t) (const jerry_char_t *source_name_p,
                                                                    size_t source_name_size,
                                                                    const jerry_char_t *source_p,
                                                                    size_t source_size,
                                                                    void *user_p);

/**
 * Engine debugger functions.
 */
bool jerry_debugger_is_connected (void);
void jerry_debugger_stop (void);
void jerry_debugger_continue (void);
void jerry_debugger_stop_at_breakpoint (bool enable_stop_at_breakpoint);
jerry_debugger_wait_for_source_status_t
jerry_debugger_wait_for_client_source (jerry_debugger_wait_for_source_callback_t callback_p,
                                       void *user_p,
                                       jerry_value_t *return_value);
void jerry_debugger_send_output (const jerry_char_t *buffer, jerry_size_t str_size);

/**
 * @}
 */

JERRY_C_API_END

#endif /* !JERRYSCRIPT_DEBUGGER_H */

#ifndef JERRYSCRIPT_SNAPSHOT_H
#define JERRYSCRIPT_SNAPSHOT_H


JERRY_C_API_BEGIN

/** \addtogroup jerry-snapshot Jerry engine interface - Snapshot feature
 * @{
 */

/**
 * Jerry snapshot format version.
 */
#define JERRY_SNAPSHOT_VERSION (70u)

/**
 * Flags for jerry_generate_snapshot and jerry_generate_function_snapshot.
 */
typedef enum
{
  JERRY_SNAPSHOT_SAVE_STATIC = (1u << 0), /**< static snapshot */
} jerry_generate_snapshot_opts_t;

/**
 * Flags for jerry_exec_snapshot.
 */
typedef enum
{
  JERRY_SNAPSHOT_EXEC_COPY_DATA = (1u << 0), /**< copy snashot data */
  JERRY_SNAPSHOT_EXEC_ALLOW_STATIC = (1u << 1), /**< static snapshots allowed */
  JERRY_SNAPSHOT_EXEC_LOAD_AS_FUNCTION = (1u << 2), /**< load snapshot as function instead of executing it */
  JERRY_SNAPSHOT_EXEC_HAS_SOURCE_NAME = (1u << 3), /**< source_name field is valid
                                                    *   in jerry_exec_snapshot_option_values_t */
  JERRY_SNAPSHOT_EXEC_HAS_USER_VALUE = (1u << 4), /**< user_value field is valid
                                                   *   in jerry_exec_snapshot_option_values_t */
} jerry_exec_snapshot_opts_t;

/**
 * Various configuration options for jerry_exec_snapshot.
 */
typedef struct
{
  jerry_value_t source_name; /**< source name string (usually a file name)
                              *   if JERRY_SNAPSHOT_EXEC_HAS_SOURCE_NAME is set in exec_snapshot_opts
                              *   Note: non-string values are ignored */
  jerry_value_t user_value; /**< user value assigned to all functions created by this script including
                             *   eval calls executed by the script if JERRY_SNAPSHOT_EXEC_HAS_USER_VALUE
                             *   is set in exec_snapshot_opts */
} jerry_exec_snapshot_option_values_t;

/**
 * Snapshot functions.
 */
jerry_value_t jerry_generate_snapshot (jerry_value_t compiled_code,
                                       uint32_t generate_snapshot_opts,
                                       uint32_t *buffer_p,
                                       size_t buffer_size);

jerry_value_t jerry_exec_snapshot (const uint32_t *snapshot_p,
                                   size_t snapshot_size,
                                   size_t func_index,
                                   uint32_t exec_snapshot_opts,
                                   const jerry_exec_snapshot_option_values_t *options_values_p);

size_t jerry_merge_snapshots (const uint32_t **inp_buffers_p,
                              size_t *inp_buffer_sizes_p,
                              size_t number_of_snapshots,
                              uint32_t *out_buffer_p,
                              size_t out_buffer_size,
                              const char **error_p);
size_t jerry_get_literals_from_snapshot (const uint32_t *snapshot_p,
                                         size_t snapshot_size,
                                         jerry_char_t *lit_buf_p,
                                         size_t lit_buf_size,
                                         bool is_c_format);
/**
 * @}
 */

JERRY_C_API_END

#endif /* !JERRYSCRIPT_SNAPSHOT_H */

#endif /* !JERRYSCRIPT_H */

#ifndef JERRYSCRIPT_PORT_H
#define JERRYSCRIPT_PORT_H


JERRY_C_API_BEGIN

/**
 * @defgroup jerry-port JerryScript Port API
 * @{
 */

/**
 * @defgroup jerry-port-process Process management API
 *
 * It is questionable whether a library should be able to terminate an
 * application. However, as of now, we only have the concept of completion
 * code around jerry_parse and jerry_run. Most of the other API functions
 * have no way of signaling an error. So, we keep the termination approach
 * with this port function.
 *
 * @{
 */

/**
 * Error codes that can be passed by the engine when calling jerry_port_fatal
 */
typedef enum
{
  JERRY_FATAL_OUT_OF_MEMORY = 10, /**< Out of memory */
  JERRY_FATAL_REF_COUNT_LIMIT = 12, /**< Reference count limit reached */
  JERRY_FATAL_DISABLED_BYTE_CODE = 13, /**< Executed disabled instruction */
  JERRY_FATAL_UNTERMINATED_GC_LOOPS = 14, /**< Garbage collection loop limit reached */
  JERRY_FATAL_FAILED_ASSERTION = 120 /**< Assertion failed */
} jerry_fatal_code_t;

/**
 * Signal the port that the process experienced a fatal failure from which it cannot
 * recover.
 *
 * A libc-based port may implement this with exit() or abort(), or both.
 *
 * Note: This function is expected to not return.
 *
 * @param code: the cause of the error.
 */
void JERRY_ATTR_NORETURN jerry_port_fatal (jerry_fatal_code_t code);

/**
 * Make the process sleep for a given time.
 *
 * This port function can be called by jerry-core when JERRY_DEBUGGER is enabled.
 * Otherwise this function is not used.
 *
 * @param sleep_time: milliseconds to sleep.
 */
void jerry_port_sleep (uint32_t sleep_time);

/**
 * jerry-port-process @}
 */

/**
 * @defgroup jerry-port-context External Context API
 * @{
 */

/**
 * Allocate a new context for the engine.
 *
 * This port function is called by jerry_init when JERRY_EXTERNAL_CONTEXT is enabled. Otherwise this function is not
 * used.
 *
 * The engine will pass the size required for the context structure. An implementation must make sure to
 * allocate at least this amount.
 *
 * Excess allocated space will be used as the engine heap when JerryScript is configured to use it's internal allocator,
 * this can be used to control the internal heap size.
 *
 * NOTE: The allocated memory must be pointer-aligned, otherwise the behavior is undefined.
 *
 * @param context_size: the size of the internal context structure
 *
 * @return total size of the allocated buffer
 */
size_t jerry_port_context_alloc (size_t context_size);

/**
 * Get the currently active context of the engine.
 *
 * This port function is called by jerry-core when JERRY_EXTERNAL_CONTEXT is enabled.
 * Otherwise this function is not used.
 *
 * @return the pointer to the currently used engine context.
 */
struct jerry_context_t *jerry_port_context_get (void);

/**
 * Free the currently used context.
 *
 * This port function is called by jerry_cleanup when JERRY_EXTERNAL_CONTEXT is enabled.
 * Otherwise this function is not used.
 *
 * @return the pointer to the engine context.
 */
void jerry_port_context_free (void);

/**
 * jerry-port-context @}
 */

/**
 * @defgroup jerry-port-io I/O API
 * @{
 */

/**
 * Display or log a debug/error message.
 *
 * The message is passed as a zero-terminated string. Messages may be logged in parts, which
 * will result in multiple calls to this functions. The implementation should consider
 * this before appending or prepending strings to the argument.
 *
 * This function is called with messages coming from the jerry engine as
 * the result of some abnormal operation or describing its internal operations
 * (e.g., data structure dumps or tracing info).
 *
 * The implementation can decide whether error and debug messages are logged to
 * the console, or saved to a database or to a file.
 */
void jerry_port_log (const char *message_p);

/**
 * Print a single character to standard output.
 *
 * This port function is never called from jerry-core directly, it is only used by jerry-ext components to print
 * information.
 *
 * @param byte: the byte to print.
 */
void jerry_port_print_byte (jerry_char_t byte);

/**
 * Print a buffer to standard output
 *
 * This port function is never called from jerry-core directly, it is only used by jerry-ext components to print
 * information.
 *
 * @param buffer_p: input buffer
 * @param buffer_size: data size
 */
void jerry_port_print_buffer (const jerry_char_t *buffer_p, jerry_size_t buffer_size);

/**
 * Read a line from standard input.
 *
 * The implementation should allocate storage necessary for the string. The result string should include the ending line
 * terminator character(s) and should be zero terminated.
 *
 * An implementation may return NULL to signal that the end of input is reached, or an error occured.
 *
 * When a non-NULL value is returned, the caller will pass the returned value to `jerry_port_line_free` when the line is
 * no longer needed. This can be used to finalize dynamically allocated buffers if necessary.
 *
 * This port function is never called from jerry-core directly, it is only used by some jerry-ext components that
 * require user input.
 *
 * @param out_size_p: size of the input string in bytes, excluding terminating zero byte
 *
 * @return pointer to the buffer storing the string,
 *         or NULL if end of input
 */
jerry_char_t *jerry_port_line_read (jerry_size_t *out_size_p);

/**
 * Free a line buffer allocated by jerry_port_line_read
 *
 * @param buffer_p: buffer returned by jerry_port_line_read
 */
void jerry_port_line_free (jerry_char_t *buffer_p);

/**
 * jerry-port-io @}
 */

/**
 * @defgroup jerry-port-fd Filesystem API
 * @{
 */

/**
 * Canonicalize a file path.
 *
 * If possible, the implementation should resolve symbolic links and other directory references found in the input path,
 * and create a fully canonicalized file path as the result.
 *
 * The function may return with NULL in case an error is encountered, in which case the calling operation will not
 * proceed.
 *
 * The implementation should allocate storage for the result path as necessary. Non-NULL return values will be passed
 * to `jerry_port_path_free` when the result is no longer needed by the caller, which can be used to finalize
 * dynamically allocated buffers.
 *
 * NOTE: The implementation must not return directly with the input, as the input buffer is released after the call.
 *
 * @param path_p: zero-terminated string containing the input path
 * @param path_size: size of the input path string in bytes, excluding terminating zero
 *
 * @return buffer with the normalized path if the operation is successful,
 *         NULL otherwise
 */
jerry_char_t *jerry_port_path_normalize (const jerry_char_t *path_p, jerry_size_t path_size);

/**
 * Free a path buffer returned by jerry_port_path_normalize.
 *
 * @param path_p: the path buffer to free
 */
void jerry_port_path_free (jerry_char_t *path_p);

/**
 * Get the offset of the basename component in the input path.
 *
 * The implementation should return the offset of the first character after the last path separator found in the path.
 * This is used by the caller to split the path into a directory name and a file name.
 *
 * @param path_p: input zero-terminated path string
 *
 * @return offset of the basename component in the input path
 */
jerry_size_t jerry_port_path_base (const jerry_char_t *path_p);

/**
 * Open a source file and read the content into a buffer.
 *
 * When the source file is no longer needed by the caller, the returned pointer will be passed to
 * `jerry_port_source_free`, which can be used to finalize the buffer.
 *
 * @param file_name_p: Path that points to the source file in the filesystem.
 * @param out_size_p: The opened file's size in bytes.
 *
 * @return pointer to the buffer which contains the content of the file.
 */
jerry_char_t *jerry_port_source_read (const char *file_name_p, jerry_size_t *out_size_p);

/**
 * Free a source file buffer.
 *
 * @param buffer_p: buffer returned by jerry_port_source_read
 */
void jerry_port_source_free (jerry_char_t *buffer_p);

/**
 * jerry-port-fs @}
 */

/**
 * @defgroup jerry-port-date Date API
 * @{
 */

/**
 * Get local time zone adjustment in milliseconds for the given input time.
 *
 * The argument is a time value representing milliseconds since unix epoch.
 *
 * Ideally, this function should satisfy the stipulations applied to LocalTZA
 * in section 21.4.1.7 of the ECMAScript version 12.0, as if called with isUTC true.
 *
 * This port function can be called by jerry-core when JERRY_BUILTIN_DATE is enabled.
 * Otherwise this function is not used.
 *
 * @param unix_ms: time value in milliseconds since unix epoch
 *
 * @return local time offset in milliseconds applied to UTC for the given time value
 */
int32_t jerry_port_local_tza (double unix_ms);

/**
 * Get the current system time in UTC.
 *
 * This port function is called by jerry-core when JERRY_BUILTIN_DATE is enabled.
 * It can also be used in the implementing application to initialize the random number generator.
 *
 * @return milliseconds since Unix epoch
 */
double jerry_port_current_time (void);

/**
 * jerry-port-date @}
 */

/**
 * jerry-port @}
 */

JERRY_C_API_END

#endif /* !JERRYSCRIPT_PORT_H */

/* vim: set fdm=marker fmr=@{,@}: */

#ifndef JERRYSCRIPT_DEBUGGER_TRANSPORT_H
#define JERRYSCRIPT_DEBUGGER_TRANSPORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


JERRY_C_API_BEGIN

/** \addtogroup jerry-debugger-transport Jerry engine debugger interface - transport control
 * @{
 */

/**
 * Maximum number of bytes transmitted or received.
 */
#define JERRY_DEBUGGER_TRANSPORT_MAX_BUFFER_SIZE 128

/**
 * Receive message context.
 */
typedef struct
{
  uint8_t *buffer_p; /**< buffer for storing the received data */
  size_t received_length; /**< number of currently received bytes */
  uint8_t *message_p; /**< start of the received message */
  size_t message_length; /**< length of the received message */
  size_t message_total_length; /**< total length for datagram protocols,
                                *   0 for stream protocols */
} jerry_debugger_transport_receive_context_t;

/**
 * Forward definition of jerry_debugger_transport_header_t.
 */
struct jerry_debugger_transport_interface_t;

/**
 * Close connection callback.
 */
typedef void (*jerry_debugger_transport_close_t) (struct jerry_debugger_transport_interface_t *header_p);

/**
 * Send data callback.
 */
typedef bool (*jerry_debugger_transport_send_t) (struct jerry_debugger_transport_interface_t *header_p,
                                                 uint8_t *message_p,
                                                 size_t message_length);

/**
 * Receive data callback.
 */
typedef bool (*jerry_debugger_transport_receive_t) (struct jerry_debugger_transport_interface_t *header_p,
                                                    jerry_debugger_transport_receive_context_t *context_p);

/**
 * Transport layer header.
 */
typedef struct jerry_debugger_transport_interface_t
{
  /* The following fields must be filled before calling jerry_debugger_transport_add(). */
  jerry_debugger_transport_close_t close; /**< close connection callback */
  jerry_debugger_transport_send_t send; /**< send data callback */
  jerry_debugger_transport_receive_t receive; /**< receive data callback */

  /* The following fields are filled by jerry_debugger_transport_add(). */
  struct jerry_debugger_transport_interface_t *next_p; /**< next transport layer */
} jerry_debugger_transport_header_t;

void jerry_debugger_transport_add (jerry_debugger_transport_header_t *header_p,
                                   size_t send_message_header_size,
                                   size_t max_send_message_size,
                                   size_t receive_message_header_size,
                                   size_t max_receive_message_size);
void jerry_debugger_transport_start (void);

bool jerry_debugger_transport_is_connected (void);
void jerry_debugger_transport_close (void);

bool jerry_debugger_transport_send (const uint8_t *message_p, size_t message_length);
bool jerry_debugger_transport_receive (jerry_debugger_transport_receive_context_t *context_p);
void jerry_debugger_transport_receive_completed (jerry_debugger_transport_receive_context_t *context_p);

void jerry_debugger_transport_sleep (void);

/**
 * @}
 */

JERRY_C_API_END

#endif /* !JERRYSCRIPT_DEBUGGER_TRANSPORT_H */
