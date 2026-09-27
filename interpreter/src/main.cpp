#include <iostream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "interpreter.hpp"
#include "lexer.hpp"

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc != 2) {
        std::cerr << "Использование: " << (argc > 0 ? argv[0] : "interpreter") << " <файл программы>\n";
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
