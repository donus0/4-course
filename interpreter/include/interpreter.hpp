#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "lexer.hpp"

class interpreter {
private:
    // Функция команды вида "op x,y;": по значениям x и y возвращает новое x
    using binary_function = std::function<long long(long long a, long long b, int line)>;

    // Значения переменных: имя -> число
    std::unordered_map<std::string, long long> _vars;

    // Имя операции в тексте программы -> функция, которая её выполняет
    std::unordered_map<std::string, std::function<void(const statement&)>> _operations;

    // Режим трассировки (флаг -d): exp печатает перевод выражения в постфикс
    bool _debug;

    void error(int line, const std::string& message) const;
    void check_args_count(const statement& st, std::size_t count) const;
    void check_command(const statement& st, std::size_t count) const;
    void check_divisor(long long b, int line) const;
    void check_shift(long long b, int line) const;
    long long get_value(const std::string& arg, int line) const;

    void add_binary(const std::string& name, binary_function f);
    void execute(const statement& st);
    void trace_command(const statement& command) const;

    void op_set(const statement& st);
    void op_exp(const statement& st);
    void op_inp(const statement& st);
    void op_out(const statement& st);

public:
    interpreter(bool debug = false);

    // Лямбды в _operations хранят указатель this, поэтому копировать нельзя
    interpreter(const interpreter&) = delete;
    interpreter& operator=(const interpreter&) = delete;

    void run(const std::string& source);
};
