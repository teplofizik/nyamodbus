cmake_minimum_required(VERSION 3.10) # �������� ������ CMake.
project(nyamodbus_apps C)

add_executable(parser_test parser_test.c)
target_link_libraries(parser_test nyamodbus emulator)

add_executable(master_slave master_slave.c)
target_link_libraries(master_slave nyamodbus emulator)

add_executable(master master.c)
target_link_libraries(master nyamodbus serial)
