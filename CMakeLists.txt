cmake_minimum_required(VERSION 3.10) # �������� ������ CMake.
project(nyamodbuslib)

include_directories(source)

# ���������� �������
add_subdirectory(source/nyamodbus)

# �������� ������-���������
add_subdirectory(source/emulator)

# ���������������� ����
add_subdirectory(source/serial)

# ���������� ��� ������
add_subdirectory(source/apps)
