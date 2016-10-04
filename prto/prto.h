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

class PRTOSHARED_EXPORT Antenna
{

public:
    // Переменные
    int Id;
    bool Enabled;
    float Number;
    QString Name;
    double Frequency;
    double Gain;
    QString ETilt;
    QString Comment;
    float BmwHoriz;
    float BmwVert;
    float X;
    float Y;
    float Z;
    float CountTrx;
    float Tilt;
    QString Owner;
    float FeederLoss;
    float FeederLeght;
    float KSVN;
    float LossOther;
    float PowerTrx;
    float PowerTotal;
    float Height;
    float Azimut;
    int Type;
    double RadHorizontal[361];     // Азимут ДН Гориз
    double RadVertical[361];      // Азимут ДН Верт
    double AzimutHorizontal[361];      // Радиус ДН Гориз
    double AzimutVertical[361];       // Радиус ДН Верт
    double RadHorizontalNormalization[361]; // Азимут Нормированная ДН Гориз
    double RadVerticalNormalization[361];  // Азимут Нормированная ДН Верт


    /// ------------------------------------------------------------------------- Функции
    Antenna();  // Инициализация

    /// Загрузка / Сохранение
    Antenna loadedBdn(const QString &f);     // Загрузка и считывание данных с bdn файла
    Antenna loadNsma(const QString f);       // Загрузить ДН из фала NSMA
    Antenna loadMsi(const QString f);        // Загрузить ДН из фала MSI
    bool saveAsMsi(const QString filepath);  // Сохранить как MSI
    bool validate(); // Проверка

    ///  Аналитические
    double beamwidthHoriz();    // Узнать ширину горизонтальной ДН
    double beamwidthVert();     // Узнать ширину вертикальной ДН
    double etilt();             // Узнать Электричечкий угол ДН

    /// Трансформация
    void clear();                                       // Обнуление всех переменных
    void mirorrHorizontal();                                 // Отражение горизонтальной ДН
    void mirorrsVertical();                                 // Отражение вертикальной ДН
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
    QString RadHorizontalToString();      // Считать горизонтальную ДН в строку - Стандартная
    QString RadVerticalToString();       // Считать вертикальную ДН в строку - Стандартная
    QString RadHorizontalToString2();     // Считать горизонтальную ДН в строку для ПКАЭМО
    QString RadVerticalToString2();      // Считать вертикальную ДН в строку для ПКАЭМО
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
    void readZoz(const QString f);
};


#endif // PRTO_H
