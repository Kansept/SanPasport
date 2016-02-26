#include <QApplication>
#include <QDir>
#include "DnView.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DnView w;

    if (a.arguments().count() > 1) {
            QStringList fileList (a.arguments());
            fileList.removeFirst();
            w.OpenFile(fileList.first());
        }

    w.show();
    
    return a.exec();
}
