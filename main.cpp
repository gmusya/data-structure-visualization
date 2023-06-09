#include "application.h"
#include "utility.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication q_app(argc, argv);
    DSVisualization::Application app;
    QApplication::exec();
    PRINT_WHERE_AM_I();
    return 0;
}
