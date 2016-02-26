#ifndef CREATEDB_H
#define CREATEDB_H

#include <QSqlDatabase>
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>

#include <prto.h>

class createDb
{

public:
    createDb();

    bool dbNew();
    bool dbSaveAs( QSqlDatabase memdb, QString filename, bool save );
    bool taskAdd(task tsk);
    bool prtoAdd(Prto adIns);
    bool prtoEdit(Prto dan);

    void importPKAEMO(const QString f);
    void exportPKAEMO(const QString fileCopy);
    QVector<Prto> prtoFromDb();
    QVector<task> taskFromDb();
    float koef();

public slots:
    void prtoAddPPC();

private:
    QString str(double dNum);
    QString quotedStr(const QString str);

signals:
    void sgnlfileNew();
};

#endif // CREATEDB_H
