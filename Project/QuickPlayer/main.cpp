#include <QApplication>
#include "mainwindow.h"
int main(int argc,char * argv[])
{
    av_register_all();
    QApplication app(argc,argv);

    Mainwindow w;
    w.resize(800,600);
    w.show();

    return app.exec();
}
