add_executable(hf_dynamixel_host_tests dynamixel_driver_test.cpp)
target_link_libraries(hf_dynamixel_host_tests PRIVATE hf::dynamixel)
target_include_directories(hf_dynamixel_host_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(hf_dynamixel_host_tests PRIVATE -Wall -Wextra -Wpedantic)
target_compile_features(hf_dynamixel_host_tests PRIVATE cxx_std_17)
add_test(NAME hf_dynamixel_host_tests COMMAND hf_dynamixel_host_tests)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    option(HF_DYNAMIXEL_ENABLE_SANITIZERS "Address/UB sanitizers for host tests" ON)
    if(HF_DYNAMIXEL_ENABLE_SANITIZERS)
        target_compile_options(hf_dynamixel_host_tests PRIVATE -fsanitize=address,undefined)
        target_link_options(hf_dynamixel_host_tests PRIVATE -fsanitize=address,undefined)
    endif()
endif()

# The code shown on the documentation site (docs/ includes these files with pymdownx.snippets), built so a
# page cannot show code that does not compile against the current headers. Compiled, never linked or run.
add_library(hf_dynamixel_doc_snippets OBJECT
    ../docs/quickstart.cpp
    ../docs/api_tasks.cpp
    ../docs/configuration.cpp
    ../docs/ports.cpp
)
target_link_libraries(hf_dynamixel_doc_snippets PRIVATE hf::dynamixel)
target_include_directories(hf_dynamixel_doc_snippets PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../docs)
target_compile_features(hf_dynamixel_doc_snippets PRIVATE cxx_std_17)
target_compile_options(hf_dynamixel_doc_snippets PRIVATE -Wall -Wextra -Wpedantic)
