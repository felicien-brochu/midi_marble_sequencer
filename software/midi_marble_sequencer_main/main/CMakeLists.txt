get_filename_component(COMMON_SRC_DIR ../../common_src ABSOLUTE)
include("${COMMON_SRC_DIR}/common_src.cmake")

set(include_dirs "${COMMON_SRC_INCLUDE_DIRS}" .)

idf_component_register(SRCS "Sequencer.cpp" "ControlBoardsController.cpp" "main.cpp" "BeatsLEDSnake.cpp" "CalibrationTest.cpp" "HorizontalCalibration.cpp" "IRSensBoards.cpp" "IRSensReader.cpp" "marble_type.cpp" "MarbleDetector.cpp" "MasterClock.cpp" "MidiController.cpp" "MidiMapper.cpp" "${COMMON_SRC_SOURCES}"
                    INCLUDE_DIRS "${include_dirs}"
                    PRIV_REQUIRES esp_timer esp_adc driver esp_driver_i2c
                    )
