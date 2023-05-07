#pragma once

#include <memory>
#include <optional>
#include <set>
#include <vector>

#include "red_black_tree.h"

namespace DSVisualization {
    template<typename Data>
    class Observable;

    template<typename Data>
    class Observer;

    using DataModelView = TreeInfo<int>;
    using ObservableModelViewPtr = std::shared_ptr<Observable<DataModelView>>;
    using ObserverModelViewPtr = std::shared_ptr<Observer<DataModelView>>;

    class Model {
    public:
        Model();
        ~Model();

        void SubscribeFromView(ObserverModelViewPtr observer_model_view);
        void UnsubscribeFromView(ObserverModelViewPtr observer_model_view);

        void Insert(int value);
        void Erase(int value);

    private:
        RedBlackTree<int> values;
        ObservableModelViewPtr observable_model_view;
    };
}// namespace DSVisualization