cmake_minimum_required(VERSION 3.10)
project(emulator C)

set(SOURCES emucontacts.c)
set(HEADERS emulator.h)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
