#ifndef CREATEDB_H
#define CREATEDB_H

#include <QSqlDatabase>
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>

#include <prto.h>

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
    void exportToPkaemo(const QString fileCopy);
    QVector<Antenna> prtoFromDb();
    QVector<Task> taskFromDb();
    float koef();
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
