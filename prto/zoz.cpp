#include "zoz.h"
#include <QtMath>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

Zoz::Zoz()
{
}

Zoz::~Zoz()
{
}

void Zoz::readZoz(const QString f)
{
    // Читаем границы
    QFile fileOpen(f);
    QStringList strlLast;


    Type = QFileInfo(f).fileName().left(2);

    if (fileOpen.open(QIODevice::ReadOnly)) {
        QTextStream streamLast(&fileOpen);
        QString strlLine;
        int i(0);

        while (!streamLast.atEnd()) {
            strlLine = streamLast.readLine();

            if (i == 0 and Type == "vs")
                Height = strlLine.split(" ", QString::SkipEmptyParts).at(1);
            if (i == 0 and Type == "zo")
                Height = strlLine.split(" ", QString::SkipEmptyParts).at(3);
            if (i == 1) {
                strlLast = strlLine.split(" ", QString::SkipEmptyParts);
                MinX = strlLast.at(0).toFloat();
                MinY = strlLast.at(1).toFloat();
            }
            if (i == 2)
                Step = strlLine.split(" ", QString::SkipEmptyParts).at(1).toFloat() - MinY;
            if (strlLine.split(" ", QString::SkipEmptyParts).size() == 3)
                strlLast = strlLine.split(" ", QString::SkipEmptyParts);

            i++;
        }
        fileOpen.close();
        MaxX = strlLast.at(0).toFloat();
        MaxY = strlLast.at(1).toFloat();
    }

    // Размер матрицы
    int numSize( (fabs(MinY)+fabs(MaxY)+1)/Step );
    if (numSize < int( (fabs(MinX)+fabs(MaxX)+1)/Step) )
        numSize = int( (fabs(MinX)+fabs(MaxX)+1)/Step );

    Values.insert(0, pow(numSize, 2), 0);

    // Запись значений
    if (fileOpen.open(QIODevice::ReadOnly)) {
        strlLast.clear();
        int i(0);
        QTextStream streamIns(&fileOpen);

        while (!streamIns.atEnd()) {
            strlLast = streamIns.readLine().split(" ", QString::SkipEmptyParts);
            if (strlLast.size() == 3 and i != 0)
                Values.replace(
                    int( ((strlLast.at(1).toFloat()+abs(MinY)) / Step) * numSize)
                            + ((strlLast.at(0).toFloat()+abs(MinX))/Step),
                    strlLast.at(2).toFloat()
                );
            i++;
        }
    }
    fileOpen.close();
}
