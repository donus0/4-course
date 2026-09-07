#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

// Стек, реализованный на базе односвязного списка.
template <typename T>
class Stack {
private:
    struct Node {
        T data;
        Node* next;

        Node(const T& value, Node* nextNode)
            : data(value), next(nextNode) {}
    };

    Node* top_;

public:
    Stack() : top_(nullptr) {}

    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    ~Stack() {
        clear();
    }

    void push(const T& value) {
        top_ = new Node(value, top_);
    }

    void pop() {
        if (empty()) {
            throw std::out_of_range("Stack::pop: стек пуст");
        }
        Node* old = top_;
        top_ = top_->next;
        delete old;
    }

    T& top() {
        if (empty()) {
            throw std::out_of_range("Stack::top: стек пуст");
        }
        return top_->data;
    }

    const T& top() const {
        if (empty()) {
            throw std::out_of_range("Stack::top: стек пуст");
        }
        return top_->data;
    }

    bool empty() const {
        return top_ == nullptr;
    }

    std::size_t size() const {
        std::size_t count = 0;
        for (Node* node = top_; node != nullptr; node = node->next) {
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
