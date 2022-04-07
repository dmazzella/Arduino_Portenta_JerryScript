#include "Arduino.h"
#include "mbed.h"

#include "Arduino_Portenta_JerryScript.h"

REDIRECT_STDOUT_TO(Serial);


void setup() {
  /* Initialize Serial */
  Serial.begin(115200);

  /* Wait Serial */
  while (!Serial) {}

  printf("\n");
  printf("Arduino Core API: %d.%d.%d\n", CORE_MAJOR, CORE_MINOR, CORE_PATCH);
  printf("Mbed OS API: %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  printf("JerryScript API: %d.%d.%d\n", JERRY_API_MAJOR_VERSION, JERRY_API_MINOR_VERSION, JERRY_API_PATCH_VERSION);

  const jerry_char_t script[] = "print ('Hello, World!', Math.random ());";

  /* Initialize engine */
  jerry_init (JERRY_INIT_EMPTY);

  /* Set log level */
  jerry_log_set_level (JERRY_LOG_LEVEL_DEBUG);

  /* Register the print function in the global object */
  jerryx_register_global ("print", jerryx_handler_print);

  /* Run script with 'eval' */
  jerry_value_free (jerry_eval (script, sizeof (script) - 1, JERRY_PARSE_NO_OPTS));

  /* Cleanup engine */
  jerry_cleanup ();
}

void loop() {
  delay(1000);
}