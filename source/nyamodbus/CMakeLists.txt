cmake_minimum_required(VERSION 3.10)
project(nyamodbus C)

set(SOURCES nyamodbus.c
            nyamodbus_master.c
            nyamodbus_slave.c
			nyamodbus_utils.c)
set(HEADERS nyamodbus.h
            nyamodbus_master.h
            nyamodbus_slave.h
			nyamodbus_utils.h)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
