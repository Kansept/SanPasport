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
    bool saveAs( QSqlDatabase memdb, QString filename, bool save );
    bool addTask(Task tsk);
    bool addAntenna(Prto adIns);
    bool prtoEdit(Prto dan);

    static void importPKAEMO(const QString f);
    void exportPKAEMO(const QString fileCopy);
    QVector<Prto> prtoFromDb();
    QVector<Task> taskFromDb();
    float koef();

    bool saveAntennasToCsv(const QString fileName);
public slots:
    void prtoAddPPC();

private:
    QString str(double dNum);
    QString quotedStr(const QString str);

signals:
    void sgnlfileNew();
};

#endif // CREATEDB_H
