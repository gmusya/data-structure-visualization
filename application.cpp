#include "application.h"
#include "utility.h"
#include <QLayout>
#include <QtWidgets>

Application::Application() {
    PRINT_WHERE_AM_I();
    view_ptr = new DSVisualization::View();
    model_ptr = new DSVisualization::Model();
    controller_ptr = new DSVisualization::Controller();

    view_ptr->SubscribeFromController(controller_ptr->GetObserver());
    model_ptr->SubscribeFromView(view_ptr->GetObserver());
    controller_ptr->SetModel(model_ptr);
}

int Application::Run() {
    PRINT_WHERE_AM_I();
    int val = QApplication::exec();

    return val;
}

Application::~Application() {
    PRINT_WHERE_AM_I();
    model_ptr->UnsubscribeFromView(view_ptr->GetObserver());
    view_ptr->UnsubscribeFromController(controller_ptr->GetObserver());
    delete view_ptr;
    delete model_ptr;
    delete controller_ptr;
}
