cmake_minimum_required(VERSION 3.10)
project(emulator C)

set(SOURCES emucontacts.c
            emulator.c
			emumaster.c)
set(HEADERS emulator.h
            emumaster.h)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} nyamodbus pthread)
