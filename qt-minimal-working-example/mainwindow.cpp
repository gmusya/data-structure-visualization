#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), value_(0)
{
    ui->setupUi(this);
    ui->graphics_view->setScene(&scene);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_clear_button_clicked()
{
    value_ = 0;
    show_value();
}

void MainWindow::on_increment_button_clicked()
{
    ++value_;
    show_value();
}


void MainWindow::on_doubling_button_clicked()
{
    value_ *= 2;
    show_value();
}

void MainWindow::show_value() {
    ui->graphics_view->scene()->clear();
    std::stringstream ss;
    ss << value_;
    ui->graphics_view->scene()->addText(ss.str().c_str());
}

