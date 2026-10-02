#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "lexer.hpp"

class interpreter {
private:
    // Значения переменных: имя -> число
    std::unordered_map<std::string, long long> _vars;

    // Имя операции в тексте программы -> функция, которая её выполняет
    std::unordered_map<std::string, std::function<void(const statement&)>> _operations;

    void error(int line, const std::string& message) const;
    void check_args_count(const statement& st, std::size_t count) const;
    void check_is_variable(const std::string& arg, int line) const;
    long long get_value(const std::string& arg, int line) const;

    void execute(const statement& st);
    void op_set(const statement& st);
    void op_sum(const statement& st);
    void op_min(const statement& st);
    void op_mul(const statement& st);
    void op_div(const statement& st);
    void op_mod(const statement& st);
    void op_and(const statement& st);
    void op_or(const statement& st);
    void op_xor(const statement& st);
    void op_shl(const statement& st);
    void op_shr(const statement& st);
    void op_inp(const statement& st);
    void op_out(const statement& st);

public:
    interpreter();

    void run(const std::string& source);
};
