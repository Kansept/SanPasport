#ifndef THREADCALCZOZ_H
#define THREADCALCZOZ_H

#include "prto.h"
#include "task.h"
#include <QDate>
#include <QDir>
#include <QTime>
#include <QThread>

class ThreadCalcZoz : public QThread
{
    Q_OBJECT

    QVector<Task> tasks;
    QVector<Antenna> antennas;
    bool stop;

public:

protected:
    void run();

public:
    void loadTask(QVector<Task> vecTasks, QVector<Antenna> vecAntennas);

public slots:
    void stopCalc();

signals:
    void startCalc(int value);
    void progressChanged(int progress);
    void done(int value);
    void doneResult(QString strResult, int idTask);
};


#endif // THREADCALCZOZ_H
