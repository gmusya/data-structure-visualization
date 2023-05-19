#pragma once

#include "observable.h"
#include "observer.h"
#include "utility.h"

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#ifdef INVARIANTS_CHECK
#include <algorithm>
#include <optional>
#endif

namespace DSVisualization {
    enum Color { red, black };
    enum Status { initial, touched, current, to_delete, rotate };
    enum Kid { left, right, non };

    template<typename T>
    struct TreeInfo;

    template<typename T>
    class RedBlackTree {
    public:
        struct Node;
        using NodePtr = Node*;

        RedBlackTree();

        using Data = TreeInfo<T>;
        using ObserverModelViewPtr = Observer<Data>*;

        void SubscribeToData(ObserverModelViewPtr observer) {
            port_.Subscribe(observer);
        }

        bool Insert(const T& value);
        bool Erase(const T& value);
        bool Find(const T& value);

        [[nodiscard]] size_t Size() const;
        [[nodiscard]] bool Empty() const;

        NodePtr FirstNode() const;
        NodePtr Root();

        void RotateLeft(typename RedBlackTree<T>::Node* d, RedBlackTree<T>* tree);

        void RotateRight(typename RedBlackTree<T>::Node* b, RedBlackTree<T>* tree);

    private:
        NodePtr SearchValue(const T& value, TreeInfo<T>& tree_info);
        void Send(TreeInfo<T>& data);


    public:
#ifdef INVARIANTS_CHECK
        [[nodiscard]] bool CheckInvariants() const;
#endif

        struct Node {
        public:
            NodePtr GetGrandParent();
            NodePtr GetUncle();

            Kid WhichKid(NodePtr kid);


            static NodePtr GetRoot(NodePtr node);
            static const Node* Next(const Node* node);
            static Color GetColor(NodePtr node);

            void Print(std::ostream& os, int depth) const;

#ifdef INVARIANTS_CHECK
            static void CheckInvariants(NodePtr node, std::vector<T>& values,
                                        std::vector<int32_t>& depths, int32_t black_depth);
#endif
            Node* parent;
            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;
            T value;
            enum Color color;
        };

        class ConstIterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;

            explicit ConstIterator(const Node* node) : node_(node) {
            }

            ConstIterator& operator++() {
                assert(node_);
                node_ = Node::Next(node_);
                return *this;
            }

            ConstIterator operator++(int) {
                assert(node_);
                return ConstIterator(Node::Next(node_));
            }

            bool operator!=(const ConstIterator& other) const {
                return node_ != other.node_;
            }

            const T& operator*() {
                return node_->value;
            }

            const T* operator->() {
                return &node_->value;
            }

        private:
            const Node* node_;
        };

        ConstIterator begin() const {
            return ConstIterator(FirstNode());
        }

        ConstIterator end() const {
            return ConstIterator(nullptr);
        }

        friend std::ostream& operator<<(std::ostream& os, const RedBlackTree<T>& t) {
            if (!t.root_) {
                return os << "Empty\n";
            }
            t.root_->Print(os, 0);
            return os;
        }

    private:
        std::unique_ptr<Node> root_ = nullptr;
        Observable<TreeInfo<T>> port_;
        TreeInfo<T> tree_info_;
        size_t size_ = 0;
        NodePtr GetNearestLeaf(NodePtr node);
    };

    template<typename T>
    struct TreeInfo {
        using NodePtr = typename RedBlackTree<T>::Node*;
        size_t tree_size = 0;
        NodePtr root = nullptr;
        std::map<NodePtr, Status> node_to_status;
    };

    template<typename T>
    TreeInfo<T> GetTreeInfo(RedBlackTree<T>& red_black_tree) {
        TreeInfo<T> tree_info;
        tree_info.root = red_black_tree.Root();
        return tree_info;
    }

    template<typename T>
    RedBlackTree<T>::RedBlackTree()
        : port_([this]() {
              return this->tree_info_;
          }) {
        PRINT_WHERE_AM_I();
    }

    template<typename T>
    void RedBlackTree<T>::Send(TreeInfo<T>& data) {
        data.root = root_.get();
        data.tree_size = size_;
        tree_info_ = data;
        port_.Notify();
    }

    template<typename T>
    bool RedBlackTree<T>::Insert(const T& value) {
        auto data = GetTreeInfo(*this);
        if (!root_) {
            root_ = std::unique_ptr<Node>(new Node{nullptr, nullptr, nullptr, value, black});
            ++size_;
            {
                data.node_to_status[root_.get()] = Status::current;
                data.root = Root();
                Send(data);
                data.node_to_status[root_.get()] = Status::touched;
                Send(data);
            }
            return true;
        }
        NodePtr node = root_.get();
        data.node_to_status[root_.get()] = Status::current;
        Send(data);
        NodePtr parent = nullptr;
        while (node) {
            parent = node;
            if (value < node->value) {
                data.node_to_status[node] = Status::touched;
                node = node->left.get();
                data.node_to_status[node] = Status::current;
            } else if (value == node->value) {
                Send(data);
                return false;
            } else {
                data.node_to_status[node] = Status::touched;
                node = node->right.get();
                data.node_to_status[node] = Status::current;
            }
            Send(data);
        }

        ++size_;
        node = new Node{parent, nullptr, nullptr, value, Color::red};
        data.node_to_status[node] = Status::current;
        (value < parent->value ? parent->left : parent->right) = std::unique_ptr<Node>(node);
        Send(data);
        while (true) {
            if (!parent || parent->color == Color::black) {
                if (!parent) {
                    node->color = Color::black;
                }
                data.root = Root();
                Send(data);
                data.node_to_status[node] = Status::touched;
                Send(data);
                return true;
            }
            if (node->GetUncle() && node->GetUncle()->color == Color::red) {
                node->GetUncle()->color = Color::black;
                node->parent->color = Color::black;
                node->parent->parent->color = Color::red;
                data.node_to_status[node] = Status::touched;
                node = node->parent->parent;
                data.node_to_status[node] = Status::current;
                Send(data);
                parent = node->parent;
            } else {
                break;
            }
        }
        if (node->GetGrandParent()->WhichKid(node->parent) == Kid::left) {
            if (node->parent->WhichKid(node) == Kid::right) {
                RotateLeft(node, this);
                data.node_to_status[node] = Status::touched;
                node = node->left.get();
                data.node_to_status[node] = Status::current;
                Send(data);
            }
            RotateRight(node->parent, this);
            node->parent->color = Color::black;
            node->parent->right->color = Color::red;
            if (!node->parent->parent) {
                data.root = Root();
            }
            Send(data);
            data.node_to_status[node] = Status::touched;
            Send(data);
            return true;
        } else {
            if (node->parent->WhichKid(node) == Kid::left) {
                RotateRight(node, this);
                data.node_to_status[node] = Status::touched;
                node = node->right.get();
                data.node_to_status[node] = Status::current;
                Send(data);
            }
            RotateLeft(node->parent, this);
            node->parent->color = Color::black;
            node->parent->left->color = Color::red;
            if (!node->parent->parent) {
                data.root = Root();
            }
            Send(data);
            data.node_to_status[node] = Status::touched;
            Send(data);
            return true;
        }
    }

    template<typename T>
    bool RedBlackTree<T>::Erase(const T& value) {
        auto data = GetTreeInfo(*this);
        NodePtr node = SearchValue(value, data);
        if (!node) {
            return false;
        }
        data.node_to_status[node] = Status::to_delete;
        Send(data);
        --size_;
        if (NodePtr node_to_delete = GetNearestLeaf(node)) {
            data.node_to_status[node_to_delete] = Status::current;
            Send(data);
            data.node_to_status[node_to_delete] = Status::to_delete;
            data.node_to_status[node] = Status::current;
            Send(data);
            node->value = node_to_delete->value;
            data.node_to_status[node] = Status::touched;
            Send(data);
            node = node_to_delete;
        }
        if (!node->parent) {
            root_ = nullptr;
            data.root = nullptr;
            return true;
        }
        Kid kid = node->parent->WhichKid(node);
        auto ptr = node->right.release();
        (kid == Kid::left ? node->parent->left : node->parent->right).release();
        (kid == Kid::left ? node->parent->left : node->parent->right).reset(ptr);
        if (ptr) {
            ptr->parent = node->parent;
        }
        std::unique_ptr<Node> tmp(node);
        if (node->color == Color::red) {
            Send(data);
            return true;
        }
        NodePtr parent = node->parent;
        {
            NodePtr nr = ptr;
            assert(node->right.get() == nullptr);
            assert(node->left.get() == nullptr);
            node->left.release();
            node->parent = nullptr;
            node = nr;
            data.node_to_status[node] = Status::current;
            Send(data);
        }
        while (true) {
            if (!node) {
                kid = parent->WhichKid(node);
                NodePtr sibling = (kid == Kid::left ? parent->right : parent->left).get();
                if (sibling->color == Color::red) {
                    parent->color = Color::red;
                    sibling->color = Color::black;
                    (kid == Kid::left ? RotateLeft(sibling, this) : RotateRight(sibling, this));
                    sibling = (kid == Kid::left ? parent->right : parent->left).get();
                    data.root = root_.get();
                    Send(data);
                }
                if (parent->color == Color::black &&
                    (!sibling->left || sibling->left->color == Color::black) &&
                    (!sibling->right || sibling->right->color == Color::black)) {
                    sibling->color = Color::red;
                    node = parent;
                    parent = node->parent;
                    data.node_to_status[node] = Status::current;
                    Send(data);
                    continue;
                }
                if (parent->color == Color::red &&
                    (!sibling->left || sibling->left->color == Color::black) &&
                    (!sibling->right || sibling->right->color == Color::black)) {
                    parent->color = Color::black;
                    sibling->color = Color::red;
                    Send(data);
                    return true;
                }
                if ((kid == Kid::left && Node::GetColor(sibling->left.get()) == Color::red &&
                     Node::GetColor(sibling->right.get()) == Color::black) ||
                    (kid == Kid::right && Node::GetColor(sibling->left.get()) == Color::black &&
                     Node::GetColor(sibling->right.get()) == Color::red)) {
                    if (kid == Kid::left) {
                        RotateRight(sibling->left.get(), this);
                        Send(data);
                    } else {
                        RotateLeft(sibling->right.get(), this);
                        Send(data);
                    }
                    sibling->color = Color::red;
                    sibling->parent->color = Color::black;
                    sibling = sibling->parent;
                    Send(data);
                }
                enum Color color = parent->color;
                if (kid == Kid::left) {
                    RotateLeft(sibling, this);
                    Send(data);
                } else {
                    RotateRight(sibling, this);
                    Send(data);
                }
                parent->color = Color::black;
                (kid == Kid::left ? sibling->right : sibling->left)->color = Color::black;
                parent->parent->color = color;
                data.root = root_.get();
                Send(data);
                return true;
            }
            if (!node->parent) {
                Send(data);
                return true;
            }
            kid = node->parent->WhichKid(node);
            NodePtr sibling = (kid == Kid::left ? node->parent->right : node->parent->left).get();
            if (sibling->color == Color::red) {
                node->parent->color = Color::red;
                sibling->color = Color::black;
                (kid == Kid::left ? RotateLeft(sibling, this) : RotateRight(sibling, this));
                sibling = (kid == Kid::left ? node->parent->right : node->parent->left).get();
                Send(data);
            }
            if (node->parent->color == Color::black &&
                (!sibling->left || sibling->left->color == Color::black) &&
                (!sibling->right || sibling->right->color == Color::black)) {
                sibling->color = Color::red;
                data.node_to_status[node] = Status::touched;
                node = node->parent;
                data.node_to_status[node] = Status::current;
                Send(data);
                continue;
            }
            if (node->parent->color == Color::red &&
                (!sibling->left || sibling->left->color == Color::black) &&
                (!sibling->right || sibling->right->color == Color::black)) {
                node->parent->color = Color::black;
                sibling->color = Color::red;
                data.root = root_.get();
                Send(data);
                return true;
            }
            if ((kid == Kid::left && Node::GetColor(sibling->left.get()) == Color::red &&
                 Node::GetColor(sibling->right.get()) == Color::black) ||
                (kid == Kid::right && Node::GetColor(sibling->left.get()) == Color::black &&
                 Node::GetColor(sibling->right.get()) == Color::red)) {
                if (kid == Kid::left) {
                    RotateRight(sibling->left.get(), this);
                    Send(data);
                } else {
                    RotateLeft(sibling->right.get(), this);
                    Send(data);
                }
                sibling->color = Color::red;
                sibling->parent->color = Color::black;
                sibling = sibling->parent;
                Send(data);
            }
            enum Color color = node->parent->color;
            if (kid == Kid::left) {
                RotateLeft(sibling, this);
                Send(data);
            } else {
                RotateRight(sibling, this);
                Send(data);
            }
            node->parent->color = Color::black;
            (kid == Kid::left ? sibling->right : sibling->left)->color = Color::black;
            node->parent->parent->color = color;
            data.root = root_.get();
            Send(data);
            return true;
        }
    }

    template<typename T>
    bool RedBlackTree<T>::Find(const T& value) {
        auto data = GetTreeInfo(*this);
        return SearchValue(value, data) != nullptr;
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
    typename RedBlackTree<T>::Node* RedBlackTree<T>::FirstNode() const {
        if (!root_) {
            return nullptr;
        }
        NodePtr node = root_.get();
        while (node->left) {
            node = node->left.get();
        }
        return node;
    }

    template<typename T>
    typename RedBlackTree<T>::Node* RedBlackTree<T>::Root() {
        return root_.get();
    }

    template<typename T>
    typename RedBlackTree<T>::Node* RedBlackTree<T>::SearchValue(const T& value,
                                                                 TreeInfo<T>& tree_info) {
        NodePtr node = root_.get();
        tree_info.node_to_status[node] = Status::current;
        Send(tree_info);
        while (node) {
            tree_info.node_to_status[node] = Status::touched;
            if (value < node->value) {
                node = node->left.get();
            } else if (value == node->value) {
                break;
            } else {
                node = node->right.get();
            }
            tree_info.node_to_status[node] = Status::current;
            Send(tree_info);
        }
        return node;
    }

    template<typename T>
    typename RedBlackTree<T>::Node* RedBlackTree<T>::GetNearestLeaf(NodePtr node) {
        NodePtr node_to_delete = node->right.get();
        if (node_to_delete) {
            while (node_to_delete->left) {
                node_to_delete = node_to_delete->left.get();
            }
        } else {
            if (node->left) {
                node_to_delete = node->left.get();
                while (node_to_delete->right) {
                    node_to_delete = node_to_delete->right.get();
                }
            }
        }
        return node_to_delete;
    }

    template<typename T>
    typename RedBlackTree<T>::Node* RedBlackTree<T>::Node::GetGrandParent() {
        if (!parent) {
            return nullptr;
        } else {
            return parent->parent;
        }
    }

    template<typename T>
    typename RedBlackTree<T>::Node* RedBlackTree<T>::Node::GetUncle() {
        if (!parent) {
            return nullptr;
        }
        if (!(parent->parent)) {
            return nullptr;
        }
        return (parent->parent->right.get() == parent ? parent->parent->left.get()
                                                      : parent->parent->right.get());
    }

    template<typename T>
    typename RedBlackTree<T>::Node* RedBlackTree<T>::Node::GetRoot(NodePtr node) {
        if (!node) {
            return nullptr;
        }
        while (node->parent) {
            node = node->parent;
        }
        return node;
    }

    template<typename T>
    const typename RedBlackTree<T>::Node* RedBlackTree<T>::Node::Next(const Node* node) {
        if (node->right) {
            node = node->right.get();
            while (node->left) {
                node = node->left.get();
            }
            return node;
        }
        while (node->parent && node->parent->right.get() == node) {
            node = node->parent;
        }
        if (!node->parent) {
            return nullptr;
        }
        node = node->parent;
        return node;
    }

    template<typename T>
    Kid RedBlackTree<T>::Node::WhichKid(NodePtr kid) {
        if (left.get() == kid) {
            return Kid::left;
        } else if (right.get() == kid) {
            return Kid::right;
        } else {
            return Kid::non;
        }
    }

    template<typename T>
    enum Color RedBlackTree<T>::Node::GetColor(NodePtr node) {
        if (!node) {
            return Color::black;
        }
        return node->color;
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
    void RedBlackTree<T>::RotateLeft(typename RedBlackTree<T>::Node* d, RedBlackTree<T>* tree) {
        PRINT_WHERE_AM_I();
        auto tree_info = GetTreeInfo(*tree);
        NodePtr b = d->parent;
        NodePtr c = d->left.get();
        NodePtr pp = b->parent;
        Kid kid = Kid::non;
        if (pp) {
            kid = pp->WhichKid(b);
        }
        tree_info.node_to_status[b] = tree_info.node_to_status[b->left.get()] =
                tree_info.node_to_status[d] = tree_info.node_to_status[d->left.get()] =
                        tree_info.node_to_status[d->right.get()] = Status::rotate;
        Send(tree_info);
        root_.release();
        d->left.release();
        b->right.release();
        b->parent = d;
        b->right.reset(c);
        if (c) {
            c->parent = b;
        }
        d->left.reset(b);
        d->parent = pp;
        if (pp) {
            if (kid == Kid::left) {
                pp->left.release();
                pp->left.reset(d);
            } else {
                pp->right.release();
                pp->right.reset(d);
            }
        }
        root_.reset(Node::GetRoot(d));
        tree_info.root = root_.get();
        Send(tree_info);
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
    void RedBlackTree<T>::RotateRight(typename RedBlackTree<T>::Node* b, RedBlackTree<T>* tree) {
        PRINT_WHERE_AM_I();
        auto tree_info = GetTreeInfo(*tree);
        NodePtr d = b->parent;
        NodePtr c = b->right.get();
        NodePtr pp = d->parent;
        Kid kid = Kid::non;
        if (pp) {
            kid = pp->WhichKid(d);
        }
        tree_info.node_to_status[d] = tree_info.node_to_status[b] =
                tree_info.node_to_status[b->left.get()] = tree_info.node_to_status[b->right.get()] =
                        tree_info.node_to_status[d->right.get()] = Status::rotate;
        Send(tree_info);
        root_.release();
        d->left.release();
        b->right.release();
        d->parent = b;
        d->left.reset(c);
        if (c) {
            c->parent = d;
        }
        b->right.reset(d);
        b->parent = pp;
        if (pp) {
            if (kid == Kid::left) {
                pp->left.release();
                pp->left.reset(b);
            } else {
                pp->right.release();
                pp->right.reset(b);
            }
        }
        root_.reset(Node::GetRoot(b));
        tree_info.root = root_.get();
        Send(tree_info);
    }

    template<typename T>
    void RedBlackTree<T>::Node::Print(std::ostream& os, int32_t depth) const {
        static auto PrintLines = [](std::ostream& os, int32_t depth) {
            if (depth > 0) {
                for (int32_t i = 1; i < depth; ++i) {
                    os << "|   ";
                }
                os << "|---";
            }
        };
        PrintLines(os, depth);
        os << "(" << value << ", " << (color == Color::red ? 'r' : 'b') << ")\n";
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

#ifdef INVARIANTS_CHECK
    template<typename T>
    bool RedBlackTree<T>::CheckInvariants() const {
        std::vector<T> values;
        std::vector<int32_t> depths;
        try {
            Node::CheckInvariants(root_.get(), values, depths, 0);
        } catch (std::runtime_error&) {
            return false;
        }
        for (size_t i = 0; i + 1 < values.size(); ++i) {
            if (values[i] > values[i + 1]) {
                return false;
            }
        }
        return *std::max_element(depths.begin(), depths.end()) ==
               *std::min_element(depths.begin(), depths.end());
    }


    template<typename T>
    void RedBlackTree<T>::Node::CheckInvariants(NodePtr node, std::vector<T>& values,
                                                std::vector<int32_t>& depths, int32_t black_depth) {
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
            if ((node->left && node->left->color == Color::RED) ||
                (node->right && node->right->color == Color::RED)) {
                throw std::runtime_error("Red node cannot have red kid");
            }
        }
        CheckInvariants(node->left.get(), values, depths, black_depth);
        values.push_back(node->value);
        CheckInvariants(node->right.get(), values, depths, black_depth);
    }
#endif
}// namespace DSVisualization