#include <QApplication>
#include <QTextCodec>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    // 解决 Qt 中文乱码问题
    // QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));
    MainWindow w;
    w.show();

    return a.exec();
}
