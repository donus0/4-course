#include "zhegalkin.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

namespace {

// Ограничение на число переменных: 2^24 значений — уже 4 Мбайт hex-строки.
constexpr std::size_t max_variables = 24;

std::size_t bit_count(std::size_t x) {
    std::size_t count = 0;
    while (x != 0) {
        x &= x - 1;
        ++count;
    }
    return count;
}

int hex_digit_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

}  // namespace

truth_table parse_hex_truth_table(const std::string& hex) {
    std::string digits;
    for (char c : hex) {
        if (std::isspace(static_cast<unsigned char>(c)) == 0 && c != '_') {
            digits += c;
        }
    }
    if (digits.size() >= 2 && digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X')) {
        digits.erase(0, 2);
    }
    if (digits.empty()) {
        throw std::invalid_argument("пустая таблица истинности");
    }

    // 4 * k бит должно быть равно 2^n, т.е. k — степень двойки.
    std::size_t k = digits.size();
    if ((k & (k - 1)) != 0) {
        throw std::invalid_argument("число hex-цифр (" + std::to_string(k) +
                                    ") не степень двойки: таблица истинности должна содержать 2^n бит");
    }
    std::size_t variables = 2;
    while ((std::size_t{1} << variables) < 4 * k) {
        ++variables;
    }
    if (variables > max_variables) {
        throw std::invalid_argument("слишком много переменных (максимум " + std::to_string(max_variables) + ")");
    }

    truth_table table{variables, {}};
    table.values.reserve(4 * k);
    for (char c : digits) {
        int value = hex_digit_value(c);
        if (value < 0) {
            throw std::invalid_argument(std::string("недопустимый символ '") + c + "'");
        }
        for (int bit = 3; bit >= 0; --bit) {
            table.values.push_back(static_cast<unsigned char>((value >> bit) & 1));
        }
    }
    return table;
}

std::vector<unsigned char> zhegalkin_coefficients(const std::vector<unsigned char>& values) {
    // На шаге step к каждому набору, где переменная step равна 1, прибавляется
    // (по модулю 2) значение на соседнем наборе, где она равна 0. После всех
    // шагов a[m] = XOR f(s) по всем подмножествам s ⊆ m — это и есть
    // коэффициент при мономе m.
    std::vector<unsigned char> a = values;
    for (std::size_t step = 1; step < a.size(); step <<= 1) {
        for (std::size_t i = 0; i < a.size(); ++i) {
            if ((i & step) != 0) {
                a[i] ^= a[i ^ step];
            }
        }
    }
    return a;
}

std::vector<std::vector<unsigned char>> pascal_triangle(const std::vector<unsigned char>& values) {
    std::vector<std::vector<unsigned char>> rows;
    rows.reserve(values.size());
    rows.push_back(values);
    for (std::size_t k = 1; k < values.size(); ++k) {
        const std::vector<unsigned char>& prev = rows.back();
        std::vector<unsigned char> row(prev.size() - 1);
        for (std::size_t j = 0; j < row.size(); ++j) {
            row[j] = prev[j] ^ prev[j + 1];
        }
        rows.push_back(std::move(row));
    }
    return rows;
}

std::string monomial_to_string(std::size_t mask, std::size_t variables) {
    if (mask == 0) {
        return "1";
    }
    std::string result;
    for (std::size_t i = 0; i < variables; ++i) {
        // x1 соответствует старшему биту маски.
        if ((mask >> (variables - 1 - i)) & 1) {
            result += "x" + std::to_string(i + 1);
        }
    }
    return result;
}

std::string polynomial_to_string(const std::vector<unsigned char>& coefficients, std::size_t variables) {
    std::vector<std::size_t> monomials;
    for (std::size_t mask = 0; mask < coefficients.size(); ++mask) {
        if (coefficients[mask] != 0) {
            monomials.push_back(mask);
        }
    }
    if (monomials.empty()) {
        return "0";
    }

    // Сначала по степени; при равной степени лексикографический порядок
    // номеров переменных совпадает с убыванием маски (x1 — старший бит).
    std::sort(monomials.begin(), monomials.end(), [](std::size_t a, std::size_t b) {
        std::size_t degree_a = bit_count(a);
        std::size_t degree_b = bit_count(b);
        if (degree_a != degree_b) {
            return degree_a < degree_b;
        }
        return a > b;
    });

    std::string result;
    for (std::size_t mask : monomials) {
        if (!result.empty()) {
            result += " ⊕ ";
        }
        result += monomial_to_string(mask, variables);
    }
    return result;
}
