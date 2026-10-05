#include "interpreter.hpp"
#include "expression.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>

bool is_number(const std::string& s) {
    std::size_t start = 0;
    if (!s.empty() && s[0] == '-') {
        start = 1;
    }
    if (start == s.size()) {
        return false;
    }
    for (std::size_t i = start; i < s.size(); i++) {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) {
            return false;
        }
    }
    return true;
}

interpreter::interpreter(bool debug) {
    _debug = debug;

    _operations["set"] = &interpreter::op_set;
    _operations["sum"] = &interpreter::op_sum;
    _operations["min"] = &interpreter::op_min;
    _operations["mul"] = &interpreter::op_mul;
    _operations["div"] = &interpreter::op_div;
    _operations["mod"] = &interpreter::op_mod;
    _operations["and"] = &interpreter::op_and;
    _operations["or"] = &interpreter::op_or;
    _operations["xor"] = &interpreter::op_xor;
    _operations["shl"] = &interpreter::op_shl;
    _operations["shr"] = &interpreter::op_shr;
    _operations["cmp"] = &interpreter::op_cmp;
    _operations["exp"] = &interpreter::op_exp;
    _operations["inp"] = &interpreter::op_inp;
    _operations["out"] = &interpreter::op_out;
}

void interpreter::error(int line, const std::string& message) const {
    throw std::runtime_error("Ошибка выполнения (строка " + std::to_string(line) + "): " + message);
}

void interpreter::check_args_count(const statement& st, std::size_t count) const {
    if (st.args.size() != count) {
        error(st.line, "операция '" + st.op + "' ожидает " + std::to_string(count) +
                           " аргумент(ов), получено " + std::to_string(st.args.size()));
    }
}

void interpreter::check_command(const statement& st, std::size_t count) const {
    check_args_count(st, count);
    if (st.args[0].empty() || is_number(st.args[0])) {
        error(st.line, "ожидалось имя переменной, получено '" + st.args[0] + "'");
    }
}

void interpreter::check_divisor(long long b, int line) const {
    if (b == 0) {
        error(line, "деление на ноль");
    }
}

void interpreter::check_shift(long long b, int line) const {
    if (b < 0 || b >= 64) {
        error(line, "величина сдвига должна быть от 0 до 63, получено " + std::to_string(b));
    }
}

long long interpreter::get_value(const std::string& arg, int line) const {
    if (is_number(arg)) {
        return std::stoll(arg);
    }
    auto it = _vars.find(arg);
    if (it == _vars.end()) {
        error(line, "переменная '" + arg + "' не определена");
    }
    return it->second;
}

// Общая часть команд вида "op x,y;": проверяет команду и достаёт значения x и y
void interpreter::read_operands(const statement& st, long long& a, long long& b) const {
    check_command(st, 2);
    a = get_value(st.args[0], st.line);
    b = get_value(st.args[1], st.line);
}

void interpreter::op_set(const statement& st) {
    check_command(st, 2);
    _vars[st.args[0]] = get_value(st.args[1], st.line);
}

void interpreter::op_sum(const statement& st) {
    long long a, b;
    read_operands(st, a, b);
    _vars[st.args[0]] = a + b;
}

void interpreter::op_min(const statement& st) {
    long long a, b;
    read_operands(st, a, b);
    _vars[st.args[0]] = a - b;
}

void interpreter::op_mul(const statement& st) {
    long long a, b;
    read_operands(st, a, b);
    _vars[st.args[0]] = a * b;
}

void interpreter::op_div(const statement& st) {
    long long a, b;
    read_operands(st, a, b);
    check_divisor(b, st.line);
    _vars[st.args[0]] = a / b;
}

void interpreter::op_mod(const statement& st) {
    long long a, b;
    read_operands(st, a, b);
    check_divisor(b, st.line);
    _vars[st.args[0]] = a % b;
}

void interpreter::op_and(const statement& st) {
    long long a, b;
    read_operands(st, a, b);
    _vars[st.args[0]] = a & b;
}

void interpreter::op_or(const statement& st) {
    long long a, b;
    read_operands(st, a, b);
    _vars[st.args[0]] = a | b;
}

void interpreter::op_xor(const statement& st) {
    long long a, b;
    read_operands(st, a, b);
    _vars[st.args[0]] = a ^ b;
}

void interpreter::op_shl(const statement& st) {
    long long a, b;
    read_operands(st, a, b);
    check_shift(b, st.line);
    // Сдвиг через unsigned, чтобы сдвиг отрицательного числа влево был определён
    _vars[st.args[0]] = static_cast<long long>(static_cast<unsigned long long>(a) << b);
}

void interpreter::op_shr(const statement& st) {
    long long a, b;
    read_operands(st, a, b);
    check_shift(b, st.line);
    _vars[st.args[0]] = a >> b;
}

void interpreter::op_cmp(const statement& st) {
    long long a, b;
    read_operands(st, a, b);
    if (a < b) {
        _vars[st.args[0]] = -1;
    } else if (a > b) {
        _vars[st.args[0]] = 1;
    } else {
        _vars[st.args[0]] = 0;
    }
}

void interpreter::trace_command(const statement& command) const {
    const std::string& target = command.args[0];
    std::string text = command.op + " " + target + "," + command.args[1] + ";";
    text.resize(std::max<std::size_t>(text.size() + 2, 16), ' ');
    std::cout << "    " << text << target << " = " << _vars.at(target) << "\n";
}

void interpreter::op_exp(const statement& st) {
    check_command(st, 2);

    if (_debug) {
        std::cout << "[трассировка] строка " << st.line << ": exp " << st.args[0] << "," << st.args[1]
                  << "\n  (~ — унарный минус)\n";
    }

    std::vector<std::string> postfix = infix_to_postfix(st.args[1], st.line, _debug);
    std::vector<statement> commands = postfix_to_commands(postfix, st.args[0], st.line);

    if (_debug) {
        std::cout << "  постфикс: " << postfix_to_string(postfix) << "\n  команды:\n";
    }
    for (const statement& command : commands) {
        execute(command);
        if (_debug) {
            trace_command(command);
        }
    }

    for (const statement& command : commands) {
        if (command.args[0][0] == '$') {
            _vars.erase(command.args[0]);
        }
    }
}

void interpreter::op_inp(const statement& st) {
    check_command(st, 1);

    std::string line;
    if (!std::getline(std::cin, line)) {
        error(st.line, "не удалось прочитать целое число с клавиатуры");
    }
    // Убираем перевод строки, который мог остаться в конце
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
        line.pop_back();
    }
    if (!is_number(line)) {
        error(st.line, "ожидалось целое число, получено '" + line + "'");
    }

    _vars[st.args[0]] = std::stoll(line);
}

void interpreter::op_out(const statement& st) {
    check_args_count(st, 1);
    std::cout << get_value(st.args[0], st.line) << "\n";
}

void interpreter::execute(const statement& st) {
    auto it = _operations.find(st.op);
    if (it == _operations.end()) {
        error(st.line, "неизвестная операция '" + st.op + "'");
    }
    // it->second — метод из таблицы, вызываем его у текущего объекта
    (this->*it->second)(st);
}

void interpreter::run(const std::string& source) {
    lexer lex(source);
    statement st;

    while (lex.next(st)) {
        execute(st);
    }
}
