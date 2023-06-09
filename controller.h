#pragma once

#include "observable.h"
#include "observer.h"

#include <iostream>
#include <memory>
#include <string>

namespace DSVisualization {
    template<typename T>
    class RedBlackTree;

    struct TreeQuery;

    class Controller {
        using Model = RedBlackTree<int>;

    public:
        explicit Controller(Model& model);
        Controller() = delete;
        Controller(const Controller&) = delete;
        Controller& operator=(const Controller&) = delete;
        Controller(Controller&&) = delete;
        Controller& operator=(Controller&&) = delete;

        ~Controller();

        [[nodiscard]] Observer<TreeQuery>* GetObserver();

    private:
        void OnNotifyFromView(const TreeQuery& value);

        Observer<TreeQuery> observer_view_controller_;
        Model* model_ptr_;
    };
}// namespace DSVisualization
