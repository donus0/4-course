add_executable(interpreter
    src/main.cpp
    src/lexer.cpp
    src/interpreter.cpp
    src/expression.cpp
)

target_include_directories(interpreter PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_options(interpreter PRIVATE -Wall -Wextra -Wpedantic)
