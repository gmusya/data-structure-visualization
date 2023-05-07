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
    template<typename Data>
    class Observer;

    template<typename Data>
    class Observable;

    using DataModelView = TreeInfo<int>;
    using ObservableModelViewPtr = std::shared_ptr<Observable<DataModelView>>;
    using ObserverModelViewPtr = std::shared_ptr<Observer<DataModelView>>;

    using DataViewController = TreeQuery;
    using ObservableViewControllerPtr = std::shared_ptr<Observable<DataViewController>>;
    using ObserverViewControllerPtr = std::shared_ptr<Observer<DataViewController>>;

    struct DrawableNode {
        float x;
        float y;
        int key;
        Color color;
        Status status;
        std::shared_ptr<DrawableNode> left;
        std::shared_ptr<DrawableNode> right;
    };

    class View : public QGraphicsView {
    public:
        View();

        [[nodiscard]] ObserverModelViewPtr GetObserver() const;
        void OnNotifyFromModel(const DataModelView& value);
        void SubscribeFromController(ObserverViewControllerPtr observer_view_controller);
        void UnsubscribeFromController(ObserverViewControllerPtr observer_view_controller);

    private:
        void AddWidgetsToLayout();

        void SetVisibleButtons(bool flag);
        void HideButtons();
        void ShowButtons();

        void OnInsertButtonPushed();
        void OnEraseButtonPushed();
        void OnFindButtonPushed();
        void HandlePushButton(DSVisualization::TreeQueryType query_type, const std::string& text);

        std::shared_ptr<DrawableNode>
        GetDrawableNode(const TreeInfo<int>& tree_info,
                        const std::shared_ptr<RedBlackTree<int>::Node>& node, int depth,
                        int& counter);

        void DrawTree(const std::shared_ptr<DrawableNode>& root);
        void DrawNode(const std::shared_ptr<DrawableNode>& node);
        void DrawEdgeBetweenNodes(const std::shared_ptr<DrawableNode>& parent, bool is_child_left);
        void RecursiveDraw(const std::shared_ptr<DrawableNode>& node);

        const float default_radius = 50;
        float width = 5;
        float radius = 50;
        float height = 3;
        QPushButton* button1;
        QPushButton* button2;
        QPushButton* button3;
        QGridLayout* mainLayout;
        QLineEdit* addressText1;
        QLineEdit* addressText2;
        QLineEdit* addressText3;
        QGraphicsView* tree_view;
        QGraphicsScene* scene;
        float max_x = 0;
        int cnt = 0;
        ObserverModelViewPtr observer_model_view;
        ObservableViewControllerPtr observable_view_controller;
    };
}// namespace DSVisualization