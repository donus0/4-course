#pragma once

#include <cstddef>

// Бинарное дерево поиска (BST) без балансировки: ключи хранятся так, что
// для любого узла всё левое поддерево меньше ключа, а правое — больше.
// Даёт операции вставки и поиска за O(h), где h — высота дерева.
template <typename Key, typename Value>
class BinaryTree {
private:
    struct Node {
        Key key;
        Value value;
        Node* left;
        Node* right;

        Node(const Key& k, const Value& v)
            : key(k), value(v), left(nullptr), right(nullptr) {}
    };

    Node* root_;
    std::size_t size_;

    static void destroy(Node* node) {
        if (node == nullptr) {
            return;
        }
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    static Node* findNode(Node* node, const Key& key) {
        while (node != nullptr) {
            if (key < node->key) {
                node = node->left;
            } else if (node->key < key) {
                node = node->right;
            } else {
                return node;
            }
        }
        return nullptr;
    }

    template <typename Visitor>
    static void inorder(Node* node, Visitor& visitor) {
        if (node == nullptr) {
            return;
        }
        inorder(node->left, visitor);
        visitor(node->key, node->value);
        inorder(node->right, visitor);
    }

public:
    BinaryTree() : root_(nullptr), size_(0) {}

    BinaryTree(const BinaryTree&) = delete;
    BinaryTree& operator=(const BinaryTree&) = delete;

    ~BinaryTree() {
        destroy(root_);
    }

    // Поиск значения по ключу. Возвращает указатель на значение или nullptr,
    // если ключ в дереве не встречается.
    Value* find(const Key& key) {
        Node* node = findNode(root_, key);
        return node == nullptr ? nullptr : &node->value;
    }

    const Value* find(const Key& key) const {
        Node* node = findNode(root_, key);
        return node == nullptr ? nullptr : &node->value;
    }

    // Вставляет ключ со значением по умолчанию, если его ещё нет, и
    // возвращает ссылку на значение (как std::map::operator[]).
    Value& operator[](const Key& key) {
        Node** slot = &root_;
        while (*slot != nullptr) {
            if (key < (*slot)->key) {
                slot = &(*slot)->left;
            } else if ((*slot)->key < key) {
                slot = &(*slot)->right;
            } else {
                return (*slot)->value;
            }
        }
        *slot = new Node(key, Value());
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
    void inorderTraversal(Visitor visitor) const {
        inorder(root_, visitor);
    }
};
