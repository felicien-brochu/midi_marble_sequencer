get_filename_component(COMMON_SRC_DIR ../../common_src ABSOLUTE)
include("${COMMON_SRC_DIR}/common_src.cmake")

set(include_dirs "${COMMON_SRC_INCLUDE_DIRS}" .)

idf_component_register(SRCS "DisplayController.cpp" "InteractionController.cpp" "I2CSlaveController.cpp" "main.cpp" "LEDRow.cpp" "${COMMON_SRC_SOURCES}"
                       INCLUDE_DIRS "${include_dirs}"
                       PRIV_REQUIRES esp_timer esp_driver_i2c esp_driver_gpio)
