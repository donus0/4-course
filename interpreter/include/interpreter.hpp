#pragma once

#include <string>
#include <unordered_map>

#include "lexer.hpp"

class interpreter {
private:
    std::unordered_map<std::string, long long> _vars;

    // Имя операции -> метод этого класса, который её выполняет
    std::unordered_map<std::string, void (interpreter::*)(const statement&)> _operations;

    // Таблица переходов: смещение команды в тексте -> куда прыгнуть.
    // Для "if"/"while" прыжок выполняется, когда условие ложно,
    // для "}" (конец ветки if или тела while) — всегда.
    std::unordered_map<std::size_t, position> _jumps;

    bool _debug;

    void build_jumps(const std::string& source);

    void error(int line, const std::string& message) const;
    void check_args_count(const statement& st, std::size_t count) const;
    void check_command(const statement& st, std::size_t count) const;
    void check_divisor(long long b, int line) const;
    void check_shift(long long b, int line) const;
    long long get_value(const std::string& arg, int line) const;
    void read_operands(const statement& st, long long& a, long long& b) const;

    void execute(const statement& st);
    void trace_command(const statement& command) const;

    void op_set(const statement& st);
    void op_sum(const statement& st);
    void op_min(const statement& st);
    void op_mul(const statement& st);
    void op_div(const statement& st);
    void op_mod(const statement& st);
    void op_and(const statement& st);
    void op_or(const statement& st);
    void op_xor(const statement& st);
    void op_not(const statement& st);
    void op_shl(const statement& st);
    void op_shr(const statement& st);
    void op_cmp(const statement& st);
    void op_exp(const statement& st);
    void op_inp(const statement& st);
    void op_out(const statement& st);

public:
    interpreter(bool debug = false);

    void run(const std::string& source);
};
