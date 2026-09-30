add_executable(expr_tree
    src/main.cpp
    src/expression_tree.cpp
)

target_include_directories(expr_tree PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_options(expr_tree PRIVATE -Wall -Wextra -Wpedantic)
