# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# TODO: remove vfs. It isn't require from the main but esp_driver_uart require it.
set(COMPONENTS "main gpio esp_driver_uart vfs esp_gdbstub time io")

project(repl-main)

