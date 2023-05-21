#include "application.h"
#include "utility.h"

namespace DSVisualization {
    Application::Application()
        : main_window_(), model_(), view_(&main_window_), controller_(model_) {
        PRINT_WHERE_AM_I();
        // main_window_.show();
        view_.SubscribeToQuery(controller_.GetObserver());
        model_.SubscribeToData(view_.GetObserver());
    }

    Application::~Application() {
        PRINT_WHERE_AM_I();
    }
}// namespace DSVisualization
