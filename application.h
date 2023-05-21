#pragma once

#include "controller.h"
#include "main_window.h"
#include "red_black_tree.h"
#include "view.h"

#include <iostream>

#include <QApplication>

namespace DSVisualization {
    class Application {
    public:
        Application();
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;
        ~Application();

    private:
        MainWindow main_window_;
        RedBlackTree<int> model_;
        View view_;
        Controller controller_;
    };
}// namespace DSVisualization