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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "jerryscript.h"
#include "mbed.h"


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
  putchar (byte);
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
  char *line_p = NULL;
  size_t allocated = 0;
  size_t bytes = 0;

  while (true)
  {
    allocated += 64;
    line_p = realloc (line_p, allocated);

    while (bytes < allocated - 1)
    {
      char ch = (char) fgetc (stdin);

      if (feof (stdin))
      {
        free (line_p);
        return NULL;
      }

      line_p[bytes++] = ch;

      if (ch == '\n')
      {
        *out_size_p = (jerry_size_t) bytes;
        line_p[bytes++] = '\0';
        return (jerry_char_t *) line_p;
      }
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
  current_context_p = malloc (total_size);

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
