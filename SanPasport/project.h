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
    bool saveAs(QSqlDatabase memdb, QString filename, bool save);
    bool addTask(Task tsk);
    bool addAntenna(Antenna antenna);
    bool editAntenna(Antenna antenna);
    static void importPKAEMO(const QString f);
    void exportToPkaemo(const QString filePath);
    QVector<Antenna> getAntennas();
    QVector<Task> getTasks();
    float getKoef();
    bool saveAntennasToCsv(const QString fileName);

public slots:
    void addAntennaPPC();
    QVector<Task> getTasks(int type);

private:
    QString str(double dNum);
    QString quotedStr(const QString str);

signals:
    void sgnlfileNew();
};

#endif // CREATEDB_H
