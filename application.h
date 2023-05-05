#pragma once

#include <iostream>

#include <QApplication>
#include "view.h"
#include "model.h"
#include "controller.h"

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
