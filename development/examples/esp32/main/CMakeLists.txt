if(NOT DEFINED APP_TYPE)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(APP_TYPE "${DEFAULT_APP_TYPE}")
    else()
        message(FATAL_ERROR "APP_TYPE not defined — use build_app.sh")
    endif()
endif()

if(NOT DEFINED APP_SOURCE_FILE)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(APP_SOURCE_FILE "${DEFAULT_SOURCE_FILE}")
    else()
        message(FATAL_ERROR "APP_SOURCE_FILE not defined — use build_app.sh")
    endif()
endif()

idf_component_register(
    SRCS "${APP_SOURCE_FILE}"
    INCLUDE_DIRS "." "include"
    REQUIRES driver freertos esp_driver_uart hf_dynamixel
)

target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)
target_compile_definitions(${COMPONENT_LIB} PRIVATE "EXAMPLE_TYPE_${APP_TYPE}=1")
