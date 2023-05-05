#pragma once

#include "queries.h"
#include "red_black_tree.h"
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtWidgets>
#include <set>

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
        int x;
        int y;
        int key;
        Color color;
        std::shared_ptr<DrawableNode> left;
        std::shared_ptr<DrawableNode> right;
    };

    class View : public QGraphicsView {
    public:
        View();
        void DoStuff();

        void OnInsertButtonPushed();
        void OnEraseButtonPushed();
        void OnFindButtonPushed();

        [[nodiscard]] ObserverModelViewPtr GetObserver() const;
        void OnNotifyFromModel(const DataModelView& value);
        void SubscribeFromController(ObserverViewControllerPtr observer_view_controller);
        void UnsubscribeFromController(ObserverViewControllerPtr observer_view_controller);

        void Draw(std::shared_ptr<DrawableNode> node);
        void RecursiveDraw(std::shared_ptr<DrawableNode> node);
    private:
        QGridLayout* mainLayout;
        QLineEdit* addressText1;
        QLineEdit* addressText2;
        QLineEdit* addressText3;
        QLabel* label;
        QPainter* painter;
        QGraphicsView* tree_view;
        QGraphicsScene scene;
        ObserverModelViewPtr observer_model_view;
        ObservableViewControllerPtr observable_view_controller;

        void HandlePushButton(DSVisualization::TreeQueryType query_type, const std::string& text);
    };
}// namespace DSVisualization