#pragma once

#include <cstddef>
#include <utility>

template <typename Key, typename Value>
class binary_tree final {
private:
    struct node final {
        Key key;
        Value value;
        node* left;
        node* right;

        node(const Key& k, const Value& v)
            : key(k), value(v), left(nullptr), right(nullptr) {}

        node(const node& other)
            : key(other.key), value(other.value), left(nullptr), right(nullptr) {
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

        node(node&& other) noexcept
            : key(std::move(other.key)), value(std::move(other.value)),
              left(other.left), right(other.right) {
            other.left = nullptr;
            other.right = nullptr;
        }

        node& operator=(const node& other) {
            if (this != &other) {
                node copy(other);
                swap(copy);
            }
            return *this;
        }

        node& operator=(node&& other) noexcept {
            if (this != &other) {
                node temp(std::move(other));
                swap(temp);
            }
            return *this;
        }

        ~node() {
            delete left;
            delete right;
        }

        void swap(node& other) noexcept {
            std::swap(key, other.key);
            std::swap(value, other.value);
            std::swap(left, other.left);
            std::swap(right, other.right);
        }
    };

    node* root_;
    std::size_t size_;

    static node* find_node(node* n, const Key& key) {
        while (n != nullptr) {
            if (key < n->key) {
                n = n->left;
            } else if (n->key < key) {
                n = n->right;
            } else {
                return n;
            }
        }
        return nullptr;
    }

    template <typename Visitor>
    static void inorder(node* n, Visitor& visitor) {
        if (n == nullptr) {
            return;
        }
        inorder(n->left, visitor);
        visitor(n->key, n->value);
        inorder(n->right, visitor);
    }

public:
    binary_tree() : root_(nullptr), size_(0) {}

    binary_tree(const binary_tree& other)
        : root_(other.root_ != nullptr ? new node(*other.root_) : nullptr), size_(other.size_) {}

    binary_tree(binary_tree&& other) noexcept : root_(other.root_), size_(other.size_) {
        other.root_ = nullptr;
        other.size_ = 0;
    }

    binary_tree& operator=(const binary_tree& other) {
        if (this != &other) {
            binary_tree copy(other);
            swap(copy);
        }
        return *this;
    }

    binary_tree& operator=(binary_tree&& other) noexcept {
        if (this != &other) {
            delete root_;
            root_ = other.root_;
            size_ = other.size_;
            other.root_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    ~binary_tree() {
        delete root_;
    }

    void swap(binary_tree& other) noexcept {
        std::swap(root_, other.root_);
        std::swap(size_, other.size_);
    }

    Value* find(const Key& key) {
        node* n = find_node(root_, key);
        return n == nullptr ? nullptr : &n->value;
    }

    const Value* find(const Key& key) const {
        node* n = find_node(root_, key);
        return n == nullptr ? nullptr : &n->value;
    }

    Value& operator[](const Key& key) {
        node** slot = &root_;
        while (*slot != nullptr) {
            if (key < (*slot)->key) {
                slot = &(*slot)->left;
            } else if ((*slot)->key < key) {
                slot = &(*slot)->right;
            } else {
                return (*slot)->value;
            }
        }
        *slot = new node(key, Value());
        ++size_;
        return (*slot)->value;
    }

    std::size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    template <typename Visitor>
    void inorder_traversal(Visitor visitor) const {
        inorder(root_, visitor);
    }
};
