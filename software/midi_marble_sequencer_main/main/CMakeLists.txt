get_filename_component(COMMON_SRC_DIR ../../common_src ABSOLUTE)
include("${COMMON_SRC_DIR}/common_src.cmake")

set(SOURCES "${COMMON_SRC_SOURCES}" main.cpp BeatsLEDSnake.cpp CalibrationTest.cpp HorizontalCalibration.cpp IRSensBoards.cpp IRSensReader.cpp marble_type.cpp MarbleDetector.cpp MasterClock.cpp MidiController.cpp MidiMapper.cpp)
set(include_dirs "${COMMON_SRC_INCLUDE_DIRS}" .)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "${include_dirs}"
                    PRIV_REQUIRES esp_timer esp_adc
                    )
