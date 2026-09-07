#include "InfixToPostfix.hpp"
#include "Stack.hpp"

#include <cctype>
#include <stdexcept>

namespace {

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

bool isRightAssociative(char op) {
    return op == '^';
}

bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

}  // namespace

std::string infixToPostfix(const std::string& infix) {
    Stack<char> operators;
    std::string postfix;

    for (char c : infix) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            // Числа однозначные, поэтому каждая цифра — отдельный операнд.
            postfix += c;
            postfix += ' ';
        } else if (c == '(') {
            operators.push(c);
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
        } else if (isOperator(c)) {
            while (!operators.empty() && operators.top() != '(' &&
                   (precedence(operators.top()) > precedence(c) ||
                    (precedence(operators.top()) == precedence(c) && !isRightAssociative(c)))) {
                postfix += operators.top();
                postfix += ' ';
                operators.pop();
            }
            operators.push(c);
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
