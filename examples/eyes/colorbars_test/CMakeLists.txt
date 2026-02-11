cmake_minimum_required(VERSION 3.16)
# Add grokbot top-level components directory so shared components (e.g. display_eyes)
# can be discovered when building examples under examples/*
set(EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/../../../components")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(colorbars_test)
