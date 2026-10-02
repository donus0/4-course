#include "expression.hpp"

#include <cctype>
#include <iostream>
#include <stdexcept>

static void syntax_error(int line, const std::string& message) {
    throw std::runtime_error("Синтаксическая ошибка (строка " + std::to_string(line) +
                             "): в выражении " + message);
}

static bool is_binary_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%';
}

// Унарный минус '~' выполняется раньше любых бинарных операторов
static int precedence(char op) {
    switch (op) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
        case '%':
            return 2;
        case '~':
            return 3;
        default:
            return 0;
    }
}

std::string postfix_to_string(const std::vector<std::string>& postfix) {
    std::string result;
    for (const std::string& token : postfix) {
        if (!result.empty()) {
            result += ' ';
        }
        result += token;
    }
    return result;
}

// Дополняет строку пробелами до width символов. Длина считается в символах,
// а не в байтах, чтобы русские слова в UTF-8 не сбивали колонки.
static std::string pad(const std::string& s, std::size_t width) {
    std::size_t length = 0;
    for (char c : s) {
        if ((static_cast<unsigned char>(c) & 0xC0) != 0x80) {
            length++;
        }
    }
    return length < width ? s + std::string(width - length, ' ') : s + ' ';
}

static void print_step(const std::string& token, const std::vector<char>& operators,
                       const std::vector<std::string>& postfix) {
    std::string stack;
    for (char op : operators) {
        if (!stack.empty()) {
            stack += ' ';
        }
        stack += op;
    }
    std::cout << "  " << pad(token, 8) << pad(stack, 16) << postfix_to_string(postfix) << "\n";
}

std::vector<std::string> infix_to_postfix(const std::string& expr, int line, bool trace) {
    std::vector<std::string> postfix;
    std::vector<char> operators;

    if (trace) {
        std::cout << "  " << pad("символ", 8) << pad("стек", 16) << "выход\n";
    }
    // true — дальше должен идти операнд (число, переменная, '(' или унарный минус),
    // false — дальше должен идти бинарный оператор или ')'
    bool expect_operand = true;

    std::size_t i = 0;
    while (i < expr.size()) {
        char c = expr[i];

        if (std::isspace(static_cast<unsigned char>(c))) {
            i++;
            continue;
        }

        std::size_t token_start = i;
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
            if (!expect_operand) {
                syntax_error(line, "пропущен оператор перед '" + std::string(1, c) + "'");
            }
            std::size_t start = i;
            bool is_number = std::isdigit(static_cast<unsigned char>(c));
            while (i < expr.size() &&
                   (std::isalnum(static_cast<unsigned char>(expr[i])) || expr[i] == '_')) {
                if (is_number && !std::isdigit(static_cast<unsigned char>(expr[i]))) {
                    syntax_error(line, "некорректное число '" + expr.substr(start, i - start + 1) + "'");
                }
                i++;
            }
            postfix.push_back(expr.substr(start, i - start));
            expect_operand = false;
        } else if (c == '(') {
            if (!expect_operand) {
                syntax_error(line, "пропущен оператор перед '('");
            }
            operators.push_back(c);
            i++;
        } else if (c == ')') {
            if (expect_operand) {
                syntax_error(line, "ожидался операнд перед ')'");
            }
            while (!operators.empty() && operators.back() != '(') {
                postfix.push_back(std::string(1, operators.back()));
                operators.pop_back();
            }
            if (operators.empty()) {
                syntax_error(line, "лишняя закрывающая скобка");
            }
            operators.pop_back();
            i++;
        } else if (c == '-' && expect_operand) {
            // Унарный минус ничего не выталкивает из стека: он относится к следующему операнду
            operators.push_back('~');
            i++;
        } else if (is_binary_operator(c)) {
            if (expect_operand) {
                syntax_error(line, "ожидался операнд перед '" + std::string(1, c) + "'");
            }
            while (!operators.empty() && operators.back() != '(' &&
                   precedence(operators.back()) >= precedence(c)) {
                postfix.push_back(std::string(1, operators.back()));
                operators.pop_back();
            }
            operators.push_back(c);
            expect_operand = true;
            i++;
        } else {
            syntax_error(line, "недопустимый символ '" + std::string(1, c) + "'");
        }

        if (trace) {
            print_step(expr.substr(token_start, i - token_start), operators, postfix);
        }
    }

    if (expect_operand) {
        syntax_error(line, "ожидался операнд в конце");
    }
    while (!operators.empty()) {
        if (operators.back() == '(') {
            syntax_error(line, "не закрыта скобка");
        }
        postfix.push_back(std::string(1, operators.back()));
        operators.pop_back();
    }

    if (trace) {
        print_step("конец", operators, postfix);
    }

    return postfix;
}

static statement make_command(const std::string& op, const std::string& a, const std::string& b, int line) {
    statement st;
    st.op = op;
    st.args = {a, b};
    st.line = line;
    return st;
}

static std::string operator_command(char op) {
    switch (op) {
        case '+':
            return "sum";
        case '-':
            return "min";
        case '*':
            return "mul";
        case '/':
            return "div";
        default:
            return "mod";
    }
}

// Стек хранит имена, где лежат промежуточные значения: переменные программы,
// числа или временные переменные $1, $2, ... Если левый операнд уже временная
// переменная, результат записывается прямо в неё, иначе заводится новая.
std::vector<statement> postfix_to_commands(const std::vector<std::string>& postfix,
                                           const std::string& dst, int line) {
    std::vector<statement> commands;
    std::vector<std::string> operands;
    int temp_count = 0;

    for (const std::string& token : postfix) {
        if (token == "~") {
            std::string a = operands.back();
            operands.pop_back();
            std::string temp = "$" + std::to_string(++temp_count);
            commands.push_back(make_command("set", temp, "0", line));
            commands.push_back(make_command("min", temp, a, line));
            operands.push_back(temp);
        } else if (token.size() == 1 && is_binary_operator(token[0])) {
            std::string b = operands.back();
            operands.pop_back();
            std::string a = operands.back();
            operands.pop_back();

            if (a[0] != '$') {
                std::string temp = "$" + std::to_string(++temp_count);
                commands.push_back(make_command("set", temp, a, line));
                a = temp;
            }
            commands.push_back(make_command(operator_command(token[0]), a, b, line));
            operands.push_back(a);
        } else {
            operands.push_back(token);
        }
    }

    commands.push_back(make_command("set", dst, operands.back(), line));
    return commands;
}
