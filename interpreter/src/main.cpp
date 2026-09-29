#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "interpreter.hpp"

// Читает весь файл в одну строку
std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("не удалось открыть файл: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char** argv) {
#ifdef _WIN32
    // Чтобы русские сообщения нормально выводились в консоли Windows
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc != 2) {
        std::cerr << "Использование: interpreter <файл программы>\n";
        return 1;
    }

    try {
        std::string source = read_file(argv[1]);

        interpreter interp;
        interp.run(source);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
