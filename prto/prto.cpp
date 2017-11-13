#include "prto.h"

#include <QtMath>

Antenna::Antenna()
{
    clear();
}


/* -------- Обнуление всех переменных -------- */
void Antenna::clear()
{  
    Id = -1;
    Enabled = true;
    Name = "";
    Frequency = 0;
    Gain = 0;
    ETilt = "";
    Comment = "";
    BmwHoriz = 0;
    BmwVert = 0;
    X = 0;
    Y = 0;
    Z = 0;
    CountTrx = 1;
    Tilt = 0;
    Owner = "";
    FeederLoss = 0;
    FeederLeght = 0;
    KSVN = 1;
    LossOther = 0;
    PowerTrx = 0;
    PowerTotal = 0;
    Height = 0;
    Azimut = 0;
    Type = 0;

    for (int i=0; i<=360; i++) {
        AzimutHorizontal[i] = 0;
        AzimutVertical[i] = 0;
        RadHorizontal[i] = 0;
        RadVertical[i] = 0;
        RadHorizontalNormalization[i] = 0;
        RadVerticalNormalization[i] = 0;
    }
}

/// -------------------------------------------- ЗАГРУЗКА / СОХРАНЕНИЕ -------------------------------------------- ///

/* -------- Загрузить ДН из фала MSI -------- */
Antenna Antenna::loadMsi(const QString f)
{
    clear();
    Antenna danLoad;

    int j=1;
    int trig=0;
    int k=0;

    QFile open_file(f);
    if (open_file.open(QIODevice::ReadOnly)) {

        QString strFile(open_file.readAll());
        strFile.replace(QRegExp("\\t"), " ");

        QStringList strlstString;
        QStringList splstr;
        strlstString = strFile.split(QRegExp("\\n"), QString::SkipEmptyParts);

        for (int i=0; i < strlstString.size(); i++)
        {
            splstr = strlstString.at(i).split(" ", QString::SkipEmptyParts);
            if (splstr.at(0) == "NAME")
            {
                for (int iat=1; iat <= (splstr.size()-1); iat++ )
                {
                    Name.append(splstr.at(iat).simplified());
                    if (iat != (splstr.size()-1))
                    {
                        Name.append(" ");
                    }
                }
                Name = Name.simplified();
            }
            else if (splstr.at(0) == "FREQUENCY")
            {
                Frequency = splstr.at(1).toDouble();
            }
            else if (splstr.at(0) == "GAIN")
            {
                if (splstr.size() == 3 && splstr.at(2).simplified() == "dBd")
                    Gain = splstr.at(1).toFloat() + 2.14;
                else
                    Gain = splstr.at(1).toFloat();
            }
            else if (splstr.at(0) == "TILT")
            {
                for (int i=1; i<splstr.size(); i++)
                {
                    ETilt = splstr.at(i).simplified() + " ";
                }
                ETilt = ETilt.simplified();
            }
            else if (splstr.at(0) == "COMMENT")
            {
                for(int i=1; i<splstr.size(); i++)
                {
                    Comment = Comment + splstr.at(i).simplified() + " ";
                }
                Comment = Comment.simplified();
            }
            if (trig == 1)
            {
                if (k < 360) {
                    AzimutHorizontal[k] = splstr.at(0).toDouble();
                    RadHorizontal[k] = splstr.at(1).toDouble();
                    k++;
                }
            }
            if (trig == 2)
            {
                if (k < 360) {
                    AzimutVertical[k] = splstr.at(0).toDouble();
                    RadVertical[k] = splstr.at(1).toDouble();
                    k++;
                }
            }
            if (splstr.at(0) == "HORIZONTAL") {trig=1; k=0;}
            if (splstr.at(0) == "VERTICAL") {trig=2; k=0;}
            j++;

        } // << for(int i=0; i < strlstString.size(); i++)

        AzimutHorizontal[360]=360;
        AzimutVertical[360]=360;
        RadVertical[360]=RadVertical[0];
        RadHorizontal[360]=RadHorizontal[0];

        for (k=0; k<361; k++)
        {
            RadHorizontal[k] = -RadHorizontal[k];
            RadVertical[k] = -RadVertical[k];
        }

    } // << ------ if(open_file.open(QIODevice::ReadOnly))

    open_file.close();
    Type = 100;

    return danLoad;
}


/* -------- Загрузить ДН из фала NSMA -------- */
Antenna Antenna::loadNsma(const QString f)
{
    clear();
    Antenna danLoad;
    QFile open_file(f);
    int intAz;
    int intTrig;
    intTrig = 0; // 1 - ждём горизонтальную ДН; 2 - ждём вертикальную ДН;

    if(open_file.open(QIODevice::ReadOnly))
    {
       QString strFile;
       QStringList strlstString;
       strFile = open_file.readAll();

       strlstString = strFile.split(QRegExp("\\n"), QString::SkipEmptyParts);

       for (int i=0; i < strlstString.size(); i++)
       {
           if ( (strlstString.at(i).contains("ENDFIL", Qt::CaseInsensitive) ||
                strlstString.at(i).contains("PATCUT", Qt::CaseInsensitive) )
                && (intTrig == 10 || intTrig == 20) )
           {
              intTrig = 99;
           }
           // Название антенны
           if ( strlstString.at(i).contains("ANTMAN", Qt::CaseInsensitive) )
           {
               Name = strlstString.at(i).split(",", QString::SkipEmptyParts).at(1);
           }
           if ( strlstString.at(i).contains("MODNUM", Qt::CaseInsensitive) )
           {
               Name += " " + strlstString.at(i).split(",", QString::SkipEmptyParts).at(1);
           }
           // Коэфициент усиления
           if ( strlstString.at(i).contains("MDGAIN", Qt::CaseInsensitive) )
           {
               Gain = strlstString.at(i).split(",", QString::SkipEmptyParts).at(1).toFloat();
           }
           // Частота
           if ( strlstString.at(i).contains("PATFRE", Qt::CaseInsensitive) )
           {
               Frequency = strlstString.at(i).split(",", QString::SkipEmptyParts).at(1).toFloat();
           }
           // Угол
           if ( strlstString.at(i).contains("ELTILT", Qt::CaseInsensitive) )
           {
               ETilt = strlstString.at(i).split(",", QString::SkipEmptyParts).at(1);
           }
           // Коментарий
           if (strlstString.at(i).contains("DESCR1", Qt::CaseInsensitive)) { Comment = strlstString.at(i).split(",", QString::SkipEmptyParts).at(1); }
           if (strlstString.at(i).contains("DESCR2", Qt::CaseInsensitive)) { Comment += " " + strlstString.at(i).split(",", QString::SkipEmptyParts).at(1); }
           if (strlstString.at(i).contains("DESCR3", Qt::CaseInsensitive)) { Comment += " " + strlstString.at(i).split(",", QString::SkipEmptyParts).at(1); }
           if (strlstString.at(i).contains("DESCR4", Qt::CaseInsensitive)) { Comment += " " + strlstString.at(i).split(",", QString::SkipEmptyParts).at(1); }
           if (strlstString.at(i).contains("DESCR5", Qt::CaseInsensitive)) { Comment += " " + strlstString.at(i).split(",", QString::SkipEmptyParts).at(1); }
           // Ждем горизонтальную диаграмму
           if ( strlstString.at(i).contains("PATCUT:,AZ", Qt::CaseInsensitive) )
           {
               intTrig = 1;
           }
           // Ждем верикальную диаграмму
           if ( strlstString.at(i).contains("PATCUT:,EL", Qt::CaseInsensitive) )
           {
               intTrig = 2;
           }
           // Считываем горизонтальную ДН
           if (intTrig == 10)
           {
               intAz = strlstString.at(i).split(",", QString::SkipEmptyParts).at(0).toFloat();
               if(intAz < 0) { intAz = 360 + intAz; }
               RadHorizontal[intAz] = strlstString.at(i).split(",", QString::SkipEmptyParts).at(1).toFloat();
               AzimutHorizontal[intAz] = intAz;
           }
           // Считываем вертикальную ДН
           if (intTrig == 20)
           {
               intAz = strlstString.at(i).split(",", QString::SkipEmptyParts).at(0).toFloat();
               if(intAz < 0) { intAz = 360 + intAz; }
               RadVertical[intAz] = strlstString.at(i).split(",", QString::SkipEmptyParts).at(1).toFloat();
               AzimutVertical[intAz] = intAz;
           }
           if ( strlstString.at(i).contains("FSTLST:", Qt::CaseInsensitive) && intTrig == 1)
           {
               intTrig = 10;
           }
           if ( strlstString.at(i).contains("FSTLST:", Qt::CaseInsensitive) && intTrig == 2)
           {
               intTrig = 20;
           }
       } // << -- for(int i=0; i < strlstString.size(); i++)

       Name = Name.simplified();
       ETilt = ETilt.simplified();
       Comment = Comment.simplified();

       RadVertical[360]=RadVertical[0];
       RadHorizontal[360]=RadHorizontal[0];
    }

    if (etilt() > 100 )
    {
       mirorrsVertical();
       mirorrHorizontal();
    }

    return danLoad;
}


/* -------- Загрузка и считывание данных с bdn файла -------- */
Antenna Antenna::loadedBdn(const QString &f)
{
    Antenna danLoad;

         QString mass[800];
         int j=1;
         int trig = 0;
         QStringList splstr;
         int k=0;

         int k1=0;
         int k2=0;

         // ------ Обнуление данных ------ //
         clear();

        QFile open_file(f);

        if (open_file.open(QIODevice::ReadOnly))  {
             QTextStream stream(&open_file);
             QString linestr;

             while(!stream.atEnd()) {
                     linestr = open_file.readLine();
                     mass[j]=linestr;

                     splstr = mass[j].split(" ");

                      if (j == 1) { Name = mass[j].simplified(); }
                      if (j == 2) { Gain = mass[j].toDouble(); }
                      if (j == 3) { Comment = mass[j].simplified(); }


                      if (5<=j && j<=364) {
                             AzimutHorizontal[k1] = splstr.at(0).toDouble();
                             RadHorizontal[k1] = splstr.at(1).toDouble();
                             k1++;
                      } // << if (5<=j && j<=364)

                      if (366 <= j && j <= 725) {
                              AzimutVertical[k2] = splstr.at(0).toDouble();
                              RadVertical[k2] = splstr.at(1).toDouble();
                              k2++;
                      } // << if (366<=j && j<=725)

                      if (splstr.at(0).simplified() == "360" && trig == 0) {trig = 1; k = 0;}
                      if (splstr.at(0).simplified() == "360" && trig == 1) {trig = 2; k = 0;}
                      j++;
              } // << while(!stream.atEnd()) <<

            AzimutHorizontal[360] = 360;
            AzimutVertical[360] = 360;
            RadVertical[360] = RadVertical[0];
            RadHorizontal[360] = RadHorizontal[0];

             for (k=0; k<361; k++){
                 RadHorizontal[k]=-RadHorizontal[k];
                 RadVertical[k]=-RadVertical[k];
             } // << for (k=0; k<361; k++)

/*             initPolarGraph(dan);
             initDecartGraph(dan);
             setDanGraph(dan);
*/
    } // << if(open_file.open(QIODevice::ReadOnly))

    open_file.close();
    return danLoad;
}


/* -------- Сохранить как MSI -------- */
bool Antenna::saveAsMsi(const QString filepath)
{
    if (filepath.isEmpty())
           return false;
    else {
       QFile fileSaveAs(filepath);
       if (!fileSaveAs.open(QIODevice::WriteOnly)) {
            return false;
       }
       QTextStream out(&fileSaveAs);
       out.setCodec("Windows-1251");
       out << "NAME " << Name << "\n";
       out << "FREQUENCY " << (QString::number(Frequency)) << "\n";
       out << "GAIN " << ( QString::number(Gain) ) << " dBi\n";
       out << "TILT " << ETilt << "\n";
       out << "COMMENT " << Comment << "\n";
       out << "HORIZONTAL 360\n";
       for (int i=0; i<=359; i++) {
           out << (QString::number(AzimutHorizontal[i])) << ".0 " << (QString::number(fabs(RadHorizontal[i]))) << "\n";
       }
       out << "VERTICAL 360\n";
       for (int i=0; i<=359; i++) {
           out << (QString::number(AzimutVertical[i])) << ".0 " << (QString::number(fabs(RadVertical[i]))) << "\n";
       }
       fileSaveAs.close();
    }
    return true;
}


/* -------- Проверка -------- */
bool Antenna::validate()
{
    if(Name == "" || Frequency == 0) {
        return false;
    } else {
        return true;
    }
}
/// -------------------------------------------- АНАЛИТИЧЕСКИЕ -------------------------------------------- ///

/* -------- Узнать ширину горизонтальной ДН -------- */
double Antenna::beamwidthHoriz()
{
      float Beam_H=0;
      for (int i = 0; i<=360; i++) {
         if(-RadHorizontal[i] <= 3) { Beam_H++; }
      }
      if(Beam_H == 361) { Beam_H += -1; }
      return Beam_H;
}


/* -------- Узнать ширину вертикальной ДН -------- */
double Antenna::beamwidthVert()
{
    float Beam_V=0;
    for (int i = 0; i<=360; i++) {
        if (-RadVertical[i] <= 3) { Beam_V++; }
    }
    return Beam_V;
}


/* -------- Узнать Электричечкий угол ДН -------- */
double Antenna::etilt()
{
    int i=0;
    while (!(RadVertical[i] == 0) || i>360) { i=i+1; }
    return AzimutVertical[i];
}


/// -------------------------------------------- ТРАНСФОРМАЦИЯ -------------------------------------------- ///

/* -------- Отражение горизонтальной ДН -------- */
void Antenna::mirorrHorizontal()
{
     double tempMirror_Horiz[360];
     int k = 359;
     for (int i=0; i<=359; i++) {
          tempMirror_Horiz[i] = RadHorizontal[k];
          k = k-1;
     }
     for (int j=0; j<=359; j++) {
          RadHorizontal[j] = tempMirror_Horiz[j];
     }
}


/* -------- Отражение вертикальной ДН -------- */
void Antenna::mirorrsVertical()
{
    double tempMirror_Vert[360];
    int k = 359;
    for (int i=0; i<=359; i++) {
         tempMirror_Vert[i] = RadVertical[k];
         k = k-1;
    }
    for (int jh=0; jh<=359; jh++) {
         RadVertical[jh] = tempMirror_Vert[jh];
    }
}


/* -------- Поворот ДН -------- */
void Antenna::rotate(int angleHorizPat,  int angleVertPat)
{
    double tempRotate_Horiz[360];
    double tempRotate_Vert[360];
    int kH = 0;
    int kV = 0;

    for (int i=0; i<=359; i++) {
         if (angleHorizPat<0) { angleHorizPat=360+angleHorizPat;}
         if (angleVertPat<0)  { angleVertPat=360+angleVertPat;}

         if (angleHorizPat-i>0) {kH=360-angleHorizPat+i;}
         if (angleHorizPat-i<=0) {kH=-(angleHorizPat-i);}
         tempRotate_Horiz[i]=RadHorizontal[kH];

         if (angleVertPat-i>0) {kV=360-angleVertPat+i;}
         if (angleVertPat-i<=0) {kV=-(angleVertPat-i);}
         tempRotate_Vert[i]=RadVertical[kV];
     }
     for (int i=0; i<=359; i++) {
         RadHorizontal[i]=tempRotate_Horiz[i];
         RadVertical[i]=tempRotate_Vert[i];
     }
}


/* -------- Симметрия низа ДН -------- */
void Antenna::symmetryBottom(bool horizPat, bool vertPat)
{
    double dTempRadHoriz[361]; // Азимут ДН
    double dTempRadVert[361]; // Азимут ДН

    for (int i=0; i < 361; i++) {
        dTempRadHoriz[i] = RadHorizontal[i];
        dTempRadVert[i] = RadVertical[i];
    }

    int intETilt = etilt();
    int intPoint;
    int intPoint2;

    if (horizPat == true || vertPat == true) {
        for (int i=0; i < 180; i++) {
         intPoint = (180+intETilt)+i; if(intPoint > 359) { intPoint -= 360; }
         intPoint2 = (180+intETilt)-i;  if(intPoint2 > 359) { intPoint2 -= 360; }
         if (horizPat == true) { RadHorizontal[intPoint] = dTempRadHoriz[intPoint2]; }
         if (vertPat == true) { RadVertical[intPoint] = dTempRadVert[intPoint2]; }
        }
    }
}


/* -------- Симметрия верха ДН -------- */
void Antenna::symmetryTop(bool horizPat, bool vertPat)
{
    double dTempRadHoriz[361]; // Азимут ДН
    double dTempRadVert[361];  // Азимут ДН

    for (int i=0; i < 361; i++) {
        dTempRadHoriz[i] = RadHorizontal[i];
        dTempRadVert[i] = RadVertical[i];
    }

    int intETilt = etilt();
    int intPoint;
    int intPoint2;

    if(horizPat == true || vertPat == true)
        for(int i=0; i < 180; i++)
        {
            intPoint = (180+intETilt)+i; if(intPoint > 359) { intPoint -= 360; }
            intPoint2 = (180+intETilt)-i;  if(intPoint2 > 359) { intPoint2 -= 360; }
            if (horizPat == true) { RadHorizontal[intPoint2] = dTempRadHoriz[intPoint]; }
            if (vertPat == true) { RadVertical[intPoint2] = dTempRadVert[intPoint]; }
        }
}


/* -------- Поджать ДН -------- */
void Antenna::preloadPatternVert()
{
      for (int i = 0; i<=360; i++)
          if (RadVertical[i] < -6)
              RadVertical[i] = (floor(RadVertical[i]*10.1))/10;
}


/// -------------------------------------------- РАСЧЕТ -------------------------------------------- ///

/* -------- Расчет мощности -------- */
float Antenna::calcPower()
{
    return (PowerTrx*(pow(10,(-(FeederLoss)*FeederLeght-LossOther)/10)) * (1-pow((KSVN-1)/(KSVN+1),2))) * CountTrx;
}


/* -------- Нормирование ДН -------- */
void Antenna::NormalzationPattern()
{
    rotate(Azimut, Tilt);

    for (int i = 0; i < 361; i++) {
        RadHorizontalNormalization[i] = pow( 10, (RadHorizontal[i]/10) );
        RadVerticalNormalization[i]  = pow( 10, (RadVertical[i]/10)  );
    }
}


/* -------- Горизонтальная минимальная ДН -------- */
float Antenna::radHorizMin()
{
    float RadMin;
    RadMin = 0;

    for (int i=0; i<=359; i++)
        if (RadHorizontal[i] < RadMin)
            RadMin = RadHorizontal[i];

    return RadMin;
}


/* -------- Вертикальная минимальная ДН -------- */
float Antenna::radVertMin()
{
    float RadMin;
    RadMin = 0;

    for (int i=0; i<=359; i++)
        if (RadVertical[i] < RadMin)
            RadMin = RadVertical[i];

    return RadMin;
}


/* -------- Расчет ПДУ -------- */
float Antenna::calcPDU(const float Xpos, const float Ypos, const float Zpos, const float fKoef)
{
    float E, P, G, R, L, AngleH, AngleV;

    P = calcPower();
    G = Gain;
    L = sqrt( pow((X - Xpos),2) + pow((Y - Ypos), 2) );
    R = sqrt( pow((Z - Zpos),2) + pow(L, 2) );

    // Координата по горизонтали --------------------------
    if (Xpos != 0) {
        AngleH = (atan2(Ypos,Xpos)*180)/3.14;
        AngleH = -(AngleH - 90);
        if (AngleH < 0) {
            AngleH = AngleH + 360;
        }
    } else {
        AngleH = (Ypos >= 0)? 0 : 180;
    }
    // -------------------------- Координата по горизонтали

    // Координата по вертикали --------------------------
    int pntZ(Z - Zpos);
    if (pntZ != 0) {
        AngleV = (atan2(R, pntZ)*180) / 3.14;
        AngleV = -(AngleV - 90);
        if (AngleV < 0) {
            AngleV = AngleV + 360;
        }
    } else {
        AngleV = (R >= 0)? 0 : 180;
    }
    // -------------------------- Координата по вертикали

    E = (sqrt(30 * P * G) / R ) * fKoef * RadHorizontalNormalization[int(AngleH)] * RadVerticalNormalization[int(AngleV)];

    return (pow(E,2) / 3.77);
}


/// -------------------------------------------- ВСПОМОГАТЕЛЬНЫЕ -------------------------------------------- ///
/* --------  Считать горизонтальную ДН в строку - Стандартная -------- */
QString Antenna::RadHorizontalToString()
{
    QString qsRadHoriz;
    qsRadHoriz.clear();

    for (int i=0; i<=360; i++) {
         qsRadHoriz.append(QString::number(RadHorizontal[i])).append(";");
    }
    return qsRadHoriz;
}


/* --------  Считать вертикальную ДН в строку - Стандартная -------- */
QString Antenna::RadVerticalToString()
{
    QString qsRadVert;
    qsRadVert.clear();

    for (int i=0; i<=360; i++) {
         qsRadVert.append(QString::number(RadVertical[i])).append(";");
    }
    return qsRadVert;
}


/* --------  Считать горизонтальную ДН в строку для ПКАЭМО -------- */
QString Antenna::RadHorizontalToString2()
{
    QString qsRadHoriz;
    qsRadHoriz.clear();

    for (int i=0; i<=359; i++) {
         qsRadHoriz.append(QString::number(-RadHorizontal[i])).append(";");
    }

    qsRadHoriz.remove(qsRadHoriz.size()-1,1);
    return qsRadHoriz;
}


/* --------  Считать вертикальную ДН в строку для ПКАЭМО -------- */
QString Antenna::RadVerticalToString2()
{
    QString qsRadVert;
    qsRadVert.clear();

    for (int i=0; i<=359; i++) {
         qsRadVert.append(QString::number(-RadVertical[i])).append(";");
    }

    qsRadVert.remove(qsRadVert.size()-1,1);
    return qsRadVert;
}
