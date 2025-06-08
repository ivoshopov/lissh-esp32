idf_component_register(SRCS "uart.c"
                    INCLUDE_DIRS ""
                    PRIV_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/tinylisp/src/"
                    PRIV_REQUIRES esp_driver_uart
                    )

