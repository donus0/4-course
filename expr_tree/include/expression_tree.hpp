#pragma once

#include <map>
#include <set>
#include <stack>
#include <string>

class expression_tree final {
private:
    struct node final {
        char value;
        node* left;
        node* right;

        node(char v, node* l = nullptr, node* r = nullptr);

        node(const node& other);                 
        node(node&& other) noexcept;             
        node& operator=(const node& other);      
        node& operator=(node&& other) noexcept;  
        ~node();                                 

        void swap(node& other) noexcept;
    };

    node* _root;

    static node* build(const std::string& expression);
    static void make_node(std::stack<char>& ops, std::stack<node*>& nodes);

    static void print_node(const node* n, int depth);
    static void prefix_walk(const node* n, std::string& result);
    static void postfix_walk(const node* n, std::string& result);
    static std::string infix_walk(const node* n);
    static void collect_variables(const node* n, std::set<char>& vars);
    static double evaluate_node(const node* n, const std::map<char, double>& values);

public:
    // Строит дерево по инфиксной записи, например "(a+3)*b-5/c".
    expression_tree(const std::string& expression);

    expression_tree(const expression_tree& other);                 
    expression_tree(expression_tree&& other) noexcept;             
    expression_tree& operator=(const expression_tree& other);      
    expression_tree& operator=(expression_tree&& other) noexcept;  
    ~expression_tree();                                            

    void print() const;

    std::string prefix() const;
    std::string infix() const;
    std::string postfix() const;

    std::string variables() const;

    double evaluate(const std::map<char, double>& values) const;
};
