#include "mainwindow.h"
#include <QApplication>                     // manages the GUI app's control flow and main settings

int main(int argc, char *argv[]) {          // argc - number of command-line arguments, argv - the array of command-line arguments
    QApplication a(argc, argv);             // a is this program's QApplication
//    a.setWindowIcon(QIcon("./icon.png"));
    MainWindow w;                           // this is the main widget for the application
//    QIcon icon("./icon.png");
//    w.setWindowIcon(icon);
    w.show();                               // make is visible
    return a.exec();                        // main() passes control to Qt, and exec() will return when the application exits
}
