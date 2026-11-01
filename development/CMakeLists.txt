# =============================================================================
# hf-dynamixel-driver — Root CMakeLists.txt
# =============================================================================
# Portable Protocol 2.0 wrapper around the official ROBOTIS Dynamixel SDK.
# Public headers stay SDK-free. Integrate via this file or
# cmake/hf_dynamixel_build_settings.cmake.
# =============================================================================
cmake_minimum_required(VERSION 3.16)

project(hf_dynamixel
  VERSION 0.4.0
  LANGUAGES CXX
)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/hf_dynamixel_build_settings.cmake)

add_library(${HF_DYNAMIXEL_TARGET_NAME} STATIC
    ${HF_DYNAMIXEL_SOURCE_FILES}
)
target_include_directories(${HF_DYNAMIXEL_TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<BUILD_INTERFACE:${HF_DYNAMIXEL_VERSION_HEADER_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE ${HF_DYNAMIXEL_PRIVATE_INCLUDE_DIRS}
)
target_compile_features(${HF_DYNAMIXEL_TARGET_NAME} PUBLIC cxx_std_17)
target_compile_definitions(${HF_DYNAMIXEL_TARGET_NAME}
    PRIVATE ${HF_DYNAMIXEL_SDK_COMPILE_DEFINITIONS}
)
set_target_properties(${HF_DYNAMIXEL_TARGET_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

add_library(hf::dynamixel ALIAS ${HF_DYNAMIXEL_TARGET_NAME})

option(HF_DYNAMIXEL_ENABLE_WARNINGS "Enable compiler warnings for the driver" ON)
if(HF_DYNAMIXEL_ENABLE_WARNINGS)
  target_compile_options(${HF_DYNAMIXEL_TARGET_NAME}
      PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(HF_DYNAMIXEL_BUILD_TESTS "Build host unit tests (scripted transport)" OFF)
if(HF_DYNAMIXEL_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests/host)
endif()

option(HF_DYNAMIXEL_BUILD_HOST_EXAMPLE "Build the host ping/read example" OFF)
if(HF_DYNAMIXEL_BUILD_HOST_EXAMPLE)
  add_subdirectory(examples/host)
endif()

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/hf_dynamixelConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/hf_dynamixelConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/hf_dynamixelConfig.cmake"
  INSTALL_DESTINATION lib/cmake/hf_dynamixel
)

install(TARGETS ${HF_DYNAMIXEL_TARGET_NAME}
    EXPORT hf_dynamixelTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(DIRECTORY inc/ DESTINATION include
    PATTERN "*.in" EXCLUDE)
install(FILES "${HF_DYNAMIXEL_VERSION_HEADER}" DESTINATION include)
install(EXPORT hf_dynamixelTargets
    FILE hf_dynamixelTargets.cmake
    NAMESPACE hf::
    DESTINATION lib/cmake/hf_dynamixel
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/hf_dynamixelConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/hf_dynamixelConfigVersion.cmake"
    DESTINATION lib/cmake/hf_dynamixel
)
