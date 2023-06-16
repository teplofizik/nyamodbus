cmake_minimum_required(VERSION 3.10) # Проверка версии CMake.
project(nyamodbuslib)

include_directories(source)

# Библиотека модбаса
add_subdirectory(source/nyamodbus)

# Приложения для тестов
add_subdirectory(source/apps)
