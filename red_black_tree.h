#pragma once

#include "observable.h"
#include "observer.h"

#include <cassert>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#ifdef INVARIANTS_CHECK
#include <algorithm>
#include <optional>
#include <vector>
#endif

namespace DSVisualization {
    enum Color { RED, BLACK };
    enum Status { DEFAULT, TOUCHED, CURRENT, TO_DELETE, ROTATE };
    enum Kid { LEFT, RIGHT };

    template<typename T>
    class TreeInfo;

    template<typename T>
    class RedBlackTree {
    public:
        RedBlackTree();
        bool Erase(const T& value, std::shared_ptr<Observable<TreeInfo<int>>> port = nullptr);
        bool Insert(const T& value, std::shared_ptr<Observable<TreeInfo<int>>> port = nullptr);
        [[nodiscard]] size_t Size() const;
        [[nodiscard]] bool Empty() const;
        void Print(std::ostream& os) const;
        [[nodiscard]] std::string Str() const;
        std::vector<T> Values() const;
        ~RedBlackTree();

#ifdef INVARIANTS_CHECK
        [[nodiscard]] bool CheckInvariants() const;
#endif

        class Node {
        public:
            using NodePtr = std::shared_ptr<Node>;
            explicit Node(const T& value1);
            Node(const T& value1, NodePtr parent1, enum Color color1);
            void Print(std::ostream& os, int depth) const;
            NodePtr GetGrandParent();
            NodePtr GetUncle();
            static NodePtr GetRoot(NodePtr node);
            Kid WhichKid(NodePtr kid);
            static enum Color Color(NodePtr node);
            static void RotateLeft(NodePtr d,
                                   const std::shared_ptr<Observable<TreeInfo<int>>>& port,
                                   RedBlackTree<T>* tree);
            static void RotateRight(NodePtr b,
                                    const std::shared_ptr<Observable<TreeInfo<int>>>& port,
                                    RedBlackTree<T>* tree);
            static void Unlink(NodePtr node);
            static void Values(NodePtr node, std::vector<T>& values);
            static NodePtr Next(NodePtr node);
#ifdef INVARIANTS_CHECK
            static void CheckInvariants(NodePtr node, std::vector<T>& values,
                                        std::vector<int32_t>& depths, int32_t black_depth);
#endif
            NodePtr parent;
            NodePtr left;
            NodePtr right;
            T value;
            enum Color color;

            friend RedBlackTree<T>;
        };
        using NodePtr = std::shared_ptr<Node>;

        NodePtr FirstNode();
        NodePtr Root();

    private:
        NodePtr SearchValue(const T& value, const std::shared_ptr<Observable<TreeInfo<int>>> port,
                            TreeInfo<int>& tree_info);
        void UpdateRoot();

        NodePtr root_;
        size_t size_;
        NodePtr GetNearestLeaf(NodePtr node);
    };

    template<typename T>
    class TreeInfo {
    public:
        using NodePtr = std::shared_ptr<typename RedBlackTree<T>::Node>;
        NodePtr root;
        std::map<NodePtr, Status> node_to_status;
    };

    template<typename T>
    TreeInfo<T> GetTreeInfo(RedBlackTree<T>& red_black_tree) {
        using NodePtr = std::shared_ptr<typename RedBlackTree<T>::Node>;

        TreeInfo<T> tree_info;
        for (NodePtr node = red_black_tree.FirstNode(); node != nullptr;
             node = RedBlackTree<T>::Node::Next(node)) {
            tree_info.node_to_status[node] = DEFAULT;
        }
        tree_info.root = red_black_tree.Root();
        return tree_info;
    }

    template<typename T>
    RedBlackTree<T>::RedBlackTree() {
        root_ = nullptr;
        size_ = 0;
    }

    namespace {
        template<typename T>
        void Send(const std::shared_ptr<Observable<TreeInfo<T>>>& port, const TreeInfo<T>& data) {
            if (!port) {
                return;
            }
            port->Notify(data);
        }
    }// namespace

    template<typename T>
    bool RedBlackTree<T>::Insert(const T& value, std::shared_ptr<Observable<TreeInfo<int>>> port) {
        auto data = GetTreeInfo(*this);
        if (!root_) {
            root_ = std::make_shared<Node>(value);
            ++size_;
            {
                data.node_to_status[root_] = Status::CURRENT;
                data.root = Root();
                Send(port, data);
                data.node_to_status[root_] = Status::TOUCHED;
                Send(port, data);
            }
            return true;
        }
        NodePtr node = root_;
        data.node_to_status[root_] = Status::CURRENT;
        Send(port, data);
        NodePtr parent = nullptr;
        while (node) {
            parent = node;
            if (value < node->value) {
                data.node_to_status[node] = Status::TOUCHED;
                node = node->left;
                data.node_to_status[node] = Status::CURRENT;
            } else if (value == node->value) {
                Send(port, data);
                return false;
            } else {
                data.node_to_status[node] = Status::TOUCHED;
                node = node->right;
                data.node_to_status[node] = Status::CURRENT;
            }
            Send(port, data);
        }

        ++size_;
        node = std::make_shared<Node>(value, parent, Color::RED);
        data.node_to_status[node] = Status::CURRENT;
        (value < parent->value ? parent->left : parent->right) = node;
        Send(port, data);
        while (true) {
            if (!parent || parent->color == Color::BLACK) {
                if (!parent) {
                    node->color = Color::BLACK;
                }
                data.root = Root();
                Send(port, data);
                data.node_to_status[node] = Status::TOUCHED;
                Send(port, data);
                return true;
            }
            if (node->GetUncle() && node->GetUncle()->color == Color::RED) {
                node->GetUncle()->color = Color::BLACK;
                node->parent->color = Color::BLACK;
                node->parent->parent->color = Color::RED;
                data.node_to_status[node] = Status::TOUCHED;
                node = node->parent->parent;
                data.node_to_status[node] = Status::CURRENT;
                Send(port, data);
                parent = node->parent;
            } else {
                break;
            }
        }
        if (node->GetGrandParent()->WhichKid(node->parent) == Kid::LEFT) {
            if (node->parent->WhichKid(node) == Kid::RIGHT) {
                Node::RotateLeft(node, port, this);
                data.node_to_status[node] = Status::TOUCHED;
                node = node->left;
                data.node_to_status[node] = Status::CURRENT;
                Send(port, data);
            }
            Node::RotateRight(node->parent, port, this);
            node->parent->color = Color::BLACK;
            node->parent->right->color = Color::RED;
            if (!node->parent->parent) {
                root_ = node->parent;
                data.root = Root();
            }
            Send(port, data);
            data.node_to_status[node] = Status::TOUCHED;
            Send(port, data);
            return true;
        } else {
            if (node->parent->WhichKid(node) == Kid::LEFT) {
                Node::RotateRight(node, port, this);
                data.node_to_status[node] = Status::TOUCHED;
                node = node->right;
                data.node_to_status[node] = Status::CURRENT;
                Send(port, data);
            }
            Node::RotateLeft(node->parent, port, this);
            node->parent->color = Color::BLACK;
            node->parent->left->color = Color::RED;
            if (!node->parent->parent) {
                root_ = node->parent;
                data.root = Root();
            }
            Send(port, data);
            data.node_to_status[node] = Status::TOUCHED;
            Send(port, data);
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
    std::shared_ptr<typename RedBlackTree<T>::Node>
    RedBlackTree<T>::SearchValue(const T& value,
                                 const std::shared_ptr<Observable<TreeInfo<int>>> port,
                                 TreeInfo<int>& tree_info) {
        NodePtr node = root_;
        tree_info.node_to_status[node] = Status::CURRENT;
        Send(port, tree_info);
        while (node) {
            tree_info.node_to_status[node] = Status::TOUCHED;
            if (value < node->value) {
                node = node->left;
            } else if (value == node->value) {
                break;
            } else {
                node = node->right;
            }
            tree_info.node_to_status[node] = Status::CURRENT;
            Send(port, tree_info);
        }
        return node;
    }

    template<typename T>
    std::shared_ptr<typename RedBlackTree<T>::Node> RedBlackTree<T>::GetNearestLeaf(NodePtr node) {
        NodePtr node_to_delete = node->right;
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
    bool RedBlackTree<T>::Erase(const T& value, std::shared_ptr<Observable<TreeInfo<int>>> port) {
        auto data = GetTreeInfo(*this);
        NodePtr node = SearchValue(value, port, data);
        if (!node) {
            return false;
        }
        data.node_to_status[node] = Status::TO_DELETE;
        Send(port, data);
        --size_;
        if (NodePtr node_to_delete = GetNearestLeaf(node)) {
            data.node_to_status[node_to_delete] = Status::CURRENT;
            Send(port, data);
            data.node_to_status[node_to_delete] = Status::TO_DELETE;
            data.node_to_status[node] = Status::CURRENT;
            Send(port, data);
            node->value = node_to_delete->value;
            data.node_to_status[node] = Status::TOUCHED;
            Send(port, data);
            node = node_to_delete;
        }
        if (!node->parent) {
            root_ = nullptr;
            data.root = root_;
            return true;
        }
        Kid kid = node->parent->WhichKid(node);
        (kid == Kid::LEFT ? node->parent->left : node->parent->right) = node->right;
        if (node->right) {
            node->right->parent = node->parent;
        }
        if (node->color == Color::RED) {
            node->right = node->left = node->parent = nullptr;
            Send(port, data);
            return true;
        }
        NodePtr parent = node->parent;
        {
            NodePtr nr = node->right;
            node->right = node->left = node->parent = nullptr;
            node = nr;
            data.node_to_status[node] = Status::CURRENT;
            Send(port, data);
        }
        while (true) {
            if (!node) {
                kid = parent->WhichKid(node);
                NodePtr sibling = (kid == Kid::LEFT ? parent->right : parent->left);
                if (sibling->color == Color::RED) {
                    parent->color = Color::RED;
                    sibling->color = Color::BLACK;
                    (kid == Kid::LEFT ? Node::RotateLeft(sibling, port, this)
                                      : Node::RotateRight(sibling, port, this));
                    sibling = (kid == Kid::LEFT ? parent->right : parent->left);
                    UpdateRoot();
                    data.root = root_;
                    Send(port, data);
                }
                if (parent->color == Color::BLACK &&
                    (!sibling->left || sibling->left->color == Color::BLACK) &&
                    (!sibling->right || sibling->right->color == Color::BLACK)) {
                    sibling->color = Color::RED;
                    node = parent;
                    parent = node->parent;
                    data.node_to_status[node] = Status::CURRENT;
                    Send(port, data);
                    continue;
                }
                if (parent->color == Color::RED &&
                    (!sibling->left || sibling->left->color == Color::BLACK) &&
                    (!sibling->right || sibling->right->color == Color::BLACK)) {
                    parent->color = Color::BLACK;
                    sibling->color = Color::RED;
                    Send(port, data);
                    return true;
                }
                if ((kid == Kid::LEFT && Node::Color(sibling->left) == Color::RED &&
                     Node::Color(sibling->right) == Color::BLACK) ||
                    (kid == Kid::RIGHT && Node::Color(sibling->left) == Color::BLACK &&
                     Node::Color(sibling->right) == Color::RED)) {
                    if (kid == Kid::LEFT) {
                        Node::RotateRight(sibling->left, port, this);
                        Send(port, data);
                    } else {
                        Node::RotateLeft(sibling->right, port, this);
                        Send(port, data);
                    }
                    sibling->color = Color::RED;
                    sibling->parent->color = Color::BLACK;
                    sibling = sibling->parent;
                    Send(port, data);
                }
                enum Color color = parent->color;
                if (kid == Kid::LEFT) {
                    Node::RotateLeft(sibling, port, this);
                    Send(port, data);
                } else {
                    Node::RotateRight(sibling, port, this);
                    Send(port, data);
                }
                parent->color = Color::BLACK;
                (kid == Kid::LEFT ? sibling->right : sibling->left)->color = Color::BLACK;
                parent->parent->color = color;
                UpdateRoot();
                data.root = root_;
                Send(port, data);
                return true;
            }
            if (!node->parent) {
                Send(port, data);
                return true;
            }
            kid = node->parent->WhichKid(node);
            NodePtr sibling = (kid == Kid::LEFT ? node->parent->right : node->parent->left);
            if (sibling->color == Color::RED) {
                node->parent->color = Color::RED;
                sibling->color = Color::BLACK;
                (kid == Kid::LEFT ? Node::RotateLeft(sibling, port, this)
                                  : Node::RotateRight(sibling, port, this));
                sibling = (kid == Kid::LEFT ? node->parent->right : node->parent->left);
                Send(port, data);
            }
            if (node->parent->color == Color::BLACK &&
                (!sibling->left || sibling->left->color == Color::BLACK) &&
                (!sibling->right || sibling->right->color == Color::BLACK)) {
                sibling->color = Color::RED;
                data.node_to_status[node] = Status::TOUCHED;
                node = node->parent;
                data.node_to_status[node] = Status::CURRENT;
                Send(port, data);
                continue;
            }
            if (node->parent->color == Color::RED &&
                (!sibling->left || sibling->left->color == Color::BLACK) &&
                (!sibling->right || sibling->right->color == Color::BLACK)) {
                node->parent->color = Color::BLACK;
                sibling->color = Color::RED;
                UpdateRoot();
                data.root = root_;
                Send(port, data);
                return true;
            }
            if ((kid == Kid::LEFT && Node::Color(sibling->left) == Color::RED &&
                 Node::Color(sibling->right) == Color::BLACK) ||
                (kid == Kid::RIGHT && Node::Color(sibling->left) == Color::BLACK &&
                 Node::Color(sibling->right) == Color::RED)) {
                if (kid == Kid::LEFT) {
                    Node::RotateRight(sibling->left, port, this);
                    Send(port, data);
                } else {
                    Node::RotateLeft(sibling->right, port, this);
                    Send(port, data);
                }
                sibling->color = Color::RED;
                sibling->parent->color = Color::BLACK;
                sibling = sibling->parent;
                Send(port, data);
            }
            enum Color color = node->parent->color;
            if (kid == Kid::LEFT) {
                Node::RotateLeft(sibling, port, this);
                Send(port, data);
            } else {
                Node::RotateRight(sibling, port, this);
                Send(port, data);
            }
            node->parent->color = Color::BLACK;
            (kid == Kid::LEFT ? sibling->right : sibling->left)->color = Color::BLACK;
            node->parent->parent->color = color;
            UpdateRoot();
            data.root = root_;
            Send(port, data);
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
    std::shared_ptr<typename RedBlackTree<T>::Node> RedBlackTree<T>::Node::Next(NodePtr node) {
        if (node->right) {
            node = node->right;
            while (node->left) {
                node = node->left;
            }
            return node;
        }
        while (node->parent && node->parent->right == node) {
            node = node->parent;
        }
        if (!node->parent) {
            return nullptr;
        }
        node = node->parent;
        return node;
    }


    template<typename T>
    RedBlackTree<T>::Node::Node(const T& value1, NodePtr parent1, enum Color color1) {
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
    Kid RedBlackTree<T>::Node::WhichKid(NodePtr kid) {
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
    void RedBlackTree<T>::Node::RotateLeft(NodePtr d,
                                           const std::shared_ptr<Observable<TreeInfo<int>>>& port,
                                           RedBlackTree<T>* tree) {
        auto tree_info = GetTreeInfo(*tree);
        NodePtr b = d->parent;
        NodePtr c = d->left;
        NodePtr pp = b->parent;
        Kid kid;
        if (pp) {
            kid = pp->WhichKid(b);
        }
        tree_info.node_to_status[b] = tree_info.node_to_status[b->left] =
                tree_info.node_to_status[d] = tree_info.node_to_status[d->left] =
                        tree_info.node_to_status[d->right] = Status::ROTATE;
        Send(port, tree_info);
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
        tree_info.root = GetRoot(d);
        Send(port, tree_info);
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
    void RedBlackTree<T>::Node::RotateRight(NodePtr b,
                                            const std::shared_ptr<Observable<TreeInfo<int>>>& port,
                                            RedBlackTree<T>* tree) {
        auto tree_info = GetTreeInfo(*tree);
        NodePtr d = b->parent;
        NodePtr c = b->right;
        NodePtr pp = d->parent;
        Kid kid;
        if (pp) {
            kid = pp->WhichKid(d);
        }
        tree_info.node_to_status[d] = tree_info.node_to_status[b] =
                tree_info.node_to_status[b->left] = tree_info.node_to_status[b->right] =
                        tree_info.node_to_status[d->right] = Status::ROTATE;
        port->Notify(tree_info);
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
        tree_info.root = GetRoot(b);
        port->Notify(tree_info);
    }

    template<typename T>
    std::shared_ptr<typename RedBlackTree<T>::Node> RedBlackTree<T>::Node::GetRoot(NodePtr node) {
        if (!node) {
            return nullptr;
        }
        while (node->parent) {
            node = node->parent;
        }
        return node;
    }

    template<typename T>
    void RedBlackTree<T>::Node::Unlink(NodePtr node) {
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
    enum Color RedBlackTree<T>::Node::Color(NodePtr node) {
        if (!node) {
            return Color::BLACK;
        }
        return node->color;
    }

    template<typename T>
    void RedBlackTree<T>::Node::Values(NodePtr node, std::vector<T>& values) {
        if (!node) {
            return;
        }
        Values(node->left, values);
        values.push_back(node->value);
        Values(node->right, values);
    }

    template<typename T>
    std::shared_ptr<typename RedBlackTree<T>::Node> RedBlackTree<T>::FirstNode() {
        if (!root_) {
            return nullptr;
        }
        NodePtr node = root_;
        while (node->left) {
            node = node->left;
        }
        return node;
    }

    template<typename T>
    std::shared_ptr<typename RedBlackTree<T>::Node> RedBlackTree<T>::Root() {
        return root_;
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
        CheckInvariants(node->left, values, depths, black_depth);
        values.push_back(node->value);
        CheckInvariants(node->right, values, depths, black_depth);
    }
#endif
}// namespace DSVisualization