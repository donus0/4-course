#include "PostfixEval.hpp"
#include "Stack.hpp"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>


bool isOperator(const std::string& token) {
    return token.size() == 1 && std::string("+-*/^").find(token[0]) != std::string::npos;
}

double applyOperator(char op, double a, double b) {
    switch (op) {
        case '+':
            return a + b;
        case '-':
            return a - b;
        case '*':
            return a * b;
        case '/':
            if (b == 0.0) {
                throw std::domain_error("Деление на ноль");
            }
            return a / b;
        case '^':
            return std::pow(a, b);
        default:
            throw std::invalid_argument(std::string("Неизвестный оператор: ") + op);
    }
}

double evaluatePostfix(const std::string& postfix) {
    Stack<double> values;
    std::istringstream tokens(postfix);
    std::string token;

    while (tokens >> token) {
        if (isOperator(token)) {
            if (values.size() < 2) {
                throw std::invalid_argument("Некорректное постфиксное выражение: недостаточно операндов");
            }
            double b = values.top();
            values.pop();
            double a = values.top();
            values.pop();
            values.push(applyOperator(token[0], a, b));
        } else {
            std::size_t pos = 0;
            double number = 0.0;
            try {
                number = std::stod(token, &pos);
            } catch (const std::exception&) {
                throw std::invalid_argument("Некорректный операнд: " + token);
            }
            if (pos != token.size()) {
                throw std::invalid_argument("Некорректный операнд: " + token);
            }
            values.push(number);
        }
    }

    if (values.size() != 1) {
        throw std::invalid_argument("Некорректное постфиксное выражение");
    }

    return values.top();
}
