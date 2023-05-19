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

        void RotateLeft(typename RedBlackTree<T>::Node* d);
        void RotateRight(typename RedBlackTree<T>::Node* b);

    private:
        NodePtr SearchValue(const T& value);
        void Send();
        void Send(TreeInfo<T>& tree_info);

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
        void SetNodeStatus(NodePtr node, Status status);
        void SetNodeStatus(NodePtr node, Status status, TreeInfo<T>* tree_info);
        void ClearTreeInfo();
    };
    template<typename T>
    void RedBlackTree<T>::SetNodeStatus(RedBlackTree::NodePtr node, Status status,
                                        TreeInfo<T>* tree_info) {
        tree_info->node_to_status[node] = status;
    }

    template<typename T>
    void RedBlackTree<T>::Send(TreeInfo<T>& tree_info) {
        std::swap(tree_info_, tree_info);
        Send();
        std::swap(tree_info_, tree_info);
    }

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
    void RedBlackTree<T>::Send() {
        tree_info_.tree_size = size_;
        tree_info_.root = root_.get();
        port_.Notify();
    }

    template<typename T>
    void RedBlackTree<T>::SetNodeStatus(NodePtr node, Status status) {
        tree_info_.node_to_status[node] = status;
    }

    template<typename T>
    void RedBlackTree<T>::ClearTreeInfo() {
        tree_info_.node_to_status.clear();
    }

    template<typename T>
    bool RedBlackTree<T>::Insert(const T& value) {
        ClearTreeInfo();
        if (!root_) {
            root_ = std::unique_ptr<Node>(new Node{nullptr, nullptr, nullptr, value, black});
            ++size_;
            {
                SetNodeStatus(root_.get(), current);
                tree_info_.root = Root();
                Send();
                SetNodeStatus(root_.get(), touched);
                Send();
            }
            return true;
        }
        NodePtr node = root_.get();
        SetNodeStatus(root_.get(), current);
        Send();
        NodePtr parent = nullptr;
        while (node) {
            parent = node;
            if (value < node->value) {
                SetNodeStatus(node, touched);
                node = node->left.get();
                SetNodeStatus(node, current);
            } else if (value == node->value) {
                Send();
                return false;
            } else {
                SetNodeStatus(node, touched);
                node = node->right.get();
                SetNodeStatus(node, current);
            }
            Send();
        }

        ++size_;
        node = new Node{parent, nullptr, nullptr, value, Color::red};
        SetNodeStatus(node, current);
        (value < parent->value ? parent->left : parent->right) = std::unique_ptr<Node>(node);
        Send();
        while (true) {
            if (!parent || parent->color == Color::black) {
                if (!parent) {
                    node->color = Color::black;
                }
                tree_info_.root = Root();
                Send();
                SetNodeStatus(node, touched);
                Send();
                return true;
            }
            if (node->GetUncle() && node->GetUncle()->color == Color::red) {
                node->GetUncle()->color = Color::black;
                node->parent->color = Color::black;
                node->parent->parent->color = Color::red;
                SetNodeStatus(node, touched);
                node = node->parent->parent;
                SetNodeStatus(node, current);
                Send();
                parent = node->parent;
            } else {
                break;
            }
        }
        if (node->GetGrandParent()->WhichKid(node->parent) == Kid::left) {
            if (node->parent->WhichKid(node) == Kid::right) {
                RotateLeft(node);
                SetNodeStatus(node, touched);
                node = node->left.get();
                SetNodeStatus(node, current);
                Send();
            }
            RotateRight(node->parent);
            node->parent->color = Color::black;
            node->parent->right->color = Color::red;
            if (!node->parent->parent) {
                tree_info_.root = Root();
            }
            Send();
            SetNodeStatus(node, touched);
            Send();
            return true;
        } else {
            if (node->parent->WhichKid(node) == Kid::left) {
                RotateRight(node);
                SetNodeStatus(node, touched);
                node = node->right.get();
                SetNodeStatus(node, current);
                Send();
            }
            RotateLeft(node->parent);
            node->parent->color = Color::black;
            node->parent->left->color = Color::red;
            if (!node->parent->parent) {
                tree_info_.root = Root();
            }
            Send();
            SetNodeStatus(node, touched);
            Send();
            return true;
        }
    }

    template<typename T>
    bool RedBlackTree<T>::Erase(const T& value) {
        ClearTreeInfo();
        NodePtr node = SearchValue(value);
        if (!node) {
            return false;
        }
        SetNodeStatus(node, to_delete);
        Send();
        --size_;
        if (NodePtr node_to_delete = GetNearestLeaf(node)) {
            SetNodeStatus(node_to_delete, current);
            Send();
            SetNodeStatus(node_to_delete, to_delete);
            SetNodeStatus(node, current);
            Send();
            node->value = node_to_delete->value;
            SetNodeStatus(node, touched);
            Send();
            node = node_to_delete;
        }
        if (!node->parent) {
            root_ = nullptr;
            tree_info_.root = nullptr;
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
            Send();
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
            SetNodeStatus(node, current);
            Send();
        }
        while (true) {
            if (!node) {
                kid = parent->WhichKid(node);
                NodePtr sibling = (kid == Kid::left ? parent->right : parent->left).get();
                if (sibling->color == Color::red) {
                    parent->color = Color::red;
                    sibling->color = Color::black;
                    (kid == Kid::left ? RotateLeft(sibling) : RotateRight(sibling));
                    sibling = (kid == Kid::left ? parent->right : parent->left).get();
                    tree_info_.root = root_.get();
                    Send();
                }
                if (parent->color == Color::black &&
                    (!sibling->left || sibling->left->color == Color::black) &&
                    (!sibling->right || sibling->right->color == Color::black)) {
                    sibling->color = Color::red;
                    node = parent;
                    parent = node->parent;
                    SetNodeStatus(node, current);
                    Send();
                    continue;
                }
                if (parent->color == Color::red &&
                    (!sibling->left || sibling->left->color == Color::black) &&
                    (!sibling->right || sibling->right->color == Color::black)) {
                    parent->color = Color::black;
                    sibling->color = Color::red;
                    Send();
                    return true;
                }
                if ((kid == Kid::left && Node::GetColor(sibling->left.get()) == Color::red &&
                     Node::GetColor(sibling->right.get()) == Color::black) ||
                    (kid == Kid::right && Node::GetColor(sibling->left.get()) == Color::black &&
                     Node::GetColor(sibling->right.get()) == Color::red)) {
                    if (kid == Kid::left) {
                        RotateRight(sibling->left.get());
                        Send();
                    } else {
                        RotateLeft(sibling->right.get());
                        Send();
                    }
                    sibling->color = Color::red;
                    sibling->parent->color = Color::black;
                    sibling = sibling->parent;
                    Send();
                }
                enum Color color = parent->color;
                if (kid == Kid::left) {
                    RotateLeft(sibling);
                    Send();
                } else {
                    RotateRight(sibling);
                    Send();
                }
                parent->color = Color::black;
                (kid == Kid::left ? sibling->right : sibling->left)->color = Color::black;
                parent->parent->color = color;
                tree_info_.root = root_.get();
                Send();
                return true;
            }
            if (!node->parent) {
                Send();
                return true;
            }
            kid = node->parent->WhichKid(node);
            NodePtr sibling = (kid == Kid::left ? node->parent->right : node->parent->left).get();
            if (sibling->color == Color::red) {
                node->parent->color = Color::red;
                sibling->color = Color::black;
                (kid == Kid::left ? RotateLeft(sibling) : RotateRight(sibling));
                sibling = (kid == Kid::left ? node->parent->right : node->parent->left).get();
                Send();
            }
            if (node->parent->color == Color::black &&
                (!sibling->left || sibling->left->color == Color::black) &&
                (!sibling->right || sibling->right->color == Color::black)) {
                sibling->color = Color::red;
                SetNodeStatus(node, touched);
                node = node->parent;
                SetNodeStatus(node, current);
                Send();
                continue;
            }
            if (node->parent->color == Color::red &&
                (!sibling->left || sibling->left->color == Color::black) &&
                (!sibling->right || sibling->right->color == Color::black)) {
                node->parent->color = Color::black;
                sibling->color = Color::red;
                tree_info_.root = root_.get();
                Send();
                return true;
            }
            if ((kid == Kid::left && Node::GetColor(sibling->left.get()) == Color::red &&
                 Node::GetColor(sibling->right.get()) == Color::black) ||
                (kid == Kid::right && Node::GetColor(sibling->left.get()) == Color::black &&
                 Node::GetColor(sibling->right.get()) == Color::red)) {
                if (kid == Kid::left) {
                    RotateRight(sibling->left.get());
                    Send();
                } else {
                    RotateLeft(sibling->right.get());
                    Send();
                }
                sibling->color = Color::red;
                sibling->parent->color = Color::black;
                sibling = sibling->parent;
                Send();
            }
            enum Color color = node->parent->color;
            if (kid == Kid::left) {
                RotateLeft(sibling);
                Send();
            } else {
                RotateRight(sibling);
                Send();
            }
            node->parent->color = Color::black;
            (kid == Kid::left ? sibling->right : sibling->left)->color = Color::black;
            node->parent->parent->color = color;
            tree_info_.root = root_.get();
            Send();
            return true;
        }
    }

    template<typename T>
    bool RedBlackTree<T>::Find(const T& value) {
        ClearTreeInfo();
        return SearchValue(value) != nullptr;
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
    typename RedBlackTree<T>::Node* RedBlackTree<T>::SearchValue(const T& value) {
        NodePtr node = root_.get();
        SetNodeStatus(node, current);
        Send();
        while (node) {
            SetNodeStatus(node, touched);
            if (value < node->value) {
                node = node->left.get();
            } else if (value == node->value) {
                break;
            } else {
                node = node->right.get();
            }
            SetNodeStatus(node, current);
            Send();
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
    void RedBlackTree<T>::RotateLeft(typename RedBlackTree<T>::Node* d) {
        PRINT_WHERE_AM_I();
        auto current_tree_info = GetTreeInfo(*this);
        NodePtr b = d->parent;
        NodePtr c = d->left.get();
        NodePtr pp = b->parent;
        Kid kid = Kid::non;
        if (pp) {
            kid = pp->WhichKid(b);
        }
        SetNodeStatus(b, rotate, &current_tree_info);
        SetNodeStatus(b->left.get(), rotate, &current_tree_info);
        SetNodeStatus(d, rotate, &current_tree_info);
        SetNodeStatus(d->left.get(), rotate, &current_tree_info);
        SetNodeStatus(d->right.get(), rotate, &current_tree_info);
        Send(current_tree_info);
        NodePtr old_root = root_.release();
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
        root_.reset(Node::GetRoot(old_root));
        tree_info_.root = root_.get();
        Send(current_tree_info);
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
    void RedBlackTree<T>::RotateRight(typename RedBlackTree<T>::Node* b) {
        PRINT_WHERE_AM_I();
        auto current_tree_info = GetTreeInfo(*this);
        NodePtr d = b->parent;
        NodePtr c = b->right.get();
        NodePtr pp = d->parent;
        Kid kid = Kid::non;
        if (pp) {
            kid = pp->WhichKid(d);
        }
        SetNodeStatus(d, rotate, &current_tree_info);
        SetNodeStatus(b, rotate, &current_tree_info);
        SetNodeStatus(b->left.get(), rotate, &current_tree_info);
        SetNodeStatus(b->right.get(), rotate, &current_tree_info);
        SetNodeStatus(d->right.get(), rotate, &current_tree_info);
        Send(current_tree_info);
        NodePtr old_root = root_.release();
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
        root_.reset(Node::GetRoot(old_root));
        tree_info_.root = root_.get();
        Send(current_tree_info);
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
        if (black_depth == 0 && node->color == Color::red) {
            throw std::runtime_error("Root must be black");
        }
        if (node->color == Color::black) {
            ++black_depth;
        } else {
            if ((node->left && node->left->color == Color::red) ||
                (node->right && node->right->color == Color::red)) {
                throw std::runtime_error("red node cannot have red kid");
            }
        }
        CheckInvariants(node->left.get(), values, depths, black_depth);
        values.push_back(node->value);
        CheckInvariants(node->right.get(), values, depths, black_depth);
    }
#endif
}// namespace DSVisualization