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
    enum class Status { initial, touched, current, to_delete, rotate, found };
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
            : port_([]() {
                  return TreeInfo<T>{0, nullptr, {}};
              }) {
            PRINT_WHERE_AM_I();
        }

        ~RedBlackTree() {
            port_.SendByReference({});
        }

        void SubscribeToData(ObserverModelViewPtr observer) {
            port_.Subscribe(observer);
        }

        class TreeInfoWrapper : public TreeInfo<T> {
        public:
            TreeInfoWrapper(TreeInfo<T> tree_info, std::function<void(TreeInfo<T>)> deleter)
                : TreeInfo<T>(std::move(tree_info)), deleter_(std::move(deleter)) {
            }

            TreeInfo<T>& GetTreeInfo() {
                return *static_cast<TreeInfo<T>*>(this);
            }

            ~TreeInfoWrapper() {
                deleter_(std::move(GetTreeInfo()));
            }

        private:
            std::function<void(TreeInfo<T>)> deleter_;
        };

        bool Insert(const T& value) {
            if (!root_) {
                root_ = std::unique_ptr<Node>(
                        new Node{nullptr, nullptr, nullptr, value, Color::black});
                ++size_;
                TreeInfoWrapper tree_info_wrapper({size_, root_.get(), {}},
                                                  [this](TreeInfo<T> tree_info) {
                                                      port_.SendByValue(std::move(tree_info));
                                                  });
                port_.SendByReference(
                        tree_info_wrapper.SetNodeStatus(root_.get(), Status::current));
                port_.SendByReference(
                        tree_info_wrapper.SetNodeStatus(root_.get(), Status::touched));
                return true;
            }
            TreeInfoWrapper tree_info_wrapper({size_, root_.get(), {}},
                                              [this](TreeInfo<T> tree_info) {
                                                  port_.SendByValue(std::move(tree_info));
                                              });
            NodePtr parent = SearchNearValue(value, static_cast<TreeInfo<T>*>(&tree_info_wrapper));
            if (parent != nullptr && parent->value == value) {
                return false;
            }
            ++size_;
            auto node = new Node{parent, nullptr, nullptr, value, Color::red};
            port_.SendByReference(tree_info_wrapper.SetNodeStatus(node, Status::current));
            (value < parent->value ? parent->left : parent->right) = std::unique_ptr<Node>(node);
            while (GetNodeColor(parent) == Color::black ||
                   GetNodeColor(node->GetUncle()) == Color::red) {
                if (GetNodeColor(parent) == Color::black) {
                    if (!parent) {
                        node->color = Color::black;
                    }
                    tree_info_wrapper.root = Root();
                    port_.SendByReference(tree_info_wrapper.SetNodeStatus(node, Status::touched));
                    return true;
                } else {
                    node->GetUncle()->color = Color::black;
                    node->parent->color = Color::black;
                    node->GetGrandParent()->color = Color::red;
                    tree_info_wrapper.SetNodeStatus(node, Status::touched);
                    node = node->GetGrandParent();
                    port_.SendByReference(tree_info_wrapper.SetNodeStatus(node, Status::current));
                    parent = node->parent;
                }
            }
            Kid parent_grandparent = node->GetGrandParent()->WhichKid(node->parent);
            Kid node_parent = node->parent->WhichKid(node);
            if (node_parent == Opposite(parent_grandparent)) {
                Rotate(node, parent_grandparent);
                tree_info_wrapper.root = Root();
                port_.SendByReference(tree_info_wrapper);
                tree_info_wrapper.SetNodeStatus(node, Status::touched);
                node = GetKid(node, parent_grandparent).get();
                port_.SendByReference(tree_info_wrapper.SetNodeStatus(node, Status::current));
            }
            Rotate(node->parent, Opposite(parent_grandparent));
            tree_info_wrapper.root = Root();
            port_.SendByReference(tree_info_wrapper);
            node->parent->color = Color::black;
            GetKid(node->parent, Opposite(parent_grandparent))->color = Color::red;
            if (!node->GetGrandParent()) {
                tree_info_wrapper.root = Root();
            }
            port_.SendByReference(tree_info_wrapper.SetNodeStatus(node, Status::touched));
            return true;
        }

        bool Erase(const T& value) {
            TreeInfoWrapper tree_info_wrapper({size_, root_.get(), {}},
                                              [this](TreeInfo<T> tree_info) {
                                                  port_.SendByValue(std::move(tree_info));
                                              });
            NodePtr node = SearchNearValue(value, &tree_info_wrapper.GetTreeInfo());
            if (!node || node->value != value) {
                return false;
            }
            port_.SendByReference(tree_info_wrapper.SetNodeStatus(node, Status::to_delete));
            --size_;
            if (NodePtr node_to_delete = GetNearestLeaf(node)) {
                port_.SendByReference(
                        tree_info_wrapper.SetNodeStatus(node_to_delete, Status::current));
                tree_info_wrapper.SetNodeStatus(node_to_delete, Status::to_delete);
                port_.SendByReference(tree_info_wrapper.SetNodeStatus(node, Status::current));
                node->value = node_to_delete->value;
                port_.SendByReference(tree_info_wrapper.SetNodeStatus(node, Status::touched));
                node = node_to_delete;
            }
            if (!node->parent) {
                root_ = nullptr;
                tree_info_wrapper.root = nullptr;
                port_.SendByReference(tree_info_wrapper);
                return true;
            }
            Kid kid = node->parent->WhichKid(node);
            auto ptr = node->right.release();
            GetKid(node->parent, kid).release();
            GetKid(node->parent, kid).reset(ptr);
            if (ptr) {
                ptr->parent = node->parent;
            }
            std::unique_ptr<Node> tmp(node);
            if (node->color == Color::red) {
                port_.SendByReference(tree_info_wrapper);
                return true;
            }
            NodePtr parent = node->parent;
            node = ptr;
            port_.SendByReference(tree_info_wrapper.SetNodeStatus(node, Status::current));
            while (parent) {
                kid = parent->WhichKid(node);
                NodePtr sibling = GetKid(parent, Opposite(kid)).get();
                if (sibling->color == Color::red) {
                    parent->color = Color::red;
                    sibling->color = Color::black;
                    Rotate(sibling, kid);
                    sibling = GetKid(parent, Opposite(kid)).get();
                    tree_info_wrapper.root = Root();
                    port_.SendByReference(tree_info_wrapper);
                }
                if (GetNodeColor(sibling->left.get()) == Color::black &&
                    GetNodeColor(sibling->right.get()) == Color::black) {
                    if (parent->color == Color::black) {
                        sibling->color = Color::red;
                        node = parent;
                        parent = node->parent;
                        port_.SendByReference(
                                tree_info_wrapper.SetNodeStatus(node, Status::current));
                        continue;
                    } else {
                        parent->color = Color::black;
                        sibling->color = Color::red;
                        port_.SendByReference(tree_info_wrapper);
                        return true;
                    }
                }
                if (GetNodeColor(GetKid(sibling, kid).get()) == Color::red &&
                    GetNodeColor(GetKid(sibling, Opposite(kid)).get()) == Color::black) {
                    Rotate(GetKid(sibling, kid).get(), Opposite(kid));
                    tree_info_wrapper.root = Root();
                    port_.SendByReference(tree_info_wrapper);
                    sibling->color = Color::red;
                    sibling->parent->color = Color::black;
                    sibling = sibling->parent;
                    port_.SendByReference(tree_info_wrapper);
                }
                Color color = parent->color;
                Rotate(sibling, kid);
                tree_info_wrapper.root = Root();
                port_.SendByReference(tree_info_wrapper);
                parent->color = Color::black;
                GetKid(sibling, Opposite(kid))->color = Color::black;
                parent->parent->color = color;
                tree_info_wrapper.root = root_.get();
                port_.SendByReference(tree_info_wrapper);
                return true;
            }
            return true;
        }

        bool Find(const T& value) {
            TreeInfoWrapper tree_info_wrapper({size_, root_.get(), {}},
                                              [this](TreeInfo<T> tree_info) {
                                                  port_.SendByValue(std::move(tree_info));
                                              });
            auto result = SearchNearValue(value, &tree_info_wrapper.GetTreeInfo());
            if (result != nullptr && result->value == value) {
                port_.SendByReference(tree_info_wrapper.SetNodeStatus(result, Status::found));
                return true;
            } else {
                return false;
            }
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

        std::unique_ptr<Node>& GetKid(NodePtr node, Kid direction) {
            if (direction == Kid::left) {
                return node->left;
            } else {
                return node->right;
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
            TreeInfoWrapper tree_info_wrapper({size_, root_.get(), {}},
                                              [this](TreeInfo<T> tree_info) {
                                                  port_.SendByValue(std::move(tree_info));
                                              });
            auto current_tree_info = GetTreeInfo(*this);
            NodePtr b = d->parent;
            NodePtr c = d->left.get();
            NodePtr pp = b->parent;
            Kid kid = Kid::non;
            if (pp) {
                kid = pp->WhichKid(b);
            }
            port_.SendByReference(tree_info_wrapper.SetNodeStatus(b, Status::rotate)
                                          .SetNodeStatus(b->left.get(), Status::rotate)
                                          .SetNodeStatus(d, Status::rotate)
                                          .SetNodeStatus(d->left.get(), Status::rotate)
                                          .SetNodeStatus(d->right.get(), Status::rotate));
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
            tree_info_wrapper.root = root_.get();
            port_.SendByReference(tree_info_wrapper);
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
            TreeInfoWrapper tree_info_wrapper({size_, root_.get(), {}},
                                              [this](TreeInfo<T> tree_info) {
                                                  port_.SendByValue(std::move(tree_info));
                                              });
            auto current_tree_info = GetTreeInfo(*this);
            NodePtr d = b->parent;
            NodePtr c = b->right.get();
            NodePtr pp = d->parent;
            Kid kid = Kid::non;
            if (pp) {
                kid = pp->WhichKid(d);
            }
            port_.SendByReference(tree_info_wrapper.SetNodeStatus(d, Status::rotate)
                                          .SetNodeStatus(b, Status::rotate)
                                          .SetNodeStatus(b->left.get(), Status::rotate)
                                          .SetNodeStatus(b->right.get(), Status::rotate)
                                          .SetNodeStatus(d->right.get(), Status::rotate));
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
            tree_info_wrapper.root = root_.get();
            port_.SendByReference(tree_info_wrapper);
        }

        NodePtr SearchNearValue(const T& value, TreeInfo<T>* tree_info) {
            NodePtr node = root_.get();
            port_.SendByReference(tree_info->SetNodeStatus(node, Status::current));
            while (node) {
                tree_info->SetNodeStatus(node, Status::touched);
                if (value < node->value) {
                    if (!node->left) {
                        break;
                    }
                    node = node->left.get();
                } else if (value == node->value) {
                    break;
                } else {
                    if (!node->right) {
                        break;
                    }
                    node = node->right.get();
                }
                port_.SendByReference(tree_info->SetNodeStatus(node, Status::current));
            }
            return node;
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

        std::unique_ptr<Node> root_ = nullptr;
        Observable<TreeInfo<T>> port_;
        size_t size_ = 0;
    };

    template<typename T>
    struct TreeInfo {
        size_t tree_size = 0;
        const typename RedBlackTree<T>::Node* root = nullptr;
        std::unordered_map<const typename RedBlackTree<T>::Node*, Status> node_to_status;

        TreeInfo<T>& SetNodeStatus(const typename RedBlackTree<T>::Node* node, Status status) {
            node_to_status[node] = status;
            return *this;
        }
    };

    template<typename T>
    TreeInfo<T> GetTreeInfo(RedBlackTree<T>& red_black_tree) {
        TreeInfo<T> tree_info;
        tree_info.root = red_black_tree.Root();
        return tree_info;
    }
}// namespace DSVisualization
