#===============================================================================
# hf-dynamixel-driver — Build Settings
#===============================================================================

include_guard(GLOBAL)

set(HF_DYNAMIXEL_TARGET_NAME "hf_dynamixel")

set(HF_DYNAMIXEL_VERSION_MAJOR 0)
set(HF_DYNAMIXEL_VERSION_MINOR 4)
set(HF_DYNAMIXEL_VERSION_PATCH 0)
set(HF_DYNAMIXEL_VERSION
    "${HF_DYNAMIXEL_VERSION_MAJOR}.${HF_DYNAMIXEL_VERSION_MINOR}.${HF_DYNAMIXEL_VERSION_PATCH}")
set(HF_DYNAMIXEL_VERSION_STRING "${HF_DYNAMIXEL_VERSION}")

set(HF_DYNAMIXEL_SDK_TAG "4.1.0")
set(HF_DYNAMIXEL_SDK_COMMIT "f838bc90f72fcf5b9c279432d6e12fc24969daa8")

set(HF_DYNAMIXEL_VERSION_TEMPLATE
    "${CMAKE_CURRENT_LIST_DIR}/../inc/dynamixel_version.h.in")
set(HF_DYNAMIXEL_VERSION_HEADER_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/hf_dynamixel_generated")
set(HF_DYNAMIXEL_VERSION_HEADER
    "${HF_DYNAMIXEL_VERSION_HEADER_DIR}/dynamixel_version.h")

file(MAKE_DIRECTORY "${HF_DYNAMIXEL_VERSION_HEADER_DIR}")

if(EXISTS "${HF_DYNAMIXEL_VERSION_TEMPLATE}")
    configure_file(
        "${HF_DYNAMIXEL_VERSION_TEMPLATE}"
        "${HF_DYNAMIXEL_VERSION_HEADER}"
        @ONLY
    )
else()
    message(WARNING "dynamixel_version.h.in not found at ${HF_DYNAMIXEL_VERSION_TEMPLATE}")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/hf_dynamixel_sdk_sources.cmake")

set(HF_DYNAMIXEL_PUBLIC_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../inc"
    "${HF_DYNAMIXEL_VERSION_HEADER_DIR}"
)

set(HF_DYNAMIXEL_PRIVATE_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../src"
    "${HF_DYNAMIXEL_SDK_INCLUDE_DIR}"
)

set(HF_DYNAMIXEL_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/../src/dynamixel_status.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/dynamixel_bus.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/dynamixel_device.cpp"
    ${HF_DYNAMIXEL_SDK_SOURCE_FILES}
)

set(HF_DYNAMIXEL_IDF_REQUIRES driver esp_timer)
