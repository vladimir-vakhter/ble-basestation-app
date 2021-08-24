#include "mainwindow.h"
#include <QApplication>                     // manages the GUI app's control flow and main settings

int main(int argc, char *argv[]) {          // argc - number of command-line arguments, argv - the array of command-line arguments
    QApplication a(argc, argv);             // a is this program's QApplication
    MainWindow w;                           // this is the main widget for the application
    w.show();                               // make is visible
    return a.exec();                        // main() passes control to Qt, and exec() will return when the application exits
}
