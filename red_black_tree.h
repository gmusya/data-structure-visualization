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
    enum class Color { red, black };
    enum class Status { initial, touched, current, to_delete, rotate };
    enum class Kid { left, right, non };

    template<typename T>
    struct TreeInfo;

    template<typename T>
    class RedBlackTree {
    public:
        struct Node;
        using NodePtr = Node*;
        using Data = TreeInfo<T>;
        using ObserverModelViewPtr = Observer<Data>*;

        RedBlackTree()
            : port_([this]() {
                  return this->tree_info_;
              }) {
            PRINT_WHERE_AM_I();
        }

        void SubscribeToData(ObserverModelViewPtr observer) {
            port_.Subscribe(observer);
        }

        bool Insert(const T& value) {
            ClearTreeInfo();
            if (!root_) {
                root_ = std::unique_ptr<Node>(
                        new Node{nullptr, nullptr, nullptr, value, Color::black});
                ++size_;
                {
                    SetNodeStatus(root_.get(), Status::current);
                    tree_info_.root = Root();
                    Send();
                    SetNodeStatus(root_.get(), Status::touched);
                    Send();
                }
                return true;
            }
            NodePtr node = root_.get();
            SetNodeStatus(root_.get(), Status::current);
            Send();
            NodePtr parent = nullptr;
            while (node) {
                parent = node;
                if (value < node->value) {
                    SetNodeStatus(node, Status::touched);
                    node = node->left.get();
                    SetNodeStatus(node, Status::current);
                } else if (value == node->value) {
                    Send();
                    return false;
                } else {
                    SetNodeStatus(node, Status::touched);
                    node = node->right.get();
                    SetNodeStatus(node, Status::current);
                }
                Send();
            }

            ++size_;
            node = new Node{parent, nullptr, nullptr, value, Color::red};
            SetNodeStatus(node, Status::current);
            (value < parent->value ? parent->left : parent->right) = std::unique_ptr<Node>(node);
            Send();
            while (true) {
                if (!parent || parent->color == Color::black) {
                    if (!parent) {
                        node->color = Color::black;
                    }
                    tree_info_.root = Root();
                    Send();
                    SetNodeStatus(node, Status::touched);
                    Send();
                    return true;
                }
                if (node->GetUncle() && node->GetUncle()->color == Color::red) {
                    node->GetUncle()->color = Color::black;
                    node->parent->color = Color::black;
                    node->parent->parent->color = Color::red;
                    SetNodeStatus(node, Status::touched);
                    node = node->parent->parent;
                    SetNodeStatus(node, Status::current);
                    Send();
                    parent = node->parent;
                } else {
                    break;
                }
            }
            if (node->GetGrandParent()->WhichKid(node->parent) == Kid::left) {
                if (node->parent->WhichKid(node) == Kid::right) {
                    RotateLeft(node);
                    SetNodeStatus(node, Status::touched);
                    node = node->left.get();
                    SetNodeStatus(node, Status::current);
                    Send();
                }
                RotateRight(node->parent);
                node->parent->color = Color::black;
                node->parent->right->color = Color::red;
                if (!node->parent->parent) {
                    tree_info_.root = Root();
                }
                Send();
                SetNodeStatus(node, Status::touched);
                Send();
                return true;
            } else {
                if (node->parent->WhichKid(node) == Kid::left) {
                    RotateRight(node);
                    SetNodeStatus(node, Status::touched);
                    node = node->right.get();
                    SetNodeStatus(node, Status::current);
                    Send();
                }
                RotateLeft(node->parent);
                node->parent->color = Color::black;
                node->parent->left->color = Color::red;
                if (!node->parent->parent) {
                    tree_info_.root = Root();
                }
                Send();
                SetNodeStatus(node, Status::touched);
                Send();
                return true;
            }
        }

        bool Erase(const T& value) {
            ClearTreeInfo();
            NodePtr node = SearchValue(value);
            if (!node) {
                return false;
            }
            SetNodeStatus(node, Status::to_delete);
            Send();
            --size_;
            if (NodePtr node_to_delete = GetNearestLeaf(node)) {
                SetNodeStatus(node_to_delete, Status::current);
                Send();
                SetNodeStatus(node_to_delete, Status::to_delete);
                SetNodeStatus(node, Status::current);
                Send();
                node->value = node_to_delete->value;
                SetNodeStatus(node, Status::touched);
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
                assert(node->right.get() == nullptr);
                assert(node->left.get() == nullptr);
                node->parent = nullptr;
                node = ptr;
                SetNodeStatus(node, Status::current);
                Send();
            }
            while (true) {
                if (!parent) {
                    Send();
                    return true;
                }
                kid = parent->WhichKid(node);
                NodePtr sibling = (kid == Kid::left ? parent->right : parent->left).get();
                if (sibling->color == Color::red) {
                    parent->color = Color::red;
                    sibling->color = Color::black;
                    Rotate(sibling, kid);
                    sibling = (kid == Kid::left ? parent->right : parent->left).get();
                    tree_info_.root = root_.get();
                    Send();
                }
                if (GetNodeColor(sibling->left.get()) == Color::black &&
                    GetNodeColor(sibling->right.get()) == Color::black) {
                    if (parent->color == Color::black) {
                        sibling->color = Color::red;
                        node = parent;
                        parent = node->parent;
                        SetNodeStatus(node, Status::current);
                        Send();
                        continue;
                    } else {
                        parent->color = Color::black;
                        sibling->color = Color::red;
                        Send();
                        return true;
                    }
                }
                if ((kid == Kid::left && GetNodeColor(sibling->left.get()) == Color::red &&
                     GetNodeColor(sibling->right.get()) == Color::black) ||
                    (kid == Kid::right && GetNodeColor(sibling->left.get()) == Color::black &&
                     GetNodeColor(sibling->right.get()) == Color::red)) {
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
                Color color = parent->color;
                Rotate(sibling, kid);
                Send();
                parent->color = Color::black;
                (kid == Kid::left ? sibling->right : sibling->left)->color = Color::black;
                parent->parent->color = color;
                tree_info_.root = root_.get();
                Send();
                return true;
            }
        }

        bool Find(const T& value) {
            ClearTreeInfo();
            return SearchValue(value) != nullptr;
        }

        [[nodiscard]] size_t Size() const {
            return size_;
        }

        [[nodiscard]] bool Empty() const {
            return size_ == 0;
        }

        NodePtr Root() {
            return root_.get();
        }

    private:
        NodePtr FirstNode() const {
            if (!root_) {
                return nullptr;
            }
            NodePtr node = root_.get();
            while (node->left) {
                node = node->left.get();
            }
            return node;
        }

        Kid Opposite(Kid kid) {
            switch (kid) {
                case Kid::left:
                    return Kid::right;
                case Kid::right:
                    return Kid::left;
                default:
                    return Kid::non;
            }
        }

        void Rotate(typename RedBlackTree<T>::Node* node, Kid direction) {
            if (direction == Kid::left) {
                RotateLeft(node);
            } else {
                RotateRight(node);
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
        void RotateLeft(typename RedBlackTree<T>::Node* d) {
            PRINT_WHERE_AM_I();
            auto current_tree_info = GetTreeInfo(*this);
            NodePtr b = d->parent;
            NodePtr c = d->left.get();
            NodePtr pp = b->parent;
            Kid kid = Kid::non;
            if (pp) {
                kid = pp->WhichKid(b);
            }
            SetNodeStatus(b, Status::rotate, &current_tree_info);
            SetNodeStatus(b->left.get(), Status::rotate, &current_tree_info);
            SetNodeStatus(d, Status::rotate, &current_tree_info);
            SetNodeStatus(d->left.get(), Status::rotate, &current_tree_info);
            SetNodeStatus(d->right.get(), Status::rotate, &current_tree_info);
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
            root_.reset(UpdateRoot(old_root));
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
        void RotateRight(typename RedBlackTree<T>::Node* b) {
            PRINT_WHERE_AM_I();
            auto current_tree_info = GetTreeInfo(*this);
            NodePtr d = b->parent;
            NodePtr c = b->right.get();
            NodePtr pp = d->parent;
            Kid kid = Kid::non;
            if (pp) {
                kid = pp->WhichKid(d);
            }
            SetNodeStatus(d, Status::rotate, &current_tree_info);
            SetNodeStatus(b, Status::rotate, &current_tree_info);
            SetNodeStatus(b->left.get(), Status::rotate, &current_tree_info);
            SetNodeStatus(b->right.get(), Status::rotate, &current_tree_info);
            SetNodeStatus(d->right.get(), Status::rotate, &current_tree_info);
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
            root_.reset(UpdateRoot(old_root));
            tree_info_.root = root_.get();
            Send(current_tree_info);
        }

        NodePtr SearchValue(const T& value) {
            NodePtr node = root_.get();
            SetNodeStatus(node, Status::current);
            Send();
            while (node) {
                SetNodeStatus(node, Status::touched);
                if (value < node->value) {
                    node = node->left.get();
                } else if (value == node->value) {
                    break;
                } else {
                    node = node->right.get();
                }
                SetNodeStatus(node, Status::current);
                Send();
            }
            return node;
        }

        void Send() {
            tree_info_.tree_size = size_;
            tree_info_.root = root_.get();
            port_.Notify();
        }

        void Send(TreeInfo<T>& tree_info) {
            std::swap(tree_info_, tree_info);
            Send();
            std::swap(tree_info_, tree_info);
        }

    public:
#ifdef INVARIANTS_CHECK
        [[nodiscard]] bool CheckInvariants() const {
            std::vector<T> values;
            std::vector<int32_t> depths;
            if (!CheckInvariants(root_.get(), &values, &depths, 0)) {
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
#endif

        struct Node {
            NodePtr GetGrandParent() {
                if (!parent) {
                    return nullptr;
                } else {
                    return parent->parent;
                }
            }

            NodePtr GetUncle() {
                if (!parent) {
                    return nullptr;
                }
                if (!(parent->parent)) {
                    return nullptr;
                }
                return (parent->parent->right.get() == parent ? parent->parent->left.get()
                                                              : parent->parent->right.get());
            }

            Kid WhichKid(NodePtr kid) {
                if (left.get() == kid) {
                    return Kid::left;
                } else if (right.get() == kid) {
                    return Kid::right;
                } else {
                    return Kid::non;
                }
            }

            void Print(std::ostream& os, int depth) const {
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

            Node* parent;
            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;
            T value;
            Color color;
        };

        class ConstIterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;

            explicit ConstIterator(const Node* node) : node_(node) {
            }

            ConstIterator& operator++() {
                assert(node_);
                node_ = NextNode(node_);
                return *this;
            }

            ConstIterator operator++(int) {
                assert(node_);
                return ConstIterator(Node::NextNode(node_));
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
#ifdef INVARIANTS_CHECK
        bool CheckInvariants(NodePtr node, std::vector<T>* values, std::vector<int32_t>* depths,
                             int32_t black_depth) const {
            if (!node) {
                depths->push_back(black_depth + 1);
                return true;
            }
            if (black_depth == 0 && node->color == Color::red) {
                return false;
            }
            if (node->color == Color::black) {
                ++black_depth;
            } else {
                if ((node->left && node->left->color == Color::red) ||
                    (node->right && node->right->color == Color::red)) {
                    return false;
                }
            }
            if (!CheckInvariants(node->left.get(), values, depths, black_depth)) {
                return false;
            }
            values->push_back(node->value);
            if (!CheckInvariants(node->right.get(), values, depths, black_depth)) {
                return false;
            }
            return true;
        }
#endif
        static const Node* NextNode(const Node* node) {
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

        static Node* NextNode(Node* node) {
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

        NodePtr GetNearestLeaf(NodePtr node) {
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

        NodePtr UpdateRoot(NodePtr node) {
            if (!node) {
                return nullptr;
            }
            while (node->parent) {
                node = node->parent;
            }
            return node;
        }

        Color GetNodeColor(NodePtr node) {
            if (!node) {
                return Color::black;
            }
            return node->color;
        }

        void SetNodeStatus(NodePtr node, Status status) {
            tree_info_.node_to_status[node] = status;
        }

        void SetNodeStatus(NodePtr node, Status status, TreeInfo<T>* tree_info) {
            tree_info->node_to_status[node] = status;
        }

        void ClearTreeInfo() {
            tree_info_.node_to_status.clear();
        }

        std::unique_ptr<Node> root_ = nullptr;
        Observable<TreeInfo<T>> port_;
        TreeInfo<T> tree_info_;
        size_t size_ = 0;
    };

    template<typename T>
    struct TreeInfo {
        size_t tree_size = 0;
        const RedBlackTree<T>::Node* root = nullptr;
        std::map<const typename RedBlackTree<T>::Node*, Status> node_to_status;
    };

    template<typename T>
    TreeInfo<T> GetTreeInfo(RedBlackTree<T>& red_black_tree) {
        TreeInfo<T> tree_info;
        tree_info.root = red_black_tree.Root();
        return tree_info;
    }
}// namespace DSVisualization
