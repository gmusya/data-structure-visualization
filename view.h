#pragma once

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

        [[nodiscard]] Observer<RedBlackTree<int>::Data>* GetObserver();
        void SubscribeToQuery(Observer<TreeQuery>* observer_view_controller);

    private:
        void OnNotifyFromModel(const RedBlackTree<int>::Data& value);
        void AddWidgetsToLayout();

        void SetEnabledButtons(bool flag);
        void DisableButtons();
        void EnableButtons();

        void OnInsertButtonPushed();
        void OnEraseButtonPushed();
        void OnFindButtonPushed();
        void HandlePushButton(DSVisualization::TreeQueryType query_type, const std::string& text);

        std::unique_ptr<DrawableNode> GetDrawableNode(const TreeInfo<int>& tree_info,
                                                      const RedBlackTree<int>::Node* node, float depth,
                                                      float& counter);

        void DrawTree(const std::unique_ptr<DrawableTree>& tree);
        void DrawNode(const std::unique_ptr<DrawableNode>& node);
        void DrawEdgeBetweenNodes(const std::unique_ptr<DrawableNode>& parent, bool is_child_left);
        void RecursiveDraw(const std::unique_ptr<DrawableNode>& node);

        static constexpr float default_width = 960;
        static constexpr float default_height = 540;
        static constexpr float default_node_diameter = 50;
        static constexpr float horizontal_space_between_nodes = 5;
        static constexpr float vertical_space_between_nodes = 3;
        static constexpr float margin = 40;
        static constexpr int draw_delay_in_ms = 500;
        float tree_width_ = 0;
        float current_width_ = default_width;
        float current_node_diameter_ = default_node_diameter;
        int trees_to_show_counter_ = 0;
        TreeQuery query_;
        std::unique_ptr<QGridLayout> main_layout_;
        std::unique_ptr<QPushButton> insert_button_;
        std::unique_ptr<QPushButton> erase_button_;
        std::unique_ptr<QPushButton> find_button_;
        std::unique_ptr<QLineEdit> insert_line_edit_;
        std::unique_ptr<QLineEdit> erase_line_edit_;
        std::unique_ptr<QLineEdit> find_line_edit_;
        std::unique_ptr<QGraphicsScene> tree_scene_;
        std::unique_ptr<QGraphicsView> tree_view_;
        std::unique_ptr<QGraphicsScene> main_scene_;
        Observer<RedBlackTree<int>::Data> observer_model_view_;
        Observable<TreeQuery> observable_view_controller_;
    };
}// namespace DSVisualization