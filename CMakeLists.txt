# Minimum CMake version required
cmake_minimum_required(VERSION 3.5)

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Project name (will show in IDF monitor, etc.)
project(esp32_secure_mesh_iot)
