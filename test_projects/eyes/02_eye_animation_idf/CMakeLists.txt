cmake_minimum_required(VERSION 3.16)

# Set target to ESP32-S3 for ESP32-S3-WROOM-N16R8
set(IDF_TARGET "esp32s3" CACHE STRING "ESP-IDF target")
set(EXTRA_COMPONENT_DIRS "../components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(eye_animation_idf)
