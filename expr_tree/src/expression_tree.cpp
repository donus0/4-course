#include "expression_tree.hpp"

#include <cctype>
#include <cmath>
#include <iostream>
#include <set>
#include <stack>
#include <stdexcept>
#include <utility>

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

expression_tree::node::node(char v, node* l, node* r) : value(v), left(l), right(r) {}

expression_tree::node::node(const node& other) : value(other.value), left(nullptr), right(nullptr) {
    try {
        if (other.left != nullptr) {
            left = new node(*other.left);
        }
        if (other.right != nullptr) {
            right = new node(*other.right);
        }
    } catch (...) {
        delete left;
        throw;
    }
}

expression_tree::node::node(node&& other) noexcept
    : value(other.value), left(other.left), right(other.right) {
    other.left = nullptr;
    other.right = nullptr;
}

expression_tree::node& expression_tree::node::operator=(const node& other) {
    if (this != &other) {
        node copy(other);
        swap(copy);
    }
    return *this;
}

expression_tree::node& expression_tree::node::operator=(node&& other) noexcept {
    if (this != &other) {
        node temp(std::move(other));
        swap(temp);
    }
    return *this;
}

expression_tree::node::~node() {
    delete left;
    delete right;
}

void expression_tree::node::swap(node& other) noexcept {
    std::swap(value, other.value);
    std::swap(left, other.left);
    std::swap(right, other.right);
}


void expression_tree::make_node(std::stack<char>& ops, std::stack<node*>& nodes) {
    // Узел создаём до того, как снимать поддеревья со стека: если new бросит
    // исключение, поддеревья останутся в стеке и build их удалит
    node* parent = new node(ops.top());
    ops.pop();

    // Правый операнд лежит выше левого, т.к. был добавлен позже
    parent->right = nodes.top();
    nodes.pop();
    parent->left = nodes.top();
    nodes.pop();

    nodes.push(parent);
}

expression_tree::node* expression_tree::build(const std::string& expression) {
    std::stack<char> ops;
    std::stack<node*> nodes;

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
                nodes.push(new node(c));
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
            delete nodes.top();  // деструктор узла удалит и всё его поддерево
            nodes.pop();
        }
        throw;
    }

    return nodes.top();
}

expression_tree::expression_tree(const std::string& expression) {
    _root = build(expression);
}

expression_tree::expression_tree(const expression_tree& other)
    : _root(other._root != nullptr ? new node(*other._root) : nullptr) {}

expression_tree::expression_tree(expression_tree&& other) noexcept : _root(other._root) {
    other._root = nullptr;
}

expression_tree& expression_tree::operator=(const expression_tree& other) {
    if (this != &other) {
        expression_tree copy(other);
        std::swap(_root, copy._root);
    }
    return *this;
}

expression_tree& expression_tree::operator=(expression_tree&& other) noexcept {
    if (this != &other) {
        delete _root;
        _root = other._root;
        other._root = nullptr;
    }
    return *this;
}

expression_tree::~expression_tree() {
    delete _root;
}

void expression_tree::print_node(const node* n, int depth) {
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

void expression_tree::prefix_walk(const node* n, std::string& result) {
    if (n == nullptr) {
        return;
    }
    result += n->value;
    result += ' ';
    prefix_walk(n->left, result);
    prefix_walk(n->right, result);
}

void expression_tree::postfix_walk(const node* n, std::string& result) {
    if (n == nullptr) {
        return;
    }
    postfix_walk(n->left, result);
    postfix_walk(n->right, result);
    result += n->value;
    result += ' ';
}

std::string expression_tree::infix_walk(const node* n) {
    if (n->left == nullptr) {
        return std::string(1, n->value);
    }
    return "(" + infix_walk(n->left) + " " + n->value + " " + infix_walk(n->right) + ")";
}

std::string expression_tree::prefix() const {
    std::string result;
    prefix_walk(_root, result);
    if (!result.empty()) {
        result.pop_back();  // лишний пробел в конце
    }
    return result;
}

std::string expression_tree::postfix() const {
    std::string result;
    postfix_walk(_root, result);
    if (!result.empty()) {
        result.pop_back();
    }
    return result;
}

std::string expression_tree::infix() const {
    if (_root == nullptr) {
        return "";
    }
    std::string result = infix_walk(_root);
    if (_root->left != nullptr) {
        result = result.substr(1, result.size() - 2);
    }
    return result;
}

void expression_tree::collect_variables(const node* n, std::set<char>& vars) {
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

double expression_tree::evaluate_node(const node* n, const std::map<char, double>& values) {
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
    if (_root == nullptr) {
        throw std::logic_error("Дерево пустое (было перемещено)");
    }
    return evaluate_node(_root, values);
}
