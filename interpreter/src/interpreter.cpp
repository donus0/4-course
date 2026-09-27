#include "interpreter.hpp"

#include <cctype>
#include <iostream>
#include <stdexcept>

// Поддерживаемые операции:
//   "set x,y;"    x = y              (y — число или переменная)
//   "sum x,y;"    x = x + y
//   "min x,y;"    x = x - y
//   "mul x,y;"    x = x * y
//   "div x,y;"    x = x / y          (целочисленное деление)
//   "mod x,y;"    x = x % y          (остаток от деления)
//   "inp x;"      x = <число с stdin>
//   "out x;"      вывод значения x (число или переменная) и перевод строки

[[noreturn]] static void fail(int line, const std::string& message) {
    throw std::runtime_error("Ошибка выполнения (строка " + std::to_string(line) + "): " + message);
}

static bool is_integer_literal(const std::string& s) {
    std::size_t i = (!s.empty() && s[0] == '-') ? 1 : 0;
    if (i == s.size()) {
        return false;
    }
    for (; i < s.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
            return false;
        }
    }
    return true;
}

static void expect_arg_count(const statement& st, std::size_t expected) {
    if (st.args.size() != expected) {
        fail(st.line, "операция '" + st.op + "' ожидает " + std::to_string(expected) +
                           " аргумент(ов), получено " + std::to_string(st.args.size()));
    }
}

static void expect_identifier(const std::string& arg, int line) {
    if (is_integer_literal(arg) || arg.empty()) {
        fail(line, "ожидалось имя переменной, получено '" + arg + "'");
    }
}

long long interpreter::resolve(const std::string& arg, int line) const {
    if (is_integer_literal(arg)) {
        return std::stoll(arg);
    }

    auto it = vars_.find(arg);
    if (it == vars_.end()) {
        fail(line, "переменная '" + arg + "' не определена");
    }
    return it->second;
}

void interpreter::exec_binary(const statement& st, long long (*compute)(long long, long long, int)) {
    expect_arg_count(st, 2);
    expect_identifier(st.args[0], st.line);

    long long lhs = resolve(st.args[0], st.line);
    long long rhs = resolve(st.args[1], st.line);
    vars_[st.args[0]] = compute(lhs, rhs, st.line);
}

void interpreter::op_set(const statement& st) {
    expect_arg_count(st, 2);
    expect_identifier(st.args[0], st.line);
    vars_[st.args[0]] = resolve(st.args[1], st.line);
}

void interpreter::op_sum(const statement& st) {
    exec_binary(st, [](long long a, long long b, int) { return a + b; });
}

void interpreter::op_min(const statement& st) {
    exec_binary(st, [](long long a, long long b, int) { return a - b; });
}

void interpreter::op_mul(const statement& st) {
    exec_binary(st, [](long long a, long long b, int) { return a * b; });
}

void interpreter::op_div(const statement& st) {
    exec_binary(st, [](long long a, long long b, int line) -> long long {
        if (b == 0) {
            fail(line, "деление на ноль");
        }
        return a / b;
    });
}

void interpreter::op_mod(const statement& st) {
    exec_binary(st, [](long long a, long long b, int line) -> long long {
        if (b == 0) {
            fail(line, "деление на ноль");
        }
        return a % b;
    });
}

void interpreter::op_inp(const statement& st) {
    expect_arg_count(st, 1);
    expect_identifier(st.args[0], st.line);

    std::string line;
    if (!std::getline(std::cin, line)) {
        fail(st.line, "не удалось прочитать целое число со стандартного ввода");
    }
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
        line.pop_back();
    }
    if (!is_integer_literal(line)) {
        fail(st.line, "ожидалось целое число, получено '" + line + "'");
    }

    vars_[st.args[0]] = std::stoll(line);
}

void interpreter::op_out(const statement& st) {
    expect_arg_count(st, 1);
    std::cout << resolve(st.args[0], st.line) << "\n";
}

const std::unordered_map<std::string, interpreter::handler_t> interpreter::handlers_ = {
    {"set", &interpreter::op_set},
    {"sum", &interpreter::op_sum},
    {"min", &interpreter::op_min},
    {"mul", &interpreter::op_mul},
    {"div", &interpreter::op_div},
    {"mod", &interpreter::op_mod},
    {"inp", &interpreter::op_inp},
    {"out", &interpreter::op_out},
};

void interpreter::exec(const statement& st) {
    auto it = handlers_.find(st.op);
    if (it == handlers_.end()) {
        fail(st.line, "неизвестная операция '" + st.op + "'");
    }
    (this->*(it->second))(st);
}

void interpreter::run(const std::string& source) {
    std::size_t pos = 0;
    int line = 1;
    statement st;

    while (next_statement(source, pos, line, st)) {
        exec(st);
    }
}
