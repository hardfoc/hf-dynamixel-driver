cmake_minimum_required(VERSION 3.16)

set(DEFAULT_APP_TYPE "dynamixel_readonly_example" CACHE STRING "Default app type")
set(DEFAULT_BUILD_TYPE "Debug" CACHE STRING "Default build type")
set(DEFAULT_SOURCE_FILE "dynamixel_readonly_example.cpp" CACHE STRING "Default source")

if(NOT DEFINED APP_TYPE)
    message(FATAL_ERROR
        "APP_TYPE not defined. Use build_app.sh from examples/esp32/scripts.\n"
        "Example: ./scripts/build_app.sh ${DEFAULT_APP_TYPE} ${DEFAULT_BUILD_TYPE}")
endif()

if(NOT DEFINED BUILD_TYPE)
    message(FATAL_ERROR "BUILD_TYPE not defined. Use build_app.sh.")
endif()

set(APP_TYPE "${APP_TYPE}" CACHE STRING "App type to build" FORCE)
set(BUILD_TYPE "${BUILD_TYPE}" CACHE STRING "Build type (Debug/Release)" FORCE)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_dynamixel_${APP_TYPE}_app)
