#pragma once

#include <cstddef>

// Бинарное дерево поиска (BST) без балансировки: ключи хранятся так, что
// для любого узла всё левое поддерево меньше ключа, а правое — больше.
// Даёт операции вставки и поиска за O(h), где h — высота дерева.
template <typename Key, typename Value>
class binary_tree {
private:
    struct node {
        Key key;
        Value value;
        node* left;
        node* right;

        node(const Key& k, const Value& v)
            : key(k), value(v), left(nullptr), right(nullptr) {}
    };

    node* root_;
    std::size_t size_;

    static void destroy(node* n) {
        if (n == nullptr) {
            return;
        }
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

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

    binary_tree(const binary_tree&) = delete;
    binary_tree& operator=(const binary_tree&) = delete;

    ~binary_tree() {
        destroy(root_);
    }

    // Поиск значения по ключу. Возвращает указатель на значение или nullptr,
    // если ключ в дереве не встречается.
    Value* find(const Key& key) {
        node* n = find_node(root_, key);
        return n == nullptr ? nullptr : &n->value;
    }

    const Value* find(const Key& key) const {
        node* n = find_node(root_, key);
        return n == nullptr ? nullptr : &n->value;
    }

    // Вставляет ключ со значением по умолчанию, если его ещё нет, и
    // возвращает ссылку на значение (как std::map::operator[]).
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

    // Обходит дерево в порядке возрастания ключей (левое поддерево, узел,
    // правое поддерево), вызывая visitor(key, value) для каждого узла.
    template <typename Visitor>
    void inorder_traversal(Visitor visitor) const {
        inorder(root_, visitor);
    }
};
