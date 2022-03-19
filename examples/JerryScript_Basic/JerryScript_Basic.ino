#include "Arduino.h"
#include "mbed.h"

#include "Arduino_Portenta_JerryScript.h"

REDIRECT_STDOUT_TO(Serial);


void setup() {
  /* Initialize Serial */
  Serial.begin(115200);

  /* Wait Serial */
  while (!Serial) {}

  printf("Mbed OS API: %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  printf("JerryScript API: %d.%d.%d\n", JERRY_API_MAJOR_VERSION, JERRY_API_MINOR_VERSION, JERRY_API_PATCH_VERSION);

  const jerry_char_t script[] = "print ('Hello, World!', Math.random ());";

  /* Initialize engine */
  jerry_init (JERRY_INIT_EMPTY);

  /* Set log level */
  jerry_log_set_level (JERRY_LOG_LEVEL_DEBUG);

  /* Register the print function in the global object */
  jerryx_register_global ("print", jerryx_handler_print);

  /* Setup Global scope code */
  jerry_value_t parsed_code = jerry_parse (script, sizeof (script) - 1, NULL);

  /* Check if there is any JS code parse error */
  if (jerry_value_is_error (parsed_code))
  {
    JERRYX_ERROR_MSG("parse: %d\n", jerry_value_is_error (parsed_code));
  }
  else
  {
    /* Execute the parsed source code in the Global scope */
    jerry_value_t ret_value = jerry_run (parsed_code);

    /* Check the execution return value if there is any error */
    if (jerry_value_is_error (ret_value))
    {
      JERRYX_ERROR_MSG("run: %d\n", jerry_value_is_error (ret_value));
    }

    /* Returned value must be freed */
    jerry_value_free (ret_value);
  }

  /* Parsed source code must be freed */
  jerry_value_free (parsed_code);

  /* Cleanup engine */
  jerry_cleanup ();
}

void loop() {
  delay(1000);
}