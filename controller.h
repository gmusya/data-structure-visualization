#pragma once

#include <iostream>
#include <memory>
#include <string>

namespace DSVisualization {

    class Model;

    template<typename Data>
    class Observer;

    class TreeQuery;

    using DataViewController = TreeQuery;
    using ObserverViewControllerPtr = std::shared_ptr<Observer<DataViewController>>;

    class Controller {
    public:
        Controller();
        ~Controller();
        void SetModel(Model* model);
        [[nodiscard]] ObserverViewControllerPtr GetObserver() const;
        void OnNotifyFromView(const DataViewController& value);

    private:
        ObserverViewControllerPtr observer_view_controller;
        Model* model_ptr = nullptr;
    };
}// namespace DSVisualization
