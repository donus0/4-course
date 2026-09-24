#include <iostream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "DynamicArray.hpp"
#include "InfixToPostfix.hpp"
#include "PostfixEval.hpp"

void printMenu() {
    std::cout << "\n=== Постфиксное преобразование выражений (метод Дийкстры) ===\n"
              << "1. Инфикс -> постфикс (операнды любой длины), например: (var+12)*c-9/x^2\n"
              << "2. Вычислить постфиксное выражение (числа многозначные, через пробел), например: 12 3 4 * +\n"
              << "0. Выход\n"
              << "Выбор: ";
}

void handleConvert() {
    std::cout << "Введите инфиксное выражение: ";
    std::string infix;
    std::getline(std::cin, infix);

    DynamicArray<std::string> operands;
    try {
        std::string postfix = infixToPostfix(infix, operands);
        std::cout << "Постфиксная форма: " << postfix << "\n";

        std::cout << "Операнды (входной массив, отсортирован): ";
        for (std::size_t i = 0; i < operands.size(); ++i) {
            std::cout << operands[i] << (i + 1 < operands.size() ? ", " : "\n");
        }
        if (operands.empty()) {
            std::cout << "(нет)\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Ошибка: " << e.what() << "\n";
        return;
    }

    std::cout << "Найти операнд дихотомическим поиском (пусто — пропустить): ";
    std::string query;
    std::getline(std::cin, query);
    if (!query.empty()) {
        long index = operands.binarySearch(query);
        if (index >= 0) {
            std::cout << "Найдено: \"" << query << "\" на позиции " << index << "\n";
        } else {
            std::cout << "\"" << query << "\" в выражении не встречается\n";
        }
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
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

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
