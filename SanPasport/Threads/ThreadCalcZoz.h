#ifndef THREADCALCZOZ_H
#define THREADCALCZOZ_H

#include <QDate>
#include <QDir>
#include <QTime>
#include <QThread>
#include "prto.h"

class ThreadCalcZoz : public QThread
{
    Q_OBJECT

    QVector<Task> tasks;
    QVector<Antenna> antennas;
    bool stop;

public:
   // explicit ThreadCalcZoz(QObject *parent = 0)  { }

protected:
    void run()
    {
        for (int countTask = 0; countTask < this->tasks.count(); countTask++) {
            Task taskZoz(this->tasks.at(countTask));

            QStringList strlTaskZoz(taskZoz.Params.split(";"));
            Antenna prtoCalc;
            float Xmin(strlTaskZoz.at(0).toFloat());
            float Xmax(strlTaskZoz.at(1).toFloat());
            float Xd(strlTaskZoz.at(2).toFloat());
            float Ymin(strlTaskZoz.at(3).toFloat());
            float Ymax(strlTaskZoz.at(4).toFloat());
            float Yd(strlTaskZoz.at(5).toFloat());
            float fHeight(strlTaskZoz.at(6).toFloat());
            float fPDU(0);
            int count(0);

            float fCountIteration(0);
            fCountIteration = 100 / ((fabs(Xmax) + fabs(Xmin)) / Xd);

            if (!QDir("Result").exists())
                QDir().mkdir("Result");

            QFile fileResult("Result/zo_h" + QString::number(fHeight) + "_" + QDate::currentDate().toString("yyyyMMdd")
                             + "_" + QTime::currentTime().toString("hhmmss") + ".dat");

            QTextStream txtstremResult(&fileResult);
            txtstremResult.setCodec("Windows-1251");

            if (fileResult.open(QIODevice::WriteOnly)) {
                doneResult("calc", taskZoz.Id);
                emit startCalc(-1);
                txtstremResult << QString::fromWCharArray(L"ЗО. h = ") << QString::number(fHeight)
                               << QString::fromWCharArray(L" м\n");

                for (int k=0; k < this->antennas.count(); k++) {
                    prtoCalc = this->antennas.at(k);
                    prtoCalc.NormalzationPattern();
                    this->antennas.replace(k, prtoCalc);
                }

                for ( float x = Xmin; x <= Xmax; x += Xd ) {
                    count++;
                    for ( float y = Ymin; y <= Ymax; y += Yd ) {
                        fPDU = 0;

                        for (int k=0; k < this->antennas.count(); k++) {
                            prtoCalc.clear();
                            prtoCalc = this->antennas.at(k);
                            if (this->stop) {
                                emit done(101);
                                return;
                            }
                            fPDU += pow(prtoCalc.calcPDU(x, y, fHeight, 1.22), 2);
                        }
                        txtstremResult << (QString().sprintf("%.3f", double(x)) + " ")
                                       << (QString().sprintf("%.3f", double(y)) + " ")
                                       << QString().sprintf("%.3f", sqrt(fPDU)) << "\n";
                    }
                    txtstremResult << "\n";
                    progressChanged(int(count * fCountIteration));
                }
                fileResult.close();
            }
            //        else
            //            QMessageBox::warning(this, tr("Ошибка"), tr("Ошибка создания файла результата!"));

            emit done(101);
            doneResult(fileResult.fileName(), taskZoz.Id);
        }
        this->exec();
    }

public:
    void loadTask(QVector<Task> vecTask, QVector<Antenna> vecPrto)
    {
        this->tasks = vecTask;
        this->antennas = vecPrto;
        this->stop = false;
    }

public slots:
    void stopCalc()
    {
        stop = true;
    }

signals:
    void startCalc(int value);
    void progressChanged(int progress);
    void done(int value);
    void doneResult(QString strResult, int idTask);
};


#endif // THREADCALCZOZ_H
