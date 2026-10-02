#pragma once

#include <string>
#include <vector>

#include "lexer.hpp"


//   exp x,(a+b)*c;   ->   постфикс: a b + c *   ->   set $1,a;
//                                                    sum $1,b;
//                                                    mul $1,c;
//                                                    set x,$1;

// Переводит инфиксное выражение в постфиксную запись. Унарный минус
// записывается токеном "~". Бросает std::runtime_error при синтаксической
// ошибке в выражении. Если trace = true, печатает каждый шаг алгоритма
// (символ, стек операторов, выход).
std::vector<std::string> infix_to_postfix(const std::string& expr, int line, bool trace = false);

// Постфиксная запись одной строкой через пробел: "a b + c *"
std::string postfix_to_string(const std::vector<std::string>& postfix);

// Строит по постфиксной записи команды, вычисляющие выражение и записывающие результат в переменную dst.
std::vector<statement> postfix_to_commands(const std::vector<std::string>& postfix,
                                           const std::string& dst, int line);
