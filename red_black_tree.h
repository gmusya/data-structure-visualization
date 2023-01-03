#pragma once

#include <memory>
#include <sstream>
#ifdef INVARIANTS_CHECK
#include <optional>
#include <vector>
#endif

enum Kid {
    LEFT,
    RIGHT
};

template<typename T>
class RedBlackTree {
public:
    bool Erase(const T& value);
    bool Insert(const T& value);
    [[nodiscard]] size_t Size() const;
    [[nodiscard]] bool Empty() const;
    void Print(std::ostream& os) const;
    [[nodiscard]] std::string Str() const;

#ifdef INVARIANTS_CHECK
    [[nodiscard]] bool CheckInvariants() const;
#endif

private:
    class Node {
    public:
        explicit Node(const T& value1);
        Node(const T& value1, std::shared_ptr<Node> parent1, char color1);
        void Print(std::ostream& os, int depth) const;
        std::shared_ptr<Node> GetGrandParent();
        std::shared_ptr<Node> GetUncle();
        Kid WhichKid(std::shared_ptr<Node> kid);
        static void RotateLeft(std::shared_ptr<Node> d);
        static void RotateRight(std::shared_ptr<Node> b);

#ifdef INVARIANTS_CHECK
        static void CheckInvariants(std::shared_ptr<Node> node, std::vector<T>& values, std::vector<int32_t>& depths, int32_t black_depth);
#endif
        std::shared_ptr<Node> parent;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        T value;
        char color;
    };

    std::shared_ptr<Node> root_;
    size_t size_;
};


template<typename T>
bool RedBlackTree<T>::Insert(const T& value) {
    if (!root_) {
        root_ = std::make_shared<Node>(value);
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
    node = std::make_shared<Node>(value, parent, 'r');
    (value < parent->value ? parent->left : parent->right) = node;
    while (true) {
        if (!parent || parent->color == 'b') {
            if (!parent) {
                node->color = 'b';
            }
            return true;
        }
        if (node->GetUncle() && node->GetUncle()->color == 'r') {
            node->GetUncle()->color = 'b';
            node->parent->color = 'b';
            node->parent->parent->color = 'r';
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
        node->parent->color = 'b';
        node->parent->right->color = 'r';
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
        node->parent->color = 'b';
        node->parent->left->color = 'r';
        if (!node->parent->parent) {
            root_ = node->parent;
        }
        return true;
    }
}

template<typename T>
bool RedBlackTree<T>::Erase(const T& value) {
    throw std::logic_error("Not implemented");
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
    root_->Print(os, 0);
}
template<typename T>
std::string RedBlackTree<T>::Str() const {
    std::stringstream ss;
    Print(ss);
    return ss.str();
}

template<typename T>
RedBlackTree<T>::Node::Node(const T& value1) {
    parent = nullptr;
    left = nullptr;
    right = nullptr;
    value = value1;
    color = 'b';
}

template<typename T>
RedBlackTree<T>::Node::Node(const T& value1, std::shared_ptr<Node> parent1, char color1) {
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
    os << "(" << value << ", " << color << ")\n";
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
    if (values.size() == 9) {
        return true;
    }
    return *std::max_element(depths.begin(), depths.end()) == *std::min_element(depths.begin(), depths.end());
}

template<typename T>
void RedBlackTree<T>::Node::CheckInvariants(std::shared_ptr<Node> node, std::vector<T>& values, std::vector<int32_t>& depths, int32_t black_depth) {
    if (!node) {
        depths.push_back(black_depth + 1);
        return;
    }
    if (black_depth == 0 && node->color == 'r') {
        throw std::runtime_error("Root must be black");
    }
    if (node->color == 'b') {
        ++black_depth;
    } else {
        if ((node->left && node->left->color == 'r') || (node->right && node->right->color == 'r')) {
            throw std::runtime_error("Red node cannot have red kid");
        }
    }
    CheckInvariants(node->left, values, depths, black_depth);
    values.push_back(node->value);
    CheckInvariants(node->right, values, depths, black_depth);
}
#endif

/*
    |---c
|---b
|   |---d
a
|   |---f
|---e
    |---g
 */