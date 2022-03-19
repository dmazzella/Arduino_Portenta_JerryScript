#include "Arduino.h"
#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"
#include "PluggableUSBMSD.h"
#include "mbed.h"

#include "Arduino_Portenta_JerryScript.h"

REDIRECT_STDOUT_TO(Serial);

QSPIFBlockDevice root(QSPI_SO0, QSPI_SO1, QSPI_SO2, QSPI_SO3, QSPI_SCK, QSPI_CS, QSPIF_POLARITY_MODE_1, 40000000);
USBMSD MassStorage(&root);

mbed::MBRBlockDevice js_data(&root, 1);
mbed::FATFileSystem js_data_fs("js");

void print_directory(const char *path_p) {
  /* list the content of the partitions, you may need to restart the board for the list to update if you copied new files */
  DIR *d = NULL;
  if ((d = opendir(path_p)) != NULL) {
    struct dirent *p = NULL;
    while ((p = readdir(d)) != NULL) {
      printf("%s\n", p->d_name);
    }
  }
  closedir(d);
}

void USBMSD::begin() {
  mbed::MBRBlockDevice::partition(&root, 1, 0x0B, 0, (1024 * 1024) * 10);
  if (js_data_fs.mount(&js_data)) {
    /* Reformat if we can't mount the filesystem this should only happen on the first boot */
    printf("No filesystem was found. Formatting the filsystem\n");
    js_data_fs.reformat(&js_data);
  }
}

void setup() {
  /* Initialize Serial */
  Serial.begin(115200);

  /* Initialize MassStorage */
  MassStorage.begin();

  /* Wait Serial */
  while (!Serial) {}

  printf("Mbed OS API: %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  printf("JerryScript API: %d.%d.%d\n", JERRY_API_MAJOR_VERSION, JERRY_API_MINOR_VERSION, JERRY_API_PATCH_VERSION);
}

void loop() {
  if (MassStorage.media_removed()) {

    /* Initialize engine */
    jerry_init(JERRY_INIT_EMPTY);

    /* Set log level */
    jerry_log_set_level(JERRY_LOG_LEVEL_DEBUG);

    /* Register the print function in the global object */
    jerryx_register_global("print", jerryx_handler_print);

    jerry_size_t source_size;
    jerry_char_t *source_p = NULL;
    if ((source_p = jerry_port_source_read("/js/index.js", &source_size)) != NULL) {

      jerry_parse_options_t parse_options;
      parse_options.options = JERRY_PARSE_NO_OPTS | JERRY_PARSE_MODULE;

      /* Setup Global scope code */
      jerry_value_t parsed_code = jerry_parse(source_p, source_size, &parse_options);

      /* Check if there is any JS code parse error */
      if (jerry_value_is_error(parsed_code)) {
        JERRYX_ERROR_MSG("parse: %d\n", jerry_value_is_error(parsed_code));
      } else {
        if (parse_options.options & JERRY_PARSE_MODULE) {
          /* Link modules to their dependencies */
          jerry_value_t ret_value = jerry_module_link(parsed_code, NULL, NULL);
          if (jerry_value_is_error(ret_value)) {
            JERRYX_ERROR_MSG("module_link: %d\n", jerry_value_is_error(ret_value));
          } else {
            /* Returned value must be freed */
            jerry_value_free(ret_value);

            /* Execute the parsed source code in the Global scope */
            ret_value = jerry_module_evaluate(parsed_code);

            /* Returned value must be freed */
            jerry_value_free(ret_value);
          }
        } else {
          /* Execute the parsed source code in the Global scope */
          jerry_value_t ret_value = jerry_run(parsed_code);

          /* Check the execution return value if there is any error */
          if (jerry_value_is_error(ret_value)) {
            JERRYX_ERROR_MSG("run: %d\n", jerry_value_is_error(ret_value));
          }

          /* Returned value must be freed */
          jerry_value_free(ret_value);
        }
      }

      /* Parsed source code must be freed */
      jerry_value_free(parsed_code);

      jerry_port_source_free(source_p);
    }

    /* Cleanup engine */
    jerry_cleanup();
  }

  delay(1000);
}
