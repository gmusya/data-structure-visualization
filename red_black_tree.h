#pragma once

#include <cassert>
#include <memory>
#include <sstream>
#include <vector>
#ifdef INVARIANTS_CHECK
#include <optional>
#include <vector>
#endif

enum Color { RED, BLACK };
enum Kid { LEFT, RIGHT };

template<typename T>
class RedBlackTree {
public:
    RedBlackTree();
    bool Erase(const T& value);
    bool Insert(const T& value);
    [[nodiscard]] size_t Size() const;
    [[nodiscard]] bool Empty() const;
    void Print(std::ostream& os) const;
    [[nodiscard]] std::string Str() const;
    std::vector<T> Values() const;
    ~RedBlackTree();

#ifdef INVARIANTS_CHECK
    [[nodiscard]] bool CheckInvariants() const;
#endif

private:
    class Node {
    public:
        explicit Node(const T& value1);
        Node(const T& value1, std::shared_ptr<Node> parent1, enum Color color1);
        void Print(std::ostream& os, int depth) const;
        std::shared_ptr<Node> GetGrandParent();
        std::shared_ptr<Node> GetUncle();
        Kid WhichKid(std::shared_ptr<Node> kid);
        static enum Color Color(std::shared_ptr<Node> node);
        static void RotateLeft(std::shared_ptr<Node> d);
        static void RotateRight(std::shared_ptr<Node> b);
        static void Unlink(std::shared_ptr<Node> node);
        static void Values(std::shared_ptr<Node> node, std::vector<T>& values);

#ifdef INVARIANTS_CHECK
        static void CheckInvariants(std::shared_ptr<Node> node, std::vector<T>& values, std::vector<int32_t>& depths,
                                    int32_t black_depth);
#endif
        std::shared_ptr<Node> parent;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        T value;
        enum Color color;
    };

    std::shared_ptr<Node> SearchValue(const T& value);
    void UpdateRoot();

    std::shared_ptr<Node> root_;
    size_t size_;
    std::shared_ptr<Node> GetNearestLeaf(std::shared_ptr<Node> node);
};

template<typename T>
RedBlackTree<T>::RedBlackTree() {
    root_ = nullptr;
    size_ = 0;
}

template<typename T>
bool RedBlackTree<T>::Insert(const T& value) {
    if (!root_) {
        root_ = std::make_shared<Node>(value);
        ++size_;
        return true;
    }
    std::shared_ptr<Node> node = root_;
    std::shared_ptr<Node> parent = nullptr;
    while (node) {
        parent = node;
        if (value < node->value) {
            node = node->left;
        } else if (value == node->value) {
            return false;
        } else {
            node = node->right;
        }
    }
    ++size_;
    node = std::make_shared<Node>(value, parent, Color::RED);
    (value < parent->value ? parent->left : parent->right) = node;
    while (true) {
        if (!parent || parent->color == Color::BLACK) {
            if (!parent) {
                node->color = Color::BLACK;
            }
            return true;
        }
        if (node->GetUncle() && node->GetUncle()->color == Color::RED) {
            node->GetUncle()->color = Color::BLACK;
            node->parent->color = Color::BLACK;
            node->parent->parent->color = Color::RED;
            node = node->parent->parent;
            parent = node->parent;
        } else {
            break;
        }
    }
    if (node->GetGrandParent()->WhichKid(node->parent) == Kid::LEFT) {
        if (node->parent->WhichKid(node) == Kid::RIGHT) {
            Node::RotateLeft(node);
            node = node->left;
        }
        Node::RotateRight(node->parent);
        node->parent->color = Color::BLACK;
        node->parent->right->color = Color::RED;
        if (!node->parent->parent) {
            root_ = node->parent;
        }
        return true;
    } else {
        if (node->parent->WhichKid(node) == Kid::LEFT) {
            Node::RotateRight(node);
            node = node->right;
        }
        Node::RotateLeft(node->parent);
        node->parent->color = Color::BLACK;
        node->parent->left->color = Color::RED;
        if (!node->parent->parent) {
            root_ = node->parent;
        }
        return true;
    }
}

template<typename T>
void RedBlackTree<T>::UpdateRoot() {
    while (root_ && root_->parent) {
        root_ = root_->parent;
    }
}

template<typename T>
std::shared_ptr<typename RedBlackTree<T>::Node> RedBlackTree<T>::SearchValue(const T& value) {
    std::shared_ptr<Node> node = root_;
    while (node) {
        if (value < node->value) {
            node = node->left;
        } else if (value == node->value) {
            break;
        } else {
            node = node->right;
        }
    }
    return node;
}

template<typename T>
std::shared_ptr<typename RedBlackTree<T>::Node> RedBlackTree<T>::GetNearestLeaf(std::shared_ptr<Node> node) {
    std::shared_ptr<Node> node_to_delete = node->right;
    if (node_to_delete) {
        while (node_to_delete->left) {
            node_to_delete = node_to_delete->left;
        }
    } else {
        if (node->left) {
            node_to_delete = node->left;
            while (node_to_delete->right) {
                node_to_delete = node_to_delete->right;
            }
        }
    }
    return node_to_delete;
}

template<typename T>
bool RedBlackTree<T>::Erase(const T& value) {
    std::shared_ptr<Node> node = SearchValue(value);
    if (!node) {
        return false;
    }
    --size_;
    if (std::shared_ptr<Node> node_to_delete = GetNearestLeaf(node)) {
        node->value = node_to_delete->value;
        node = node_to_delete;
    }
    if (!node->parent) {
        root_ = nullptr;
        return true;
    }
    Kid kid = node->parent->WhichKid(node);
    (kid == Kid::LEFT ? node->parent->left : node->parent->right) = node->right;
    if (node->right) {
        node->right->parent = node->parent;
    }
    if (node->color == Color::RED) {
        node->right = node->left = node->parent = nullptr;
        return true;
    }
    std::shared_ptr<Node> parent = node->parent;
    {
        std::shared_ptr<Node> nr = node->right;
        node->right = node->left = node->parent = nullptr;
        node = nr;
    }
    while (true) {
        if (!node) {
            kid = parent->WhichKid(node);
            std::shared_ptr<Node> sibling = (kid == Kid::LEFT ? parent->right : parent->left);
            if (sibling->color == Color::RED) {
                parent->color = Color::RED;
                sibling->color = Color::BLACK;
                (kid == Kid::LEFT ? Node::RotateLeft(sibling) : Node::RotateRight(sibling));
                sibling = (kid == Kid::LEFT ? parent->right : parent->left);
                UpdateRoot();
            }
            if (parent->color == Color::BLACK && (!sibling->left || sibling->left->color == Color::BLACK) &&
                (!sibling->right || sibling->right->color == Color::BLACK)) {
                sibling->color = Color::RED;
                node = parent;
                parent = node->parent;
                continue;
            }
            if (parent->color == Color::RED && (!sibling->left || sibling->left->color == Color::BLACK) &&
                (!sibling->right || sibling->right->color == Color::BLACK)) {
                parent->color = Color::BLACK;
                sibling->color = Color::RED;
                return true;
            }
            if ((kid == Kid::LEFT && Node::Color(sibling->left) == Color::RED && Node::Color(sibling->right) == Color::BLACK) ||
                (kid == Kid::RIGHT && Node::Color(sibling->left) == Color::BLACK && Node::Color(sibling->right) == Color::RED)) {
                if (kid == Kid::LEFT) {
                    Node::RotateRight(sibling->left);
                } else {
                    Node::RotateLeft(sibling->right);
                }
                sibling->color = Color::RED;
                sibling->parent->color = Color::BLACK;
                sibling = sibling->parent;
            }
            enum Color color = parent->color;
            if (kid == Kid::LEFT) {
                Node::RotateLeft(sibling);
            } else {
                Node::RotateRight(sibling);
            }
            parent->color = Color::BLACK;
            (kid == Kid::LEFT ? sibling->right : sibling->left)->color = Color::BLACK;
            parent->parent->color = color;
            UpdateRoot();
            return true;
        }
        if (!node->parent) {
            return true;
        }
        kid = node->parent->WhichKid(node);
        std::shared_ptr<Node> sibling = (kid == Kid::LEFT ? node->parent->right : node->parent->left);
        if (sibling->color == Color::RED) {
            node->parent->color = Color::RED;
            sibling->color = Color::BLACK;
            (kid == Kid::LEFT ? Node::RotateLeft(sibling) : Node::RotateRight(sibling));
            sibling = (kid == Kid::LEFT ? node->parent->right : node->parent->left);
        }
        if (node->parent->color == Color::BLACK && (!sibling->left || sibling->left->color == Color::BLACK) &&
            (!sibling->right || sibling->right->color == Color::BLACK)) {
            sibling->color = Color::RED;
            node = node->parent;
            continue;
        }
        if (node->parent->color == Color::RED && (!sibling->left || sibling->left->color == Color::BLACK) &&
            (!sibling->right || sibling->right->color == Color::BLACK)) {
            node->parent->color = Color::BLACK;
            sibling->color = Color::RED;
            UpdateRoot();
            return true;
        }
        if ((kid == Kid::LEFT && Node::Color(sibling->left) == Color::RED && Node::Color(sibling->right) == Color::BLACK) ||
            (kid == Kid::RIGHT && Node::Color(sibling->left) == Color::BLACK && Node::Color(sibling->right) == Color::RED)) {
            if (kid == Kid::LEFT) {
                Node::RotateRight(sibling->left);
            } else {
                Node::RotateLeft(sibling->right);
            }
            sibling->color = Color::RED;
            sibling->parent->color = Color::BLACK;
            sibling = sibling->parent;
        }
        enum Color color = node->parent->color;
        if (kid == Kid::LEFT) {
            Node::RotateLeft(sibling);
        } else {
            Node::RotateRight(sibling);
        }
        node->parent->color = Color::BLACK;
        (kid == Kid::LEFT ? sibling->right : sibling->left)->color = Color::BLACK;
        node->parent->parent->color = color;
        UpdateRoot();
        return true;
    }
}

template<typename T>
size_t RedBlackTree<T>::Size() const {
    return size_;
}

template<typename T>
bool RedBlackTree<T>::Empty() const {
    return size_ == 0;
}

template<typename T>
void RedBlackTree<T>::Print(std::ostream& os) const {
    if (!root_) {
        os << "Empty\n";
        return;
    }
    root_->Print(os, 0);
}

template<typename T>
std::string RedBlackTree<T>::Str() const {
    std::stringstream ss;
    Print(ss);
    return ss.str();
}

template<typename T>
RedBlackTree<T>::~RedBlackTree() {
    Node::Unlink(root_);
}

template<typename T>
std::vector<T> RedBlackTree<T>::Values() const {
    std::vector<T> values;
    Node::Values(root_, values);
    return values;
}

template<typename T>
RedBlackTree<T>::Node::Node(const T& value1) {
    parent = nullptr;
    left = nullptr;
    right = nullptr;
    value = value1;
    color = Color::BLACK;
}

template<typename T>
RedBlackTree<T>::Node::Node(const T& value1, std::shared_ptr<Node> parent1, enum Color color1) {
    parent = parent1;
    left = nullptr;
    right = nullptr;
    value = value1;
    color = color1;
}

void PrintLines(std::ostream& os, int32_t depth);

template<typename T>
void RedBlackTree<T>::Node::Print(std::ostream& os, int32_t depth) const {
    PrintLines(os, depth);
    os << "(" << value << ", " << (color == Color::RED ? 'r' : 'b') << ")\n";
    if (left) {
        left->Print(os, depth + 1);
    } else {
        PrintLines(os, depth + 1);
        os << "(NIL, b)\n";
    }
    if (right) {
        right->Print(os, depth + 1);
    } else {
        PrintLines(os, depth + 1);
        os << "(NIL, b)\n";
    }
}
template<typename T>
std::shared_ptr<typename RedBlackTree<T>::Node> RedBlackTree<T>::Node::GetGrandParent() {
    if (!parent) {
        return nullptr;
    } else {
        return parent->parent;
    }
}

template<typename T>
std::shared_ptr<typename RedBlackTree<T>::Node> RedBlackTree<T>::Node::GetUncle() {
    if (!parent) {
        return nullptr;
    }
    if (!(parent->parent)) {
        return nullptr;
    }
    return (parent->parent->right == parent ? parent->parent->left : parent->parent->right);
}

template<typename T>
Kid RedBlackTree<T>::Node::WhichKid(std::shared_ptr<Node> kid) {
    if (left == kid) {
        return Kid::LEFT;
    } else {
        return Kid::RIGHT;
    }
}

/*
       pp                                pp
       |                                 |
       b                                 d
-------|-------                   -------|-------
a            d      ------>       b             e
          ---|---              ---|---
          c     e              a     c
 */
template<typename T>
void RedBlackTree<T>::Node::RotateLeft(std::shared_ptr<Node> d) {
    std::shared_ptr<Node> b = d->parent;
    std::shared_ptr<Node> c = d->left;
    std::shared_ptr<Node> pp = b->parent;
    Kid kid;
    if (pp) {
        kid = pp->WhichKid(b);
    }
    b->parent = d;
    b->right = c;
    if (c) {
        c->parent = b;
    }
    d->left = b;
    d->parent = pp;
    if (pp) {
        if (kid == Kid::LEFT) {
            pp->left = d;
        } else {
            pp->right = d;
        }
    }
}

/*
          pp                                pp
          |                                  |
          d                                  b
   -------|-------                    -------|-------
   b             e      ------>       a             d
---|---                                          ---|---
a     c                                          c     e
 */
template<typename T>
void RedBlackTree<T>::Node::RotateRight(std::shared_ptr<Node> b) {
    std::shared_ptr<Node> d = b->parent;
    std::shared_ptr<Node> c = b->right;
    std::shared_ptr<Node> pp = d->parent;
    Kid kid;
    if (pp) {
        kid = pp->WhichKid(d);
    }
    d->parent = b;
    d->left = c;
    if (c) {
        c->parent = d;
    }
    b->right = d;
    b->parent = pp;
    if (pp) {
        if (kid == Kid::LEFT) {
            pp->left = b;
        } else {
            pp->right = b;
        }
    }
}
template<typename T>
void RedBlackTree<T>::Node::Unlink(std::shared_ptr<Node> node) {
    if (!node) {
        return;
    }
    Unlink(node->left);
    Unlink(node->right);
    node->left = nullptr;
    node->right = nullptr;
    node->parent = nullptr;
}

template<typename T>
enum Color RedBlackTree<T>::Node::Color(std::shared_ptr<Node> node) {
    if (!node) {
        return Color::BLACK;
    }
    return node->color;
}

template<typename T>
void RedBlackTree<T>::Node::Values(std::shared_ptr<Node> node, std::vector<T>& values) {
    if (!node) {
        return;
    }
    Values(node->left, values);
    values.push_back(node->value);
    Values(node->right, values);
}

#ifdef INVARIANTS_CHECK
template<typename T>
bool RedBlackTree<T>::CheckInvariants() const {
    std::vector<T> values;
    std::vector<int32_t> depths;
    try {
        Node::CheckInvariants(root_, values, depths, 0);
    } catch (std::runtime_error&) {
        return false;
    }
    for (size_t i = 0; i + 1 < values.size(); ++i) {
        if (values[i] > values[i + 1]) {
            return false;
        }
    }
    return *std::max_element(depths.begin(), depths.end()) == *std::min_element(depths.begin(), depths.end());
}

template<typename T>
void RedBlackTree<T>::Node::CheckInvariants(std::shared_ptr<Node> node, std::vector<T>& values, std::vector<int32_t>& depths,
                                            int32_t black_depth) {
    if (!node) {
        depths.push_back(black_depth + 1);
        return;
    }
    if (black_depth == 0 && node->color == Color::RED) {
        throw std::runtime_error("Root must be black");
    }
    if (node->color == Color::BLACK) {
        ++black_depth;
    } else {
        if ((node->left && node->left->color == Color::RED) || (node->right && node->right->color == Color::RED)) {
            throw std::runtime_error("Red node cannot have red kid");
        }
    }
    CheckInvariants(node->left, values, depths, black_depth);
    values.push_back(node->value);
    CheckInvariants(node->right, values, depths, black_depth);
}
#endif
