add_executable(hf_dynamixel_host_ping_read ping_read.cpp)
target_link_libraries(hf_dynamixel_host_ping_read PRIVATE hf::dynamixel)
target_include_directories(hf_dynamixel_host_ping_read PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/host)
target_compile_features(hf_dynamixel_host_ping_read PRIVATE cxx_std_17)
