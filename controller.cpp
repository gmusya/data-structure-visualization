#include "controller.h"
#include "observer.h"
#include "queries.h"
#include "red_black_tree.h"
#include "utility.h"

namespace DSVisualization {
    Controller::Controller(Model& model)
        : observer_view_controller_(
                  [this](const TreeQuery& x) {
                      OnNotifyFromView(x);
                  }),
          model_ptr_(&model) {
        PRINT_WHERE_AM_I();
    }

    Controller::~Controller() {
        PRINT_WHERE_AM_I();
    }

    Observer<TreeQuery>* Controller::GetObserver() {
        PRINT_WHERE_AM_I();
        return &observer_view_controller_;
    }

    void Controller::OnNotifyFromView(const TreeQuery& query) {
        PRINT_WHERE_AM_I();
        switch (query.query_type) {
            case TreeQueryType::insert:
                model_ptr_->Insert(query.value);
                break;
            case TreeQueryType::erase:
                model_ptr_->Erase(query.value);
                break;
            case TreeQueryType::find:
                model_ptr_->Find(query.value);
                break;
            default:
                break;
        }
    }
}// namespace DSVisualization
