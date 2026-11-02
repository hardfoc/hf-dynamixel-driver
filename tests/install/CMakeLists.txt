# Consumer of an installed hf-dynamixel-driver: find_package() plus the hf::dynamixel target.
# CI configures this against the `cmake --install` prefix (CMAKE_PREFIX_PATH).
cmake_minimum_required(VERSION 3.16)
project(hf_dynamixel_install_consumer LANGUAGES CXX)

find_package(hf_dynamixel REQUIRED)

add_executable(hf_dynamixel_install_consumer main.cpp)
target_link_libraries(hf_dynamixel_install_consumer PRIVATE hf::dynamixel)
