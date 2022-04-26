#include "mainwindow.h"

#include <QApplication>
#include <QtSingleApplication>

int main(int argc, char *argv[])
{
    // QApplication a(argc, argv);
    // MainWindow w;
    // w.show();
    // return a.exec();

    QtSingleApplication app(argc, argv);
    if (app.isRunning()) {
        app.sendMessage("raise_window_noop");
        return EXIT_SUCCESS;
    }
    MainWindow w;
    app.setActivationWindow(&w);
    w.show();
    return app.exec();
}
