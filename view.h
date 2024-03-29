#pragma once

#include "main_window.h"
#include "queries.h"
#include "red_black_tree.h"

#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <set>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtWidgets>

namespace DSVisualization {
    struct DrawableNode {
        float x;
        float y;
        int key;
        QColor outside_color;
        QColor inside_color;
        std::unique_ptr<DrawableNode> left;
        std::unique_ptr<DrawableNode> right;
    };

    struct DrawableTree {
        std::unique_ptr<DrawableNode> root;
    };

    class View : public QGraphicsView {
    public:
        View();
        View(const View&) = delete;
        View& operator=(const View&) = delete;
        View(View&&) = delete;
        View& operator=(View&&) = delete;

        [[nodiscard]] Observer<RedBlackTree<int>::Data>* GetObserver();
        void SubscribeToQuery(Observer<TreeQuery>* observer_view_controller);

    private:
        void OnNotifyFromModel(const RedBlackTree<int>::Data& value);

        void OnInsertButtonPushed();
        void OnEraseButtonPushed();
        void OnFindButtonPushed();
        void HandlePushButton(DSVisualization::TreeQueryType query_type, const std::string& text);

        std::unique_ptr<DrawableNode> GetDrawableNode(const TreeInfo<int>& tree_info,
                                                      const RedBlackTree<int>::Node* node,
                                                      float depth, float& counter);

        void DrawTree(const std::unique_ptr<DrawableTree>& tree);
        void DrawNode(const std::unique_ptr<DrawableNode>& node);
        void DrawEdgeBetweenNodes(const std::unique_ptr<DrawableNode>& parent, bool is_child_left);
        void RecursiveDraw(const std::unique_ptr<DrawableNode>& node);


        static constexpr float default_node_diameter = 50;
        static constexpr float horizontal_space_between_nodes = 5;
        static constexpr float vertical_space_between_nodes = 3;
        static constexpr int draw_delay_in_ms = 500;
        float tree_width_ = 0;
        float current_node_diameter_ = default_node_diameter;
        TreeQuery query_;
        MainWindow main_window_;
        Observer<RedBlackTree<int>::Data> observer_model_view_;
        Observable<TreeQuery> observable_view_controller_;
    };
}// namespace DSVisualization
