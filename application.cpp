#include "application.h"
#include "utility.h"

namespace DSVisualization {
    Application::Application()
        : model_(), view_(), controller_(model_) {
        PRINT_WHERE_AM_I();
        view_.SubscribeToQuery(controller_.GetObserver());
        model_.SubscribeToData(view_.GetObserver());
    }

    Application::~Application() {
        PRINT_WHERE_AM_I();
    }
}// namespace DSVisualization
