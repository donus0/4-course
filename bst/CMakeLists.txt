add_executable(bst
    src/main.cpp
)

target_include_directories(bst PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_options(bst PRIVATE -Wall -Wextra -Wpedantic)
