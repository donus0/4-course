#pragma once

#include <map>
#include <string>

// Узел дерева выражения. В листьях лежат операнды (цифра или буква),
// во внутренних узлах — операции + - * / ^
struct tree_node {
    char value;
    tree_node* left;
    tree_node* right;

    tree_node(char v, tree_node* l = nullptr, tree_node* r = nullptr)
        : value(v), left(l), right(r) {}
};

class expression_tree {
private:
    tree_node* _root;

public:
    // Строит дерево по инфиксной записи, например "(a+3)*b-5/c".
    // Если выражение записано с ошибкой, бросает std::invalid_argument
    expression_tree(const std::string& expression);
    ~expression_tree();

    // Копировать дерево не нужно, а копия по умолчанию удалила бы узлы дважды
    expression_tree(const expression_tree&) = delete;
    expression_tree& operator=(const expression_tree&) = delete;

    // Печать дерева, повёрнутого на 90 градусов (корень слева)
    void print() const;

    std::string prefix() const;
    std::string infix() const;
    std::string postfix() const;

    // Буквы-переменные из выражения, без повторов и по алфавиту
    std::string variables() const;

    // Вычисляет выражение, значения переменных берутся из values
    double evaluate(const std::map<char, double>& values) const;
};
