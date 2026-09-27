#pragma once

#include <string>
#include <unordered_map>

#include "lexer.hpp"

class interpreter {
private:
    using handler_t = void (interpreter::*)(const statement&);

    std::unordered_map<std::string, long long> vars_;

    long long resolve(const std::string& arg, int line) const;
    void exec(const statement& st);

    void exec_binary(const statement& st, long long (*compute)(long long lhs, long long rhs, int line));

    void op_set(const statement& st);
    void op_sum(const statement& st);
    void op_min(const statement& st);
    void op_mul(const statement& st);
    void op_div(const statement& st);
    void op_mod(const statement& st);
    void op_inp(const statement& st);
    void op_out(const statement& st);

    static const std::unordered_map<std::string, handler_t> handlers_;

public:
    void run(const std::string& source);
};
