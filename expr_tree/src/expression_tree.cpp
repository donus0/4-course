#include "expression_tree.hpp"

#include <cctype>
#include <cmath>
#include <iostream>
#include <set>
#include <stack>
#include <stdexcept>

// Алгоритм построения (два стека, как в методе Дийкстры):
//   - операнд сразу превращаем в лист и кладём в стек поддеревьев;
//   - '(' кладём в стек операций;
//   - ')' — собираем узлы, пока не дойдём до '(';
//   - операция — сначала собираем узлы из операций на стеке с большим
//     (или равным, если операция левоассоциативная) приоритетом, потом
//     кладём её в стек.
// "Собрать узел" = снять операцию и два верхних поддерева, сделать из них
// новый узел (операция — корень, поддеревья — левый и правый сын) и положить
// его обратно в стек поддеревьев. В конце в стеке остаётся одно дерево.

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

bool is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

bool is_operand(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) != 0;
}

void syntax_error(const std::string& message, std::size_t pos) {
    throw std::invalid_argument("Ошибка в позиции " + std::to_string(pos + 1) + ": " + message);
}

void destroy(tree_node* n) {
    if (n == nullptr) {
        return;
    }
    destroy(n->left);
    destroy(n->right);
    delete n;
}

void make_node(std::stack<char>& ops, std::stack<tree_node*>& nodes) {
    char op = ops.top();
    ops.pop();

    // Правый операнд лежит выше левого, т.к. был добавлен позже
    tree_node* right = nodes.top();
    nodes.pop();
    tree_node* left = nodes.top();
    nodes.pop();

    nodes.push(new tree_node(op, left, right));
}

tree_node* build(const std::string& expression) {
    std::stack<char> ops;
    std::stack<tree_node*> nodes;

    // true — сейчас ждём операнд или '(', false — операцию или ')'.
    // Благодаря этому флагу в make_node в стеке всегда есть два поддерева
    bool expect_operand = true;

    try {
        for (std::size_t i = 0; i < expression.size(); i++) {
            char c = expression[i];

            if (std::isspace(static_cast<unsigned char>(c))) {
                continue;
            }

            if (is_operand(c)) {
                if (!expect_operand) {
                    syntax_error("два операнда подряд (операнд — одна цифра или буква)", i);
                }
                nodes.push(new tree_node(c));
                expect_operand = false;
            } else if (c == '(') {
                if (!expect_operand) {
                    syntax_error("перед '(' пропущена операция", i);
                }
                ops.push(c);
            } else if (c == ')') {
                if (expect_operand) {
                    syntax_error("перед ')' пропущен операнд", i);
                }
                while (!ops.empty() && ops.top() != '(') {
                    make_node(ops, nodes);
                }
                if (ops.empty()) {
                    syntax_error("лишняя закрывающая скобка", i);
                }
                ops.pop();  // убираем '('
            } else if (is_operator(c)) {
                if (expect_operand) {
                    syntax_error(std::string("перед '") + c + "' пропущен операнд", i);
                }
                // '^' правоассоциативная: a^b^c = a^(b^c), поэтому такую же '^' со стека не снимаем
                while (!ops.empty() && ops.top() != '(' &&
                       (precedence(ops.top()) > precedence(c) ||
                        (precedence(ops.top()) == precedence(c) && c != '^'))) {
                    make_node(ops, nodes);
                }
                ops.push(c);
                expect_operand = true;
            } else {
                syntax_error(std::string("недопустимый символ '") + c + "'", i);
            }
        }

        if (nodes.empty() && ops.empty()) {
            throw std::invalid_argument("Пустое выражение");
        }
        if (expect_operand) {
            syntax_error("выражение оборвано, не хватает операнда", expression.size());
        }

        while (!ops.empty()) {
            if (ops.top() == '(') {
                throw std::invalid_argument("Не закрыта скобка");
            }
            make_node(ops, nodes);
        }
    } catch (...) {
        // Если выражение с ошибкой — удаляем уже созданные узлы, чтобы не было утечки
        while (!nodes.empty()) {
            destroy(nodes.top());
            nodes.pop();
        }
        throw;
    }

    return nodes.top();
}

expression_tree::expression_tree(const std::string& expression) {
    _root = build(expression);
}

expression_tree::~expression_tree() {
    destroy(_root);
}

// Сначала правое поддерево, потом узел, потом левое — тогда при повороте
// картинки на 90 градусов по часовой стрелке получится обычное дерево
void print_node(const tree_node* n, int depth) {
    if (n == nullptr) {
        return;
    }
    print_node(n->right, depth + 1);
    std::cout << std::string(depth * 4, ' ') << n->value << "\n";
    print_node(n->left, depth + 1);
}

void expression_tree::print() const {
    print_node(_root, 0);
}

// Прямой обход: узел, левое, правое
void prefix_walk(const tree_node* n, std::string& result) {
    if (n == nullptr) {
        return;
    }
    result += n->value;
    result += ' ';
    prefix_walk(n->left, result);
    prefix_walk(n->right, result);
}

// Обратный обход: левое, правое, узел
void postfix_walk(const tree_node* n, std::string& result) {
    if (n == nullptr) {
        return;
    }
    postfix_walk(n->left, result);
    postfix_walk(n->right, result);
    result += n->value;
    result += ' ';
}

// Симметричный обход: левое, узел, правое. Каждую операцию берём в скобки,
// иначе из дерева нельзя восстановить порядок действий
std::string infix_walk(const tree_node* n) {
    if (n->left == nullptr) {
        return std::string(1, n->value);
    }
    return "(" + infix_walk(n->left) + " " + n->value + " " + infix_walk(n->right) + ")";
}

std::string expression_tree::prefix() const {
    std::string result;
    prefix_walk(_root, result);
    result.pop_back();  // лишний пробел в конце
    return result;
}

std::string expression_tree::postfix() const {
    std::string result;
    postfix_walk(_root, result);
    result.pop_back();
    return result;
}

std::string expression_tree::infix() const {
    std::string result = infix_walk(_root);
    // Внешние скобки вокруг всего выражения не нужны
    if (_root->left != nullptr) {
        result = result.substr(1, result.size() - 2);
    }
    return result;
}

void collect_variables(const tree_node* n, std::set<char>& vars) {
    if (n == nullptr) {
        return;
    }
    if (std::isalpha(static_cast<unsigned char>(n->value))) {
        vars.insert(n->value);
    }
    collect_variables(n->left, vars);
    collect_variables(n->right, vars);
}

std::string expression_tree::variables() const {
    std::set<char> vars;
    collect_variables(_root, vars);
    return std::string(vars.begin(), vars.end());
}

double evaluate_node(const tree_node* n, const std::map<char, double>& values) {
    // Лист — это операнд
    if (n->left == nullptr) {
        if (std::isdigit(static_cast<unsigned char>(n->value))) {
            return n->value - '0';
        }
        auto it = values.find(n->value);
        if (it == values.end()) {
            throw std::runtime_error(std::string("Не задано значение переменной ") + n->value);
        }
        return it->second;
    }

    double a = evaluate_node(n->left, values);
    double b = evaluate_node(n->right, values);

    switch (n->value) {
        case '+':
            return a + b;
        case '-':
            return a - b;
        case '*':
            return a * b;
        case '/':
            if (b == 0) {
                throw std::runtime_error("Деление на ноль");
            }
            return a / b;
        case '^':
            return std::pow(a, b);
        default:
            throw std::runtime_error(std::string("Неизвестная операция ") + n->value);
    }
}

double expression_tree::evaluate(const std::map<char, double>& values) const {
    return evaluate_node(_root, values);
}
