#include "controller.h"
#include "model.h"
#include "observer.h"
#include "queries.h"
#include "utility.h"

namespace DSVisualization {
    Controller::Controller() {
        PRINT_WHERE_AM_I();
        observer_view_controller =
                std::make_shared<Observer<DataViewController>>([this](const DataViewController& x) {
                    OnNotifyFromView(x);
                });
    }

    Controller::~Controller() {
        PRINT_WHERE_AM_I();
    }

    void Controller::SetModel(Model* model_ptr_) {
        PRINT_WHERE_AM_I();
        model_ptr = model_ptr_;
    }

    ObserverViewControllerPtr Controller::GetObserver() const {
        PRINT_WHERE_AM_I();
        return observer_view_controller;
    }

    void Controller::OnNotifyFromView(const DataViewController& query) {
        PRINT_WHERE_AM_I();
        if (query.query_type == TreeQueryType::INSERT) {
            model_ptr->Insert(query.value);
        } else if (query.query_type == TreeQueryType::ERASE) {
            model_ptr->Erase(query.value);
        } else if (query.query_type == TreeQueryType::FIND) {
            model_ptr->Find(query.value);
        }
    }
}// namespace DSVisualization