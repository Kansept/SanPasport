#include "sitplan.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SitPlan w;
    w.show();

    return a.exec();
}
