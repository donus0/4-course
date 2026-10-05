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

interpreter::interpreter(bool debug) : _debug(debug) {
    _operations["set"] = [this](const statement& st) { op_set(st); };
    _operations["exp"] = [this](const statement& st) { op_exp(st); };
    _operations["inp"] = [this](const statement& st) { op_inp(st); };
    _operations["out"] = [this](const statement& st) { op_out(st); };

    add_binary("sum", [](long long a, long long b, int) { return a + b; });
    add_binary("min", [](long long a, long long b, int) { return a - b; });
    add_binary("mul", [](long long a, long long b, int) { return a * b; });
    add_binary("and", [](long long a, long long b, int) { return a & b; });
    add_binary("or", [](long long a, long long b, int) { return a | b; });
    add_binary("xor", [](long long a, long long b, int) { return a ^ b; });
    add_binary("cmp", [](long long a, long long b, int) -> long long { return (a > b) - (a < b); });

    add_binary("div", [this](long long a, long long b, int line) {
        check_divisor(b, line);
        return a / b;
    });
    add_binary("mod", [this](long long a, long long b, int line) {
        check_divisor(b, line);
        return a % b;
    });
    add_binary("shl", [this](long long a, long long b, int line) {
        check_shift(b, line);
        // Сдвиг через unsigned, чтобы сдвиг отрицательного числа влево был определён
        return static_cast<long long>(static_cast<unsigned long long>(a) << b);
    });
    add_binary("shr", [this](long long a, long long b, int line) {
        check_shift(b, line);
        return a >> b;
    });
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

void interpreter::add_binary(const std::string& name, binary_function f) {
    _operations[name] = [this, f](const statement& st) {
        check_command(st, 2);
        long long a = get_value(st.args[0], st.line);
        long long b = get_value(st.args[1], st.line);
        _vars[st.args[0]] = f(a, b, st.line);
    };
}

void interpreter::op_set(const statement& st) {
    check_command(st, 2);
    _vars[st.args[0]] = get_value(st.args[1], st.line);
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
    it->second(st);
}

void interpreter::run(const std::string& source) {
    lexer lex(source);
    statement st;

    while (lex.next(st)) {
        execute(st);
    }
}
