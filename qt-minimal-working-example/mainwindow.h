#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_clear_button_clicked();
    void on_doubling_button_clicked();
    void on_increment_button_clicked();

private:
    void show_value();
    Ui::MainWindow *ui;
    QGraphicsScene scene;
    int value_;
};
#endif // MAINWINDOW_H
