#pragma once

#include <cstddef>
#include <string>
#include <vector>

// Булева функция f(x1, ..., xn), заданная таблицей истинности:
// values[i] — значение f на наборе, двоичная запись i которого (x1 — старший
// бит) задаёт значения переменных. values.size() == 2^variables.
struct truth_table final {
    std::size_t variables;
    std::vector<unsigned char> values;
};

// Разбирает шестнадцатеричную запись столбца значений таблицы истинности.
// Старший бит первой цифры — f(0,...,0), младший бит последней — f(1,...,1).
// Число цифр должно быть степенью двойки (1, 2, 4, ...), т.е. n >= 2.
// Допускаются префикс 0x, пробелы и '_' между цифрами.
// При ошибке бросает std::invalid_argument.
truth_table parse_hex_truth_table(const std::string& hex);

// Коэффициенты полинома Жегалкина (преобразование Мёбиуса над GF(2), O(n * 2^n)):
// coefficients[m] == 1 <=> моном из переменных, отмеченных единицами в m,
// входит в полином. Преобразование — инволюция: применённое к коэффициентам,
// оно вновь даёт таблицу истинности.
std::vector<unsigned char> zhegalkin_coefficients(const std::vector<unsigned char>& values);

// Треугольник Паскаля по модулю 2 (метод треугольника): rows[0] — таблица
// истинности, rows[k][j] = rows[k-1][j] ^ rows[k-1][j+1]. Левый край
// rows[k][0] — коэффициент при мономе с маской k. Используется для наглядности.
std::vector<std::vector<unsigned char>> pascal_triangle(const std::vector<unsigned char>& values);

// Моном по маске: "1", "x2", "x1x3", ...
std::string monomial_to_string(std::size_t mask, std::size_t variables);

// Полином в виде "1 ⊕ x1 ⊕ x2x3"; мономы упорядочены по степени, а внутри
// одной степени — лексикографически. Нулевой полином — "0".
std::string polynomial_to_string(const std::vector<unsigned char>& coefficients, std::size_t variables);
