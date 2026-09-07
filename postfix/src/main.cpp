#include <iostream>
#include <string>

#include "InfixToPostfix.hpp"
#include "PostfixEval.hpp"

void printMenu() {
    std::cout << "\n=== Постфиксное преобразование выражений (метод Дийкстры) ===\n"
              << "1. Инфикс -> постфикс (числа однозначные), например: (1+2)*3-9/3^2\n"
              << "2. Вычислить постфиксное выражение (числа многозначные, через пробел), например: 12 3 4 * +\n"
              << "0. Выход\n"
              << "Выбор: ";
}

void handleConvert() {
    std::cout << "Введите инфиксное выражение: ";
    std::string infix;
    std::getline(std::cin, infix);
    try {
        std::string postfix = infixToPostfix(infix);
        std::cout << "Постфиксная форма: " << postfix << "\n";
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << "\n";
    }
}

void handleEvaluate() {
    std::cout << "Введите постфиксное выражение (токены через пробел): ";
    std::string postfix;
    std::getline(std::cin, postfix);
    try {
        double result = evaluatePostfix(postfix);
        std::cout << "Результат: " << result << "\n";
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << "\n";
    }
}

int main() {
    while (true) {
        printMenu();
        std::string choice;
        if (!std::getline(std::cin, choice)) {
            break;
        }

        if (choice == "0") {
            break;
        } else if (choice == "1") {
            handleConvert();
        } else if (choice == "2") {
            handleEvaluate();
        } else {
            std::cout << "Неверный выбор.\n";
        }
    }

    std::cout << "Завершение работы.\n";
    return 0;
}
