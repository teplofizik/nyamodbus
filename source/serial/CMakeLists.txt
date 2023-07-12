cmake_minimum_required(VERSION 3.10)
project(serial C)

set(SOURCES serial.c)
set(HEADERS serial.h)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} nyamodbus pthread)
