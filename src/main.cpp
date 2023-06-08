#if _MSC_VER >= 1600
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif

#include <QDebug>
#include <QMetaType>
#include <QString>
#include <QApplication>

#include "opencv_headers.h"
#include "contour.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<MV_FRAME_OUT_INFO_EX>("MV_FRAME_OUT_INFO_EX");
    // qRegisterMetaType<QList<QPersistentModelIndex>>("QList<QPersistentModelIndex>");
    MainWindow w;
    w.show();

    return a.exec();
}
