set(HF_DYNAMIXEL_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../.." CACHE PATH
    "Path to hf-dynamixel-driver root")

get_filename_component(HF_DYNAMIXEL_ROOT "${HF_DYNAMIXEL_ROOT}" ABSOLUTE)
if(NOT EXISTS "${HF_DYNAMIXEL_ROOT}/cmake/hf_dynamixel_build_settings.cmake")
    message(FATAL_ERROR
        "Cannot find hf-dynamixel-driver root at ${HF_DYNAMIXEL_ROOT}")
endif()

include("${HF_DYNAMIXEL_ROOT}/cmake/hf_dynamixel_build_settings.cmake")

idf_component_register(
    SRCS ${HF_DYNAMIXEL_SOURCE_FILES}
    INCLUDE_DIRS ${HF_DYNAMIXEL_PUBLIC_INCLUDE_DIRS}
    PRIV_INCLUDE_DIRS ${HF_DYNAMIXEL_PRIVATE_INCLUDE_DIRS}
    REQUIRES ${HF_DYNAMIXEL_IDF_REQUIRES}
)

target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)
target_compile_definitions(${COMPONENT_LIB}
    PRIVATE ${HF_DYNAMIXEL_SDK_COMPILE_DEFINITIONS})
