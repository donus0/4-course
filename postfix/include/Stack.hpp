#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

// Стек, реализованный на базе односвязного списка.
template <typename T>
class stack {
private:
    struct node {
        T data;
        node* next;

        node(const T& value, node* next_node)
            : data(value), next(next_node) {}
    };

    node* top_;

public:
    stack() : top_(nullptr) {}

    stack(const stack&) = delete;
    stack& operator=(const stack&) = delete;

    ~stack() {
        clear();
    }

    void push(const T& value) {
        top_ = new node(value, top_);
    }

    void pop() {
        if (empty()) {
            throw std::out_of_range("stack::pop: стек пуст");
        }
        node* old = top_;
        top_ = top_->next;
        delete old;
    }

    T& top() {
        if (empty()) {
            throw std::out_of_range("stack::top: стек пуст");
        }
        return top_->data;
    }

    const T& top() const {
        if (empty()) {
            throw std::out_of_range("stack::top: стек пуст");
        }
        return top_->data;
    }

    bool empty() const {
        return top_ == nullptr;
    }

    std::size_t size() const {
        std::size_t count = 0;
        for (node* n = top_; n != nullptr; n = n->next) {
            ++count;
        }
        return count;
    }

    void clear() {
        while (!empty()) {
            pop();
        }
    }
};
