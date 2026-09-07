add_executable(postfix
    src/main.cpp
    src/InfixToPostfix.cpp
    src/PostfixEval.cpp
)

target_include_directories(postfix PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_options(postfix PRIVATE -Wall -Wextra -Wpedantic)
