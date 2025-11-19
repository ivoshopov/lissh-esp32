idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "")

target_link_libraries(${COMPONENT_LIB} ${CMAKE_SOURCE_DIR}/tinylisp/libtinylisp.a "-u io_uart_module -u uart1_port -u std_port -u gpio0_port -u gpio0_dir_port -u box_primitive -u time_primitive -u loop_primitive -u print_primitive -u read_from_primitive -u io_nvs_module -u init_script_port -u begin_port -u null_port -u init_port -u espinit_port -u env_primitive -u adc_port0 -u adc_port1")
