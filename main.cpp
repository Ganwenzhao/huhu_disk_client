#include <QApplication>
#include "mywidget/login.h"
#include "common/common.h"
#include "mywidget/mainwindow.h"

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);

    Login w;
    MainWindow m;
    m.show();
    w.show();

    return app.exec();
}
