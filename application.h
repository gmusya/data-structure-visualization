#pragma once

#include "controller.h"
#include "model.h"
#include "view.h"

#include <iostream>

#include <QApplication>

class Application {
public:
    explicit Application();
    ~Application();
    static int Run();

private:
    DSVisualization::Model* model_ptr;
    DSVisualization::View* view_ptr;
    DSVisualization::Controller* controller_ptr;
};
