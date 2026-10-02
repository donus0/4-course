#include "interpreter.hpp"
#include "expression.hpp"

#include <algorithm>
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
//   "and x,y;"    x = x & y          (побитовое И)
//   "or x,y;"     x = x | y          (побитовое ИЛИ)
//   "xor x,y;"    x = x ^ y          (побитовое исключающее ИЛИ)
//   "shl x,y;"    x = x << y         (сдвиг влево, 0 <= y < 64)
//   "shr x,y;"    x = x >> y         (сдвиг вправо, 0 <= y < 64)
//   "cmp x,y;"    x = -1, если x < y; 0, если x == y; 1, если x > y
//   "exp x,e;"    x = значение выражения e, например "exp x,(a+b)*-c%7;"
//   "inp x;"      x = <число с клавиатуры>
//   "out x;"      вывод значения x (число или переменная) и перевод строки

// Проверяет, что строка — целое число, например "42" или "-7"
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

// Заполняем таблицу операций. [this] нужен, чтобы внутри лямбды
// можно было вызвать метод этого объекта (op_set, op_sum, ...).
interpreter::interpreter(bool debug) : _debug(debug) {
    _operations["set"] = [this](const statement& st) { op_set(st); };
    _operations["sum"] = [this](const statement& st) { op_sum(st); };
    _operations["min"] = [this](const statement& st) { op_min(st); };
    _operations["mul"] = [this](const statement& st) { op_mul(st); };
    _operations["div"] = [this](const statement& st) { op_div(st); };
    _operations["mod"] = [this](const statement& st) { op_mod(st); };
    _operations["and"] = [this](const statement& st) { op_and(st); };
    _operations["or"] = [this](const statement& st) { op_or(st); };
    _operations["xor"] = [this](const statement& st) { op_xor(st); };
    _operations["shl"] = [this](const statement& st) { op_shl(st); };
    _operations["shr"] = [this](const statement& st) { op_shr(st); };
    _operations["cmp"] = [this](const statement& st) { op_cmp(st); };
    _operations["exp"] = [this](const statement& st) { op_exp(st); };
    _operations["inp"] = [this](const statement& st) { op_inp(st); };
    _operations["out"] = [this](const statement& st) { op_out(st); };
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

// Первый аргумент set/sum/... должен быть переменной, а не числом
void interpreter::check_is_variable(const std::string& arg, int line) const {
    if (arg.empty() || is_number(arg)) {
        error(line, "ожидалось имя переменной, получено '" + arg + "'");
    }
}

// Если аргумент — число, возвращает его, иначе значение переменной
long long interpreter::get_value(const std::string& arg, int line) const {
    if (is_number(arg)) {
        return std::stoll(arg);
    }
    if (_vars.count(arg) == 0) {
        error(line, "переменная '" + arg + "' не определена");
    }
    return _vars.at(arg);
}

void interpreter::op_set(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);
    _vars[st.args[0]] = get_value(st.args[1], st.line);
}

void interpreter::op_sum(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);

    long long a = get_value(st.args[0], st.line);
    long long b = get_value(st.args[1], st.line);
    _vars[st.args[0]] = a + b;
}

void interpreter::op_min(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);

    long long a = get_value(st.args[0], st.line);
    long long b = get_value(st.args[1], st.line);
    _vars[st.args[0]] = a - b;
}

void interpreter::op_mul(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);

    long long a = get_value(st.args[0], st.line);
    long long b = get_value(st.args[1], st.line);
    _vars[st.args[0]] = a * b;
}

void interpreter::op_div(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);

    long long a = get_value(st.args[0], st.line);
    long long b = get_value(st.args[1], st.line);
    if (b == 0) {
        error(st.line, "деление на ноль");
    }
    _vars[st.args[0]] = a / b;
}

void interpreter::op_mod(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);

    long long a = get_value(st.args[0], st.line);
    long long b = get_value(st.args[1], st.line);
    if (b == 0) {
        error(st.line, "деление на ноль");
    }
    _vars[st.args[0]] = a % b;
}

void interpreter::op_and(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);

    long long a = get_value(st.args[0], st.line);
    long long b = get_value(st.args[1], st.line);
    _vars[st.args[0]] = a & b;
}

void interpreter::op_or(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);

    long long a = get_value(st.args[0], st.line);
    long long b = get_value(st.args[1], st.line);
    _vars[st.args[0]] = a | b;
}

void interpreter::op_xor(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);

    long long a = get_value(st.args[0], st.line);
    long long b = get_value(st.args[1], st.line);
    _vars[st.args[0]] = a ^ b;
}

// Сдвиг на отрицательное число или на >= 64 бит в C++ — неопределённое
// поведение, поэтому такой сдвиг считаем ошибкой
void interpreter::op_shl(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);

    long long a = get_value(st.args[0], st.line);
    long long b = get_value(st.args[1], st.line);
    if (b < 0 || b >= 64) {
        error(st.line, "величина сдвига должна быть от 0 до 63, получено " + std::to_string(b));
    }
    // Сдвиг через unsigned, чтобы сдвиг отрицательного числа влево был определён
    _vars[st.args[0]] = static_cast<long long>(static_cast<unsigned long long>(a) << b);
}

void interpreter::op_shr(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);

    long long a = get_value(st.args[0], st.line);
    long long b = get_value(st.args[1], st.line);
    if (b < 0 || b >= 64) {
        error(st.line, "величина сдвига должна быть от 0 до 63, получено " + std::to_string(b));
    }
    _vars[st.args[0]] = a >> b;
}

void interpreter::op_cmp(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);

    long long a = get_value(st.args[0], st.line);
    long long b = get_value(st.args[1], st.line);
    if (a < b) {
        _vars[st.args[0]] = -1;
    } else if (a == b) {
        _vars[st.args[0]] = 0;
    } else {
        _vars[st.args[0]] = 1;
    }
}

// Выражение переводится в постфиксную запись, затем в обычные команды
// (set/sum/min/...), которые сразу выполняются. Временные переменные
// ($1, $2, ...) после вычисления удаляются.
void interpreter::op_exp(const statement& st) {
    check_args_count(st, 2);
    check_is_variable(st.args[0], st.line);

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
            const std::string& target = command.args[0];
            std::string text = command.op + " " + target + "," + command.args[1] + ";";
            text.resize(std::max<std::size_t>(text.size() + 2, 16), ' ');
            std::cout << "    " << text << target << " = " << _vars.at(target) << "\n";
        }
    }

    for (auto it = _vars.begin(); it != _vars.end();) {
        if (it->first[0] == '$') {
            it = _vars.erase(it);
        } else {
            ++it;
        }
    }
}

void interpreter::op_inp(const statement& st) {
    check_args_count(st, 1);
    check_is_variable(st.args[0], st.line);

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
    if (_operations.count(st.op) == 0) {
        error(st.line, "неизвестная операция '" + st.op + "'");
    }
    // Достаём функцию по имени операции и сразу вызываем её
    _operations.at(st.op)(st);
}

void interpreter::run(const std::string& source) {
    lexer lex(source);
    statement st;

    while (lex.next(st)) {
        execute(st);
    }
}
