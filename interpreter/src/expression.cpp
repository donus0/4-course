#include "expression.hpp"

#include <cctype>
#include <iostream>
#include <stdexcept>

static void syntax_error(int line, const std::string& message) {
    throw std::runtime_error("Синтаксическая ошибка (строка " + std::to_string(line) +
                             "): в выражении " + message);
}

static std::string quoted(char c) {
    return "'" + std::string(1, c) + "'";
}

static bool is_binary_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%';
}

static bool is_name_char(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
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
                       const std::vector<std::string>& postfix, std::size_t from) {
    const std::size_t max_shown = 10;
    std::size_t first = operators.size() > max_shown ? operators.size() - max_shown : 0;
    std::string stack = first > 0 ? "..." : "";
    for (std::size_t i = first; i < operators.size(); i++) {
        if (!stack.empty()) {
            stack += ' ';
        }
        stack += operators[i];
    }

    std::vector<std::string> added(postfix.begin() + from, postfix.end());
    std::cout << "  " << pad(token, 8) << pad(stack, 24) << postfix_to_string(added) << "\n";
}

static void pop_operator(std::vector<char>& operators, std::vector<std::string>& postfix) {
    postfix.push_back(std::string(1, operators.back()));
    operators.pop_back();
}

std::vector<std::string> infix_to_postfix(const std::string& expr, int line, bool trace) {
    std::vector<std::string> postfix;
    std::vector<char> operators;

    if (trace) {
        std::cout << "  " << pad("символ", 8) << pad("стек", 24) << "в выход\n";
    }
    bool expect_operand = true;

    std::size_t i = 0;
    while (i < expr.size()) {
        char c = expr[i];

        if (std::isspace(static_cast<unsigned char>(c))) {
            i++;
            continue;
        }

        std::size_t token_start = i;
        std::size_t output_before = postfix.size();

        if (is_name_char(c)) {
            if (!expect_operand) {
                syntax_error(line, "пропущен оператор перед " + quoted(c));
            }
            bool number = std::isdigit(static_cast<unsigned char>(c));
            while (i < expr.size() && is_name_char(expr[i])) {
                if (number && !std::isdigit(static_cast<unsigned char>(expr[i]))) {
                    syntax_error(line, "некорректное число '" + expr.substr(token_start, i - token_start + 1) + "'");
                }
                i++;
            }
            postfix.push_back(expr.substr(token_start, i - token_start));
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
                pop_operator(operators, postfix);
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
                syntax_error(line, "ожидался операнд перед " + quoted(c));
            }
            while (!operators.empty() && operators.back() != '(' &&
                   precedence(operators.back()) >= precedence(c)) {
                pop_operator(operators, postfix);
            }
            operators.push_back(c);
            expect_operand = true;
            i++;
        } else {
            syntax_error(line, "недопустимый символ " + quoted(c));
        }

        if (trace) {
            print_step(expr.substr(token_start, i - token_start), operators, postfix, output_before);
        }
    }

    if (expect_operand) {
        syntax_error(line, "ожидался операнд в конце");
    }
    std::size_t output_before = postfix.size();
    while (!operators.empty()) {
        if (operators.back() == '(') {
            syntax_error(line, "не закрыта скобка");
        }
        pop_operator(operators, postfix);
    }

    if (trace) {
        print_step("конец", operators, postfix, output_before);
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

static std::string pop_operand(std::vector<std::string>& operands) {
    std::string top = operands.back();
    operands.pop_back();
    return top;
}

std::vector<statement> postfix_to_commands(const std::vector<std::string>& postfix,
                                           const std::string& dst, int line) {
    std::vector<statement> commands;
    std::vector<std::string> operands;
    int temp_count = 0;

    auto new_temp = [&](const std::string& value) {
        std::string temp = "$" + std::to_string(++temp_count);
        commands.push_back(make_command("set", temp, value, line));
        return temp;
    };

    for (const std::string& token : postfix) {
        if (token == "~") {
            std::string a = pop_operand(operands);
            std::string temp = new_temp("0");
            commands.push_back(make_command("min", temp, a, line));
            operands.push_back(temp);
        } else if (token.size() == 1 && is_binary_operator(token[0])) {
            std::string b = pop_operand(operands);
            std::string a = pop_operand(operands);
            if (a[0] != '$') {
                a = new_temp(a);
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
