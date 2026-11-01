# Explicit DynamixelSDK 4.1.0 sources. No globs. No stock platform ports.
include_guard(GLOBAL)

if(NOT DEFINED HF_DYNAMIXEL_SDK_ROOT)
    set(HF_DYNAMIXEL_SDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../external/DynamixelSDK")
endif()

set(HF_DYNAMIXEL_SDK_INCLUDE_DIR
    "${HF_DYNAMIXEL_SDK_ROOT}/c++/include/dynamixel_sdk")

set(HF_DYNAMIXEL_SDK_SOURCE_FILES
    "${HF_DYNAMIXEL_SDK_ROOT}/c++/src/dynamixel_sdk/protocol2_packet_handler.cpp"
    "${HF_DYNAMIXEL_SDK_ROOT}/c++/src/dynamixel_sdk/group_handler.cpp"
    "${HF_DYNAMIXEL_SDK_ROOT}/c++/src/dynamixel_sdk/group_sync_read.cpp"
    "${HF_DYNAMIXEL_SDK_ROOT}/c++/src/dynamixel_sdk/group_sync_write.cpp"
)

# SDK .cpp files only include their headers when __linux__/__APPLE__/WIN32/ARDUINO
# is defined. Force the Linux include path on non-desktop toolchains (ESP-IDF).
set(HF_DYNAMIXEL_SDK_COMPILE_DEFINITIONS "")
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin"
   AND NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND HF_DYNAMIXEL_SDK_COMPILE_DEFINITIONS __linux__)
endif()
