#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "zhegalkin.hpp"

namespace {

// Треугольник печатается вместе с таблицей истинности только для малого n,
// иначе он не помещается на экран.
constexpr std::size_t max_triangle_variables = 4;

void print_triangle(const truth_table& table, const std::vector<std::vector<unsigned char>>& rows) {
    std::size_t n = table.variables;
    std::size_t width = 2 * table.values.size() - 1;

    std::cout << "\nМетод треугольника (левый край строки — коэффициент при мономе):\n";
    for (std::size_t i = 0; i < n; ++i) {
        std::cout << "x" << i + 1 << " ";
    }
    std::cout << "| f | " << std::string(width, ' ') << " | моном\n";

    for (std::size_t k = 0; k < rows.size(); ++k) {
        for (std::size_t i = 0; i < n; ++i) {
            std::cout << " " << ((k >> (n - 1 - i)) & 1) << " ";
        }
        std::cout << "| " << int{table.values[k]} << " | " << std::string(k, ' ');
        for (std::size_t j = 0; j < rows[k].size(); ++j) {
            std::cout << int{rows[k][j]} << (j + 1 < rows[k].size() ? " " : "");
        }
        std::cout << std::string(width - (2 * rows[k].size() - 1) - k, ' ') << " | "
                  << monomial_to_string(k, n) << (rows[k][0] != 0 ? "  ✓" : "") << "\n";
    }
}

void process(const std::string& hex) {
    truth_table table = parse_hex_truth_table(hex);
    std::size_t n = table.variables;

    std::cout << "Переменных: " << n << "\n";
    if (table.values.size() <= 64) {
        std::cout << "Таблица истинности: ";
        for (unsigned char v : table.values) {
            std::cout << int{v};
        }
        std::cout << "\n";
    }

    std::vector<unsigned char> coefficients = zhegalkin_coefficients(table.values);

    if (n <= max_triangle_variables) {
        std::vector<std::vector<unsigned char>> rows = pascal_triangle(table.values);
        print_triangle(table, rows);
        for (std::size_t k = 0; k < rows.size(); ++k) {
            if (rows[k][0] != coefficients[k]) {
                throw std::logic_error("метод треугольника и преобразование Мёбиуса разошлись");
            }
        }
    }

    std::size_t terms = 0;
    std::size_t degree = 0;
    for (std::size_t mask = 0; mask < coefficients.size(); ++mask) {
        if (coefficients[mask] != 0) {
            ++terms;
            std::size_t d = 0;
            for (std::size_t m = mask; m != 0; m &= m - 1) {
                ++d;
            }
            degree = d > degree ? d : degree;
        }
    }

    std::cout << "\nПолином Жегалкина:\nf(";
    for (std::size_t i = 0; i < n; ++i) {
        std::cout << (i > 0 ? ", " : "") << "x" << i + 1;
    }
    std::cout << ") = " << polynomial_to_string(coefficients, n) << "\n";
    std::cout << "Слагаемых: " << terms << ", степень: " << degree << "\n";

    // Преобразование Мёбиуса — инволюция: значения полинома на всех наборах
    // получаются тем же преобразованием коэффициентов.
    bool ok = zhegalkin_coefficients(coefficients) == table.values;
    std::cout << "Проверка по таблице истинности: " << (ok ? "совпадает" : "ОШИБКА") << "\n";
}

}  // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc > 1) {
        int status = 0;
        for (int i = 1; i < argc; ++i) {
            std::cout << (i > 1 ? "\n" : "") << "f = " << argv[i] << "\n";
            try {
                process(argv[i]);
            } catch (const std::exception& e) {
                std::cout << "Ошибка: " << e.what() << "\n";
                status = 1;
            }
        }
        return status;
    }

    std::string line;
    while (true) {
        std::cout << "\nТаблица истинности в hex (например 6, 96, E8, 0x6996; пусто — выход): ";
        if (!std::getline(std::cin, line) || line.empty()) {
            break;
        }
        try {
            process(line);
        } catch (const std::exception& e) {
            std::cout << "Ошибка: " << e.what() << "\n";
        }
    }

    return 0;
}
