add_executable(zhegalkin
    src/main.cpp
    src/zhegalkin.cpp
)

target_include_directories(zhegalkin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_compile_options(zhegalkin PRIVATE -Wall -Wextra -Wpedantic)
