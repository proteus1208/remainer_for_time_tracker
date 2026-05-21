#include "remainerapp.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false); // Keep running when windows are closed

    RemainerApp app;
    app.initialize();

    return a.exec();
}
