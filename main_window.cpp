#include "main_window.h"
#include "utility.h"

#include <QtWidgets>

namespace DSVisualization {
    MainWindow::MainWindow()
        : QMainWindow(), main_layout_(new QGridLayout(this)), insert_button_(new QPushButton("Insert", this)),
          erase_button_(new QPushButton("Erase", this)), find_button_(new QPushButton("Find", this)),
          insert_line_edit_(new QLineEdit(this)), erase_line_edit_(new QLineEdit(this)),
          find_line_edit_(new QLineEdit(this)), tree_scene_(new QGraphicsScene(this)),
          tree_view_(new QGraphicsView(tree_scene_, this)), main_scene_(new QGraphicsScene(this)),
          main_view_(new QGraphicsView(main_scene_)) {
        PRINT_WHERE_AM_I();
        setMinimumSize(default_width, default_height);
        AddWidgetsToLayout();
        main_view_.setLayout(main_layout_);
        setCentralWidget(&main_view_);
        show();
    }

    void MainWindow::SetEnabledButtons(bool flag) {
        PRINT_WHERE_AM_I();
        insert_button_->setEnabled(flag);
        erase_button_->setEnabled(flag);
        find_button_->setEnabled(flag);
        insert_line_edit_->setEnabled(flag);
        erase_line_edit_->setEnabled(flag);
        find_line_edit_->setEnabled(flag);
    }

    void MainWindow::DisableButtons() {
        PRINT_WHERE_AM_I();
        SetEnabledButtons(false);
    }

    void MainWindow::EnableButtons() {
        PRINT_WHERE_AM_I();
        SetEnabledButtons(true);
    }

    void MainWindow::AddWidgetsToLayout() {
        PRINT_WHERE_AM_I();
        main_layout_->addWidget(tree_view_, 0, 0, -1, -1);
        main_layout_->addWidget(insert_line_edit_, 1, 0);
        main_layout_->addWidget(erase_line_edit_, 1, 1);
        main_layout_->addWidget(find_line_edit_, 1, 2);
        main_layout_->addWidget(insert_button_, 2, 0);
        main_layout_->addWidget(erase_button_, 2, 1);
        main_layout_->addWidget(find_button_, 2, 2);
    }
}// namespace DSVisualization
