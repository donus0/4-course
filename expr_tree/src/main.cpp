#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include "expression_tree.hpp"

// Спрашивает у пользователя значение переменной, пока не введёт число
double read_value(char name) {
    while (true) {
        std::cout << "  " << name << " = ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error("Ввод прерван");
        }
        try {
            return std::stod(line);
        } catch (const std::exception&) {
            std::cout << "  Нужно ввести число\n";
        }
    }
}

void process(const std::string& expression) {
    expression_tree tree(expression);

    std::cout << "\nДерево (корень слева, правое поддерево сверху):\n\n";
    tree.print();

    std::cout << "\nПрефиксная форма:  " << tree.prefix() << "\n";
    std::cout << "Инфиксная форма:   " << tree.infix() << "\n";
    std::cout << "Постфиксная форма: " << tree.postfix() << "\n";

    std::map<char, double> values;
    std::string vars = tree.variables();
    if (!vars.empty()) {
        std::cout << "\nВведите значения переменных:\n";
        for (char v : vars) {
            values[v] = read_value(v);
        }
    }

    double result = tree.evaluate(values);
    std::cout << "\nЗначение выражения: " << result << "\n";
}

int main() {
#ifdef _WIN32
    // Чтобы русские сообщения нормально выводились в консоли Windows
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "Построение дерева арифметического выражения\n"
              << "Операнды — одна цифра или буква, операции: + - * / ^ и скобки\n";

    while (true) {
        std::cout << "\nВведите выражение (пусто — выход), например (a+3)*b-5/c: ";
        std::string expression;
        if (!std::getline(std::cin, expression) || expression.empty()) {
            break;
        }

        try {
            process(expression);
        } catch (const std::exception& e) {
            std::cout << e.what() << "\n";
        }
    }

    return 0;
}
