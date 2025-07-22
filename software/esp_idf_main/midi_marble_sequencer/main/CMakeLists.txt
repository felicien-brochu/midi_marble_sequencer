set(SOURCES main.cpp test.cpp)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp_timer
                    )
