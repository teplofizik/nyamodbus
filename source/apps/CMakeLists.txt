cmake_minimum_required(VERSION 3.10) # Проверка версии CMake.
project(nyamodbus_apps C)

add_executable(parser_test parser_test.c)
target_link_libraries(parser_test nyamodbus)
