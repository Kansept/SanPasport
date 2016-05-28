#ifndef PRTO_H
#define PRTO_H

#include "prto_global.h"

#include <QDebug>
#include <math.h>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVector>
#include <QRegExp>
#include <QFileInfo>

class PRTOSHARED_EXPORT Prto
{

public:
    // Переменные
    int Id;
    bool Enabled;
    float Number;
    QString Name;
    double Freq;
    double Gain;
    QString ETilt;
    QString Comment;
    float BmwHoriz;
    float BmwVert;
    float X;
    float Y;
    float Z;
    float PRD;
    float Tilt;
    QString Owner;
    float FeederLoss;
    float FeederLeght;
    float KSVN;
    float LossOther;
    float PowerPRD;
    float PowerTotal;
    float Height;
    float Azimut;
    int Type;
    double RadHoriz[361];     // Азимут ДН Гориз
    double RadVert[361];      // Азимут ДН Верт
    double AzHoriz[361];      // Радиус ДН Гориз
    double AzVert[361];       // Радиус ДН Верт
    double RadHorizNorm[361]; // Азимут Нормированная ДН Гориз
    double RadVertNorm[361];  // Азимут Нормированная ДН Верт


    /// ------------------------------------------------------------------------- Функции
    Prto();  // Инициализация

    /// Загрузка / Сохранение
    Prto loadedBdn(const QString &f);           // Загрузка и считывание данных с bdn файла
    Prto loadNsma(const QString f);             // Загрузить ДН из фала NSMA
    Prto loadMsi(const QString f);              // Загрузить ДН из фала MSI
    bool saveAsMsi(const QString filepath);     // Сохранить как MSI
    bool validate(); // Проверка

    ///  Аналитические
    double beamwidthHoriz();    // Узнать ширину горизонтальной ДН
    double beamwidthVert();     // Узнать ширину вертикальной ДН
    double etilt();             // Узнать Электричечкий угол ДН

    /// Трансформация
    void clear();                                       // Обнуление всех переменных
    void mirorrHoriz();                                 // Отражение горизонтальной ДН
    void mirorrsVert();                                 // Отражение вертикальной ДН
    void rotate(int angleHorizPat, int angleVertPat);   // Поворот ДН
    void preloadPatternVert();                          // Поджать ДН
    void symmetryTop(bool horizPat, bool vertPat);      // Симметрия верха ДН
    void symmetryBottom(bool horizPat, bool vertPat);   // Симметрия низа ДН

    /// Расчет
    float calcPower();                                                                      // Расчет мощности
    void NormalzationPattern();                                                             // Нормирование ДН
    float radHorizMin();                                                                    // Горизонтальная минимальная ДН
    float radVertMin();                                                                     // Вертикальная минимальная ДН
    float calcPDU(const float Xpos, const float Ypos, const float Zpos, const float fKoef); // Расчет ПДУ

    /// Вспомогательные
    QString RadHorizToString();      // Считать горизонтальную ДН в строку - Стандартная
    QString RadVertToString();       // Считать вертикальную ДН в строку - Стандартная
    QString RadHorizToString2();     // Считать горизонтальную ДН в строку для ПКАЭМО
    QString RadVertToString2();      // Считать вертикальную ДН в строку для ПКАЭМО
};

/// ------------------------ ВРЕМЕННО РАЗМЕЩЯЕМ ЗДЕСЬ
class Task
{
public:

    bool Enabled;
    int Type;
    float Number;
    QString Data;
    QString Path;
    int Id;

    // ----------------------------------- Конструктор ----------------------------------- //
    Task()
    {
        clear();
    }

    void clear()
    {
        Enabled = true;
        Type = 0;
        Number = 0;
        Data.clear();
        Path.clear();
        Id = -1;
    }
    // ----------------------------------- Деструктор ----------------------------------- //
    ~Task()
    {
    }

};


class ZO
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

public slots:
    /* ------- Чтение файла Горизонтального сечения ЗОЗ ------- */
    void readZoz(const QString f)
    {
        // Читаем границы
        QFile fileOpen(f);
        QStringList strlLast;

        Type = QFileInfo(f).fileName().left(2);

        if(fileOpen.open(QIODevice::ReadOnly))
        {
            QTextStream streamLast(&fileOpen);
            QString strlLine;
            int i(0);

            while(!streamLast.atEnd()) {
                strlLine = streamLast.readLine();

                if(i == 0 and Type == "vs")
                    Height = strlLine.split(" ", QString::SkipEmptyParts).at(1);
                if(i == 0 and Type == "zo")
                    Height = strlLine.split(" ", QString::SkipEmptyParts).at(3);
                if(i == 1) {
                    strlLast = strlLine.split(" ", QString::SkipEmptyParts);
                    MinX = strlLast.at(0).toFloat();
                    MinY = strlLast.at(1).toFloat();
                }
                if(i == 2)
                    Step = strlLine.split(" ", QString::SkipEmptyParts).at(1).toFloat() - MinY;
                if(strlLine.split(" ", QString::SkipEmptyParts).size() == 3)
                    strlLast = strlLine.split(" ", QString::SkipEmptyParts);

                i++;
            }
            fileOpen.close();
            MaxX = strlLast.at(0).toFloat();
            MaxY = strlLast.at(1).toFloat();
        }

        // Размер матрицы
        int numSize( (fabs(MinY)+fabs(MaxY)+1)/Step );
        if( numSize < int( (fabs(MinX)+fabs(MaxX)+1)/Step ))
            numSize = int( (fabs(MinX)+fabs(MaxX)+1)/Step );

        Values.insert(0, pow(numSize, 2), 0);

        // Запись значений
        if(fileOpen.open(QIODevice::ReadOnly))
        {
            strlLast.clear();
            int i(0);
            QTextStream streamIns(&fileOpen);

            while(!streamIns.atEnd()) {
                strlLast = streamIns.readLine().split(" ", QString::SkipEmptyParts);
                if(strlLast.size() == 3 and i != 0)
                   Values.replace( int(((strlLast.at(1).toFloat()+abs(MinY))/Step)*numSize) + ((strlLast.at(0).toFloat()+abs(MinX))/Step), strlLast.at(2).toFloat());
                i++;
            }
        }
        fileOpen.close();
    }
};

#endif // PRTO_H
