#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGridLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>

namespace DSVisualization {
    class View;

    class MainWindow : public QMainWindow {
    public:
        friend View;
        MainWindow();

    private:
        void AddWidgetsToLayout();

        static constexpr float default_width = 960;
        static constexpr float default_height = 540;
        float current_width_ = default_width;

        std::unique_ptr<QGridLayout> main_layout_;
        std::unique_ptr<QPushButton> insert_button_;
        std::unique_ptr<QPushButton> erase_button_;
        std::unique_ptr<QPushButton> find_button_;
        std::unique_ptr<QLineEdit> insert_line_edit_;
        std::unique_ptr<QLineEdit> erase_line_edit_;
        std::unique_ptr<QLineEdit> find_line_edit_;
        std::unique_ptr<QGraphicsScene> tree_scene_;
        std::unique_ptr<QGraphicsView> tree_view_;
        std::unique_ptr<QGraphicsScene> main_scene_;
        std::unique_ptr<QGraphicsView> main_view_;
    };
}// namespace DSVisualization