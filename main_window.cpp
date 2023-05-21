#include "main_window.h"

#include <QtWidgets>

namespace DSVisualization {
    MainWindow::MainWindow()
        : QMainWindow(), main_layout_(new QGridLayout()), insert_button_(new QPushButton("Insert")),
          erase_button_(new QPushButton("Erase")), find_button_(new QPushButton("Find")),
          insert_line_edit_(new QLineEdit()), erase_line_edit_(new QLineEdit()),
          find_line_edit_(new QLineEdit()), tree_scene_(new QGraphicsScene()),
          tree_view_(new QGraphicsView(tree_scene_.get())), main_scene_(new QGraphicsScene()),
          main_view_(new QGraphicsView(main_scene_.get())) {
        setMinimumSize(default_width, default_height);
        AddWidgetsToLayout();
        main_view_->setLayout(main_layout_.get());
        setCentralWidget(main_view_.get());
        show();
    }

    void MainWindow::AddWidgetsToLayout() {
        main_layout_->addWidget(tree_view_.get(), 0, 0, -1, -1);
        main_layout_->addWidget(insert_line_edit_.get(), 1, 0);
        main_layout_->addWidget(erase_line_edit_.get(), 1, 1);
        main_layout_->addWidget(find_line_edit_.get(), 1, 2);
        main_layout_->addWidget(insert_button_.get(), 2, 0);
        main_layout_->addWidget(erase_button_.get(), 2, 1);
        main_layout_->addWidget(find_button_.get(), 2, 2);
    }
}// namespace DSVisualization