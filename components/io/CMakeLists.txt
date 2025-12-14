idf_component_register(SRCS "uart.c" "gpio.c" "nvs.c" "espinit-port.c" "adc.c" "adc-ctl.c" "bt.c" "gpioctl.c"
                    INCLUDE_DIRS ""
                    PRIV_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/tinylisp/src/"
                    PRIV_REQUIRES esp_driver_uart esp_driver_gpio nvs_flash esp_adc bt log
                    )

target_compile_definitions(${COMPONENT_TARGET} PRIVATE CONFIG_EXAMPLE_SSP_ENABLED=true)
