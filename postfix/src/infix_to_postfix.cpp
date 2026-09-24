#include "infix_to_postfix.hpp"
#include "stack.hpp"

#include <cctype>
#include <stdexcept>

int precedence(char op) {
    switch (op) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
            return 2;
        case '^':
            return 3;
        default:
            return 0;
    }
}

bool is_right_associative(char op) {
    return op == '^';
}

bool is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

std::string infix_to_postfix(const std::string& infix, dynamic_array<std::string>& operands) {
    stack<char> operators;
    std::string postfix;

    std::size_t i = 0;
    while (i < infix.size()) {
        char c = infix[i];

        if (std::isspace(static_cast<unsigned char>(c))) {
            ++i;
            continue;
        }

        if (std::isalnum(static_cast<unsigned char>(c))) {
            // Операнд (переменная или число) может занимать несколько символов.
            std::size_t start = i;
            while (i < infix.size() && std::isalnum(static_cast<unsigned char>(infix[i]))) {
                ++i;
            }
            std::string token = infix.substr(start, i - start);
            operands.insert_sorted(token);
            postfix += token;
            postfix += ' ';
        } else if (c == '(') {
            operators.push(c);
            ++i;
        } else if (c == ')') {
            bool matched = false;
            while (!operators.empty()) {
                char op = operators.top();
                operators.pop();
                if (op == '(') {
                    matched = true;
                    break;
                }
                postfix += op;
                postfix += ' ';
            }
            if (!matched) {
                throw std::invalid_argument("Несогласованные скобки в выражении");
            }
            ++i;
        } else if (is_operator(c)) {
            while (!operators.empty() && operators.top() != '(' &&
                   (precedence(operators.top()) > precedence(c) ||
                    (precedence(operators.top()) == precedence(c) && !is_right_associative(c)))) {
                postfix += operators.top();
                postfix += ' ';
                operators.pop();
            }
            operators.push(c);
            ++i;
        } else {
            throw std::invalid_argument(std::string("Недопустимый символ во входном выражении: ") + c);
        }
    }

    while (!operators.empty()) {
        char op = operators.top();
        operators.pop();
        if (op == '(') {
            throw std::invalid_argument("Несогласованные скобки в выражении");
        }
        postfix += op;
        postfix += ' ';
    }

    if (!postfix.empty() && postfix.back() == ' ') {
        postfix.pop_back();
    }

    return postfix;
}
