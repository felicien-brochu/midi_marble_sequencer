get_filename_component(COMMON_SRC_DIR ../../common_src ABSOLUTE)
include("${COMMON_SRC_DIR}/common_src.cmake")

set(include_dirs "${COMMON_SRC_INCLUDE_DIRS}" .)

idf_component_register(SRCS "SequencerPage.cpp" "Sequencer.cpp" "ControlBoardsController.cpp" "main.cpp" "BeatsLEDSnake.cpp" "IRSensBoards.cpp" "IRSensReader.cpp" "marble_type.cpp" "MarbleDetector.cpp" "MasterClock.cpp" "MidiController.cpp" "MidiMapper.cpp" "calibration/CalibrationStorage.cpp" "calibration/CalibrationController.cpp" "calibration/CalibrationDisplay.cpp" "calibration/SensorStatistics.cpp" "${COMMON_SRC_SOURCES}"
                    INCLUDE_DIRS "${include_dirs}"
                    PRIV_REQUIRES esp_timer esp_adc esp_driver_i2c esp_driver_gptimer nvs_flash
                    )
