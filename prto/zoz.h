#ifndef ZOZ_H
#define ZOZ_H

#include <QVector>
#include <QString>

class Zoz
{
public:
    QString Height;
    QString Type;
    float MinX;
    float MaxX;
    float MinY;
    float MaxY;
    float Step;
    QVector<double> Values;

    Zoz();
    ~Zoz();
    void readZoz(const QString f);
};

#endif // ZOZ_H
