#ifndef CREATEDB_H
#define CREATEDB_H

#include "prto.h"
#include "task.h"

#include <QSqlDatabase>
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>

class Project
{
public:
    Project();

    static bool init();
    static bool saveAs(QString filename, bool save);
    static bool addTask(Task tsk);
    static bool addAntenna(Antenna antenna);
    static bool editAntenna(Antenna antenna);
    static void importFromPkaemo(const QString f);
    static void exportToPkaemo(const QString filePath);
    static QVector<Antenna> getAntennas();
    static QVector<Task> getTasks();
    static float getKoef();
    static bool saveAntennasToCsv(const QString fileName);
    static QSqlDatabase getDatabase();

public slots:
    static void addAntennaPPC();
    static QVector<Task> getTasks(int type);

private:
    static QString str(double dNum);
    static QString quotedStr(const QString str);

signals:
    void sgnlfileNew();
};

#endif // CREATEDB_H
