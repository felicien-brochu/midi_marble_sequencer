get_filename_component(COMMON_SRC_DIR ../../common_src ABSOLUTE)
include("${COMMON_SRC_DIR}/common_src.cmake")

set(include_dirs "${COMMON_SRC_INCLUDE_DIRS}" .)

idf_component_register(SRCS "BPMPotentiometer.cpp" "PlayPauseLED.cpp" "PlayPauseSwitch.cpp" "PushButtonsController.cpp" "RotaryButtonsController.cpp" "main.cpp" "LEDColumn.cpp" "${COMMON_SRC_SOURCES}"
                       INCLUDE_DIRS "${include_dirs}"
                       PRIV_REQUIRES driver esp_timer esp_driver_i2c esp_adc)
