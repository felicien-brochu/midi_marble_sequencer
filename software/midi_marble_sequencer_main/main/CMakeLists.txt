set(SOURCES main.cpp BeatsLEDSnake.cpp CalibrationTest.cpp CD74HC4067.cpp HorizontalCalibration.cpp IRSensBoards.cpp IRSensReader.cpp marble_type.cpp MarbleDetector.cpp MasterClock.cpp MidiController.cpp MidiMapper.cpp PushButton.cpp)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_timer esp_adc
                    )
