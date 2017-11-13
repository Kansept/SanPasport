#include "SanPasport.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    if (a.arguments().count() > 1) {
            QStringList fileList (a.arguments());
            fileList.removeFirst();
            w.fileOpen(fileList.first());
    }

    w.show();
    
    return a.exec();
}
