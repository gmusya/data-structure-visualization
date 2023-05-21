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
        ~MainWindow();

    private:
        void AddWidgetsToLayout();

        static constexpr float default_width = 960;
        static constexpr float default_height = 540;
        float current_width_ = default_width;

        QGridLayout* main_layout_;
        QPushButton* insert_button_;
        QPushButton* erase_button_;
        QPushButton* find_button_;
        QLineEdit* insert_line_edit_;
        QLineEdit* erase_line_edit_;
        QLineEdit* find_line_edit_;
        QGraphicsScene* tree_scene_;
        QGraphicsView* tree_view_;
        QGraphicsScene* main_scene_;
        QGraphicsView main_view_;
    };
}// namespace DSVisualization
