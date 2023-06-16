cmake_minimum_required(VERSION 3.10) # Проверка версии CMake.
project(nyamodbus C)

set(SOURCES nyamodbus.c)
set(HEADERS nyamodbus.h)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
