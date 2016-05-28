#include "project.h"

#include <sqlite3.h>
#include <QVariant>
#include <QSqlDriver>
#include <QSqlError>
#include <QFileDialog>

using namespace std;


Project::Project()
{
}

/**
 * Новый проект
 *
 * @param trytrytr
 */
bool Project::init()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","project");
    db.setDatabaseName(":memory:");

    if (!db.database("project").open())
        return false;

    QSqlQuery query(db);

    // Таблица с заданиями
    query.exec("create table TaskCalc (id INTEGER PRIMARY KEY, " // 0
               "Enabled INTEGER, "  // 1
               "TYPE INTEGER, "     // 2
               "TASK TEXT, "        // 3
               "Path TEXT, "        // 4
               "Number REAL)");     // 5

    // Таблица с параметрами
    query.exec("create table Options (Koef REAL, "
               "KPP REAL, "
               "COMMENTS REAL)");

    query.exec("insert into Options values(1.22, 3, 0)");

    // Таблица с Антенами
    query.exec("create table Prto (id INTEGER PRIMARY KEY, "  // 0
               "Enabled TEXT, "         // 1
               "Name TEXT, "            // 2
               "Owner TEXT, "           // 3
               "Freq REAL, "            // 4
               "Gain REAL, "            // 5
               "Height REAL, "          // 6
               "Polarization TEXT, "    // 7
               "PowerTotal REAL, "      // 8
               "PowerPRD REAL, "        // 9
               "PRD INT, "              // 10
               "FeederLeght REAL, "     // 11
               "FeederLoss REAL, "      // 12
               "Ksvn REAL, "            // 13
               "LossOther REAL, "       // 14
               "X REAL, "               // 15
               "Y REAL, "               // 16
               "Z REAL, "               // 17
               "Azimut REAL, "          // 18
               "Tilt REAL, "            // 19
               "Number REAL, "          // 20
               "RadHoriz TEXT, "        // 21
               "RadVert TEXT, "         // 22
               "Pdu INT,"               // 23
               "Type INT"               // 24
               ")");

    // Таблица СитПлан
    query.exec("create table filesZO (id INTEGER PRIMARY KEY, "
               "STATUS TEXT, "
               "HIGHT INTEGER, "
               "PATH TEXT, "
               "DATA TEXT, "
               "SIZE REAL)"
               );

    //  Таблица с параметрами
    query.exec("create table OptionsSitPlan (TITLE TEXT, "
               "ZOPEN REAL, "
               "ZOCOLOR varchar(40), "
               "ZOGRIND int, "
               "albumcount int)");

    query.exec("insert into OptionsSitPlan values('Ситуационный22 план ([S]).<br/> Высота среза [H] м', 4, '', 0, 0)");

    return true;
}


// ------------------------------ Сохранить (SQLite API) ------------------------------ //
bool Project::saveAs( QSqlDatabase memdb, QString filename, bool save )
{
    bool state = false;
    QVariant v = memdb.driver()->handle();
    if (v.isValid() && qstrcmp(v.typeName(),"sqlite3*") == 0)
    {
        // v.data() returns a pointer to the handle
        sqlite3 * handle = *static_cast<sqlite3 **>(v.data());
        if (handle != 0) // check that it is not NULL
        {
            sqlite3 * pInMemory = handle;
            QByteArray array = filename.toUtf8();

            const char * zFilename = array.data();
            int rc;                   /* Function return code */
            sqlite3 *pFile;           /* Database connection opened on zFilename */
            sqlite3_backup *pBackup;  /* Backup object used to copy data */
            sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
            sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

            /* Open the database file identified by zFilename. Exit early if this fails
            ** for any reason. */
            rc = sqlite3_open( zFilename, &pFile );
            if( rc==SQLITE_OK ){

              /* If this is a 'load' operation (isSave==0), then data is copied
              ** from the database file just opened to database pInMemory.
              ** Otherwise, if this is a 'save' operation (isSave==1), then data
              ** is copied from pInMemory to pFile.  Set the variables pFrom and
              ** pTo accordingly. */
              pFrom = ( save ? pInMemory : pFile);
              pTo   = ( save ? pFile     : pInMemory);

              /* Set up the backup procedure to copy from the "main" database of
              ** connection pFile to the main database of connection pInMemory.
              ** If something goes wrong, pBackup will be set to NULL and an error
              ** code and  message left in connection pTo.
              **
              ** If the backup object is successfully created, call backup_step()
              ** to copy data from pFile to pInMemory. Then call backup_finish()
              ** to release resources associated with the pBackup object.  If an
              ** error occurred, then  an error code and message will be left in
              ** connection pTo. If no error occurred, then the error code belonging
              ** to pTo is set to SQLITE_OK.
              */
              pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
              if( pBackup ){
                (void)sqlite3_backup_step(pBackup, -1);
                (void)sqlite3_backup_finish(pBackup);
              }
              rc = sqlite3_errcode(pTo);
            }

            /* Close the database connection opened on database file zFilename
            ** and return the result of this function. */
            (void)sqlite3_close(pFile);

            if( rc == SQLITE_OK ) state = true;
        }
    }
    return state;
}


/* ------------------------------------ Импорт из ПКАЭМО ------------------------------------ */
void Project::importPKAEMO(const QString f)
{
    QVector<Prto> antennas;
    QVector<Task> tasks;

    Prto dan;

    QSqlDatabase dbPKAEMO = QSqlDatabase::addDatabase("QODBC","PKAEMO");
    dbPKAEMO.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DSN='';CharSet='UTF8';DBQ="+f);

    if (dbPKAEMO.database("PKAEMO").open()) {
        QSqlQuery q(dbPKAEMO);
        QStringList strlstRadH;
        QStringList strlstRadV;
        QStringList strlstParam;

        q.exec("SELECT * FROM Sost ORDER BY Nmbr Asc");
        while (q.next()) {
            dan.clear();
            dan.Enabled = q.value(13).toBool();
            dan.Name = q.value(2).toString();
            dan.Owner = q.value(3).toString();
            dan.Freq = q.value(4).toFloat();
            dan.Number = q.value(11).toInt();

            strlstParam = q.value(6).toString().split(";");
            // для панельных антенн
            if (q.value(1).toInt() == 100) {
                dan.Gain = q.value(5).toFloat();               // 0 - Тип передатчика (0 - Связной 1 — Вещятельный 2 — Телевизионный ), 0
                dan.PowerPRD = strlstParam.at(1).toFloat();    // 1 - Мощность передатчика Вт
                // 2 - ?
                // 3 - ?
                dan.FeederLeght = strlstParam.at(4).toFloat(); // 4 - Длина магистрального фидера, м
                dan.FeederLoss = strlstParam.at(5).toFloat();  // 5 - Погонное ослабление дБ/м
                dan.KSVN = strlstParam.at(6).toFloat();        // 6 - КСВН
                dan.LossOther = strlstParam.at(7).toFloat();   // 7 - Прочие потери
                dan.PRD = strlstParam.at(8).toInt();           // 8 - Количество предатчиков
                // 9 - Тип модуляции
                dan.Height = strlstParam.at(10).toFloat();     // 10 - Максимальный линейный размер
                dan.X = strlstParam.at(11).toFloat();          // 11 - Координата X
                dan.Y = strlstParam.at(12).toFloat();          // 12 - Координата Y
                dan.Z = strlstParam.at(13).toFloat();          // 13 - Высота подвеса
                dan.Azimut = strlstParam.at(14).toFloat();     // 14 - Установочный азимут
                dan.Tilt = strlstParam.at(15).toFloat();       // 15 - Угол наклона
                // 16 - ?
                // 17 - Время начала
                // 18 - Вреия конца
                // 19 - Тип антенны (0 - Всенаправленная 1 — Панельная 2 - Волновой канал (Уда-Яги) 3 — Логопериодическая), 1
                // 20 - Ширина ДН по уровню 3
                // 21 - Длинна антенны (для Логопериодических)
                // 22 - Нижняя частота рабочего диапозона (для Логопериодических)
                // 23 - Верхняя частота рабочего диапозона (для Логопериодических)

                // Читаем диаграммы направленности
                strlstRadH = q.value(10).toString().split(";");
                for(int i = 0; i<strlstRadH.size(); i++)
                {
                    dan.AzHoriz[i] = i;
                    dan.RadHoriz[i] =  0 - strlstRadH.at(i).toFloat();
                }
                strlstRadV = q.value(9).toString().split(";");

                for(int i = 0; i<strlstRadV.size(); i++)
                {
                    dan.AzVert[i] = i;
                    dan.RadVert[i] = -strlstRadV.at(i).toFloat();
                }
                dan.mirorrHoriz();
                dan.mirorrsVert();
                dan.Type = q.value(1).toInt();
                strlstRadH = q.value(10).toString().split(";");
                dan.PowerTotal = dan.calcPower();
            }  // конец if для панельных антенн

            // для РРС
            if (q.value(1).toInt() == 15) {
                dan.Gain = strlstParam.at(0).toFloat();
                dan.Height = strlstParam.at(1).toFloat();
                dan.PowerPRD = strlstParam.at(11).toFloat();
                dan.FeederLeght = strlstParam.at(12).toFloat();
                dan.FeederLoss = strlstParam.at(13).toFloat();
                dan.KSVN = strlstParam.at(14).toFloat();
                dan.LossOther = strlstParam.at(15).toFloat();
                dan.PRD = strlstParam.at(16).toFloat();
                dan.Tilt = strlstParam.at(18).toFloat();
                dan.X = strlstParam.at(19).toFloat();
                dan.Y = strlstParam.at(20).toFloat();
                dan.Azimut = strlstParam.at(21).toFloat();
                dan.Z = strlstParam.at(22).toFloat();
                dan.Type = 15;
                dan.PowerTotal = dan.calcPower();
            }
            antennas.append(dan);
        } // while ( q.next() )

        // Задания
        Task tskRead;
        q.exec("SELECT * FROM Calc_Task ORDER BY Nmbr Asc");
        while ( q.next() ) {
            QStringList strlTask(q.value(2).toString().split(";"));

            tskRead.Enabled = q.value(4).toBool();
            tskRead.Path = q.value(3).toString();
            tskRead.Type = strlTask.first().toInt();
            tskRead.Number = q.value(1).toInt();
            strlTask.removeFirst();
            strlTask.removeLast();
            tskRead.Data = strlTask.join(";");
            tasks.append(tskRead);

            strlTask.clear();
        }
        q.exec("SELECT * FROM Prmt");
        q.next();

        dbPKAEMO.database("PKAEMO").close();

        init();

        for (int i=0; i<antennas.size(); i++)
            Project().addAntenna(antennas.at(i));

        for (int i=0; i<tasks.size(); i++)
            Project().addTask(tasks.at(i));
    }
}

/* ------------------------------------ Добавить ПРТО ------------------------------------ */
bool Project::addAntenna(Prto adIns)
{
    QString qsDanHoriz = "";
    QString qsDanVert = "";

    for (int i=0; i<=360; i++) {
        qsDanHoriz = qsDanHoriz.append( QString::number(adIns.RadHoriz[i]) );
        qsDanHoriz = qsDanHoriz.append(";");
        qsDanVert = qsDanVert.append( QString::number(adIns.RadVert[i]) );
        qsDanVert = qsDanVert.append(";");
    }

    QSqlQuery query(QSqlDatabase::database("project"));

    query.exec("SELECT Number FROM Prto ORDER BY Number DESC");
    query.first();
    int Number(query.value(0).toInt() + 1);

    query.prepare(" INSERT INTO Prto (Enabled, Name, Owner, Freq, Gain, Height, Number, PowerTotal, PowerPRD, "
                  "   FeederLeght, FeederLoss, KSVN, LossOther, PRD, X, Y, Z, Azimut, Tilt, RadHoriz, RadVert, Type) "
                  " VALUES (:Enabled, :Name, :Owner, :Freq, :Gain, :Height, :Number, :PowerTotal, :PowerPRD, "
                  "   :FeederLeght, :FeederLoss, :KSVN, :LossOther, :PRD, :X, :Y, :Z, :Azimut, :Tilt, :RadHoriz, "
                  "   :RadVert, :Type) ");
    query.bindValue(":Enabled",     adIns.Enabled);
    query.bindValue(":Name",        adIns.Name);
    query.bindValue(":Owner",       adIns.Owner);
    query.bindValue(":Freq",        adIns.Freq);
    query.bindValue(":Gain",        adIns.Gain);
    query.bindValue(":Height",      adIns.Height);
    query.bindValue(":Number",      Number);
    query.bindValue(":PowerTotal",  adIns.PowerTotal);
    query.bindValue(":PowerPRD",    adIns.PowerPRD);
    query.bindValue(":FeederLeght", adIns.FeederLeght);
    query.bindValue(":FeederLoss",  adIns.FeederLoss);
    query.bindValue(":KSVN",        adIns.KSVN);
    query.bindValue(":LossOther",   adIns.LossOther);
    query.bindValue(":PRD",         adIns.PRD);
    query.bindValue(":X",           adIns.X);
    query.bindValue(":Y",           adIns.Y);
    query.bindValue(":Z",           adIns.Z);
    query.bindValue(":Azimut",      adIns.Azimut);
    query.bindValue(":Tilt",        adIns.Tilt);
    query.bindValue(":RadHoriz",    qsDanHoriz);
    query.bindValue(":RadVert",     qsDanVert);
    query.bindValue(":Type",        adIns.Type);

    return query.exec();
}

bool Project::prtoEdit(Prto dan)
{
    QString qsDanHoriz = "";
    QString qsDanVert = "";

    for (int i=0; i<=360; i++) {
        qsDanHoriz = qsDanHoriz.append( QString::number(dan.RadHoriz[i]) );
        qsDanHoriz = qsDanHoriz.append(";");
        qsDanVert = qsDanVert.append( QString::number(dan.RadVert[i]) );
        qsDanVert = qsDanVert.append(";");
    }

    QSqlQuery query(QSqlDatabase::database("project"));

    query.prepare(
        " UPDATE Prto SET "
        "   Type = :Type, Enabled = :Enabled, Name = :Name, Owner = :Owner, Freq = :Freq, Gain = :Gain, "
        "   Height = :Height, Number = :Number, PowerTotal = :PowerTotal, PowerPRD = :PowerPRD, "
        "   FeederLeght = :FeederLeght, FeederLoss = :FeederLoss, KSVN = :KSVN, LossOther = :LossOther, "
        "   PRD = :PRD, X = :X, Y = :Y, Z = :Z, Azimut = :Azimut, Tilt = :Tilt, RadHoriz = :RadHoriz, "
        "   RadVert = :RadVert, Type = :Type "
        " WHERE id = :id"
    );
    query.bindValue(":Type",        dan.Type);
    query.bindValue(":Enabled",     dan.Enabled);
    query.bindValue(":Name",        dan.Name);
    query.bindValue(":Number",      dan.Number);
    query.bindValue(":Owner",       dan.Owner);
    query.bindValue(":Freq",        dan.Freq);
    query.bindValue(":Gain",        dan.Gain);
    query.bindValue(":Height",      dan.Height);
    query.bindValue(":PowerTotal",  dan.PowerTotal);
    query.bindValue(":PowerPRD",    dan.PowerPRD);
    query.bindValue(":PRD",         dan.PRD);
    query.bindValue(":FeederLeght", dan.FeederLeght);
    query.bindValue(":FeederLoss",  dan.FeederLoss);
    query.bindValue(":KSVN",        dan.KSVN);
    query.bindValue(":LossOther",   dan.LossOther);
    query.bindValue(":X",           dan.X);
    query.bindValue(":Y",           dan.Y);
    query.bindValue(":Z",           dan.Z);
    query.bindValue(":Azimut",      dan.Azimut);
    query.bindValue(":Tilt",        dan.Tilt);    
    query.bindValue(":RadHoriz",    dan.RadHorizToString());
    query.bindValue(":RadVert",     dan.RadVertToString());
    query.bindValue(":id",          dan.Id);

    return query.exec();
}


/* ------------------------------------ Добавить ПРТО РРС ------------------------------------ */
void Project::prtoAddPPC()
{
    Prto dan;
    dan.Enabled = true;
    dan.Name = "PPC";
    dan.Freq = 10000;
    dan.Gain = 40;
    dan.Height = 0.3;
    dan.PowerPRD = 0.1;
    dan.PRD = 1;
    dan.Tilt = 0;
    dan.X = 0;
    dan.Y = 0;
    dan.Azimut = 0;
    dan.Z = 0;
    dan.Type = 15;

    addAntenna(dan);
}


/* ------------------------------------ Все ПРТО в вектор ------------------------------------ */
QVector<Prto> Project::prtoFromDb()
{
    QSqlQuery query(QSqlDatabase::database("project"));
    QVector<Prto> vecPrto;
    Prto apRead;

    query.exec("SELECT * FROM Prto ORDER BY Number ASC");
    while (query.next()) {
        apRead.clear();

        apRead.Id = query.value("id").toInt();
        apRead.Number = query.value("Number").toInt();
        apRead.Enabled = query.value("Enabled").toBool();
        apRead.Name = query.value("Name").toString();
        apRead.Owner = query.value("Owner").toString();
        apRead.Freq = query.value("Freq").toFloat();
        apRead.Gain = query.value("Gain").toFloat();
        apRead.Height = query.value("Height").toFloat();
        apRead.Type = query.value("Type").toInt();

        apRead.PowerTotal = query.value("PowerTotal").toFloat();
        apRead.PowerPRD = query.value("PowerPRD").toFloat();
        apRead.FeederLeght = query.value("FeederLeght").toFloat();
        apRead.FeederLoss = query.value("FeederLoss").toFloat();
        apRead.KSVN = query.value("KSVN").toFloat();
        apRead.LossOther = query.value("LossOther").toFloat();
        apRead.PRD = query.value("PRD").toInt();

        apRead.X = query.value("X").toFloat();
        apRead.Y = query.value("Y").toFloat();
        apRead.Z = query.value("Z").toFloat();
        apRead.Azimut = query.value("Azimut").toFloat();
        apRead.Tilt = query.value("Tilt").toFloat();

        QStringList qslRadHoriz;
        QStringList qslRadVert;
        qslRadHoriz = query.value("RadHoriz").toString().split(";");
        qslRadVert = query.value("RadVert").toString().split(";");

        for (int i = 0; i < 360; i++) {
            apRead.AzHoriz[i] = i;
            apRead.AzVert[i] = i;
            apRead.RadHoriz[i] = qslRadHoriz.at(i).toFloat();
            apRead.RadVert[i] = qslRadVert.at(i).toFloat();
        }
        vecPrto.append(apRead);
    }
    return vecPrto;
}


/* ------------------------------------ Все Задания в вектор ------------------------------------ */
QVector<Task> Project::taskFromDb()
{
    QSqlQuery query(QSqlDatabase::database("project"));
    QVector<Task> tasks;
    Task task;

    query.exec("SELECT * FROM TaskCalc ORDER BY Number ASC");

    while (query.next()) {
        task.clear();
        task.Enabled = query.value("Enabled").toBool();
        task.Number = query.value("Number").toInt();
        task.Type = query.value("TYPE").toInt();
        task.Data = query.value("TASK").toString();
        task.Path = query.value("Path").toString();
        tasks.append(task);
    }
    return tasks;
}


/* ------------------------------------ Экспорт в ПКАЭМО ------------------------------------ */
void Project::exportPKAEMO(const QString fileCopy)
{
    if (QFile(fileCopy).exists())
        QFile(fileCopy).remove();

    QFile qfExport("pkaemo.rto");

    qfExport.copy(fileCopy);
    qfExport.setFileName(fileCopy);
    qfExport.rename(fileCopy);

    QSqlDatabase dbPKAEMO = QSqlDatabase::addDatabase("QODBC","expPKAEMO");
    dbPKAEMO.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DSN='';CharSet='UTF8';DBQ="
                             + qfExport.fileName());

    if (dbPKAEMO.database("expPKAEMO").open())
    {
        QSqlQuery qEdit(dbPKAEMO);
        Prto apRead;
        QVector<Prto> vecPrtoAll(prtoFromDb());

        for(int i=0; i < vecPrtoAll.count(); i++)
        {
            apRead.clear();
            apRead = vecPrtoAll.at(i);

            QString qsParam;
            qsParam.clear();

            if( apRead.Type == 100 )
            {
                qsParam.append("0").append(";");                                     // 0 - Тип передатчика (0 - Связной 1 — Вещятельный 2 — Телевизионный ), 0
                qsParam.append( QString::number(apRead.PowerPRD) ).append(";");      // 1 - Мощность передатчика Вт
                qsParam.append("0").append(";");                                     // 2 - ?
                qsParam.append("0").append(";");                                     // 3 - ?
                qsParam.append( QString::number(apRead.FeederLeght) ).append(";");   // 4 - Длина магистрального фидера, м
                qsParam.append( QString::number(apRead.FeederLoss) ).append(";");    // 5 - Погонное ослабление дБ/м
                qsParam.append( QString::number(apRead.KSVN) ).append(";");          // 6 - КСВН
                qsParam.append( QString::number(apRead.LossOther) ).append(";");     // 7 - Прочие потери
                qsParam.append( QString::number(apRead.PRD) ).append(";");           // 8 - Количество предатчиков
                qsParam.append("0").append(";");                                     // 9 - Тип модуляции
                qsParam.append( QString::number(apRead.Height) ).append(";");        // 10 - Максимальный линейный размер
                qsParam.append( QString::number(apRead.X) ).append(";");             // 11 - Координата X
                qsParam.append( QString::number(apRead.Y) ).append(";");             // 12 - Координата Y
                qsParam.append( QString::number(apRead.Z) ).append(";");             // 13 - Высота подвеса
                qsParam.append( QString::number(apRead.Azimut) ).append(";");        // 14 - Установочный азимут
                qsParam.append( QString::number(apRead.Tilt) ).append(";");          // 15 - Угол наклона
                qsParam.append("0").append(";");                                     // 16 - ?
                qsParam.append("0:00").append(";");                                  // 17 - Время начала
                qsParam.append("24:00").append(";");                                 // 18 - Вреия конца
                qsParam.append("1").append(";");                                     // 19 - Тип антенны (0 - Всенаправленная 1 — Панельная 2 - Волновой канал (Уда-Яги) 3 — Логопериодическая), 1
                qsParam.append(QString::number(apRead.beamwidthHoriz())).append(";");// 20 - Ширина ДН по уровню 3
                qsParam.append("0").append(";");                                     // 21 - Длинна антенны (для Логопериодических)
                qsParam.append("0").append(";");                                     // 22 - Нижняя частота рабочего диапозона (для Логопериодических)
                qsParam.append("0");                                                 // 23 - Верхняя частота рабочего диапозона (для Логопериодических)

                apRead.mirorrHoriz();
                apRead.mirorrsVert();

                qEdit.prepare("INSERT INTO Sost ( id_type, Name, Vlad, Freq, KU, Params, Dn_vert, Dn_gor, Nmbr, Tpdu, Incl) "
                              " VALUES (:id_type, :Name, :Vlad, :Freq, :KU, :Params, :Dn_vert, :Dn_gor, :Nmbr, :Tpdu, :Incl)");
                qEdit.bindValue(":id_type", apRead.Type);
                qEdit.bindValue(":Name",    apRead.Name);
                qEdit.bindValue(":Vlad",    apRead.Owner);
                qEdit.bindValue(":Freq",    apRead.Freq);
                qEdit.bindValue(":KU",      apRead.Gain);
                qEdit.bindValue(":Params",  qsParam);
                qEdit.bindValue(":Dn_vert", apRead.RadVertToString2());
                qEdit.bindValue(":Dn_gor",  apRead.RadHorizToString2());
                qEdit.bindValue(":Nmbr",    i);
                qEdit.bindValue(":Tpdu",    "3");
                qEdit.bindValue(":Incl",    apRead.Enabled);
                qEdit.exec();
            } // if( apRead.Type == 100 )
            // Для РРС
            if( apRead.Type == 15 )
            {
                qsParam.append( QString::number(apRead.Gain) ).append(";");          // 0 - КУ
                qsParam.append( QString::number(apRead.Height) ).append(";");        // 1 - Диаметр аппертуры
                qsParam.append( "180" ).append(";");                                 // 2 - Угол раскрыва град
                qsParam.append( "0" ).append(";");                                   // 3 - Тип рефлектора ( 0 - сплошной)
                qsParam.append( "0.02" ).append(";");                                // 4 -
                qsParam.append( "0.01" ).append(";");                                // 5 -
                qsParam.append( "0.04" ).append(";");                                // 6 -
                qsParam.append( "0.02" ).append(";");                                // 7 -
                qsParam.append( "0.02" ).append(";");                                // 8 -
                qsParam.append( "0.01").append(";");                                 // 9 -
                qsParam.append( "0.005" ).append(";");                               // 10 -
                qsParam.append( QString::number(apRead.PowerPRD) ).append(";");      // 11 - Мощность ПРД
                qsParam.append( QString::number(apRead.FeederLeght) ).append(";");   // 12 - Длина фидера
                qsParam.append( QString::number(apRead.FeederLoss) ).append(";");    // 13 - Погонное осл
                qsParam.append( QString::number(apRead.KSVN) ).append(";");          // 14 - КСВН
                qsParam.append( QString::number(apRead.LossOther) ).append(";");     // 15 - Прочие потери
                qsParam.append( QString::number(apRead.PRD) ).append(";");           // 16 - Колво ПРД
                qsParam.append( "" ).append(";");                                    // 17 - Тип Модуляции
                qsParam.append( QString::number(apRead.Tilt) ).append(";");          // 18 - Угол наклона
                qsParam.append( QString::number(apRead.X) ).append(";");             // 19 - X
                qsParam.append( QString::number(apRead.Y) ).append(";");             // 20 - Y
                qsParam.append( QString::number(apRead.Azimut) ).append(";");        // 21 - Азимут
                qsParam.append( QString::number(apRead.Z) ).append(";");             // 22 - Высота подвеса
                qsParam.append("0:00").append(";");                                  // 23 - Время нач
                qsParam.append("24:00");                                             // 24 - Время кнц

                qEdit.prepare("INSERT INTO Sost ( id_type, Name, Vlad, Freq, Params, Nmbr, Tpdu, Incl) "
                              "VALUES (:id_type, :Name, :Vlad, :Freq, :Params, :Nmbr, :Tpdu, :Incl)");
                qEdit.bindValue(":id_type", apRead.Type);
                qEdit.bindValue(":Name",    apRead.Name);
                qEdit.bindValue(":Vlad",    apRead.Owner);
                qEdit.bindValue(":Freq",    apRead.Freq);
                qEdit.bindValue(":Params",  qsParam);
                qEdit.bindValue(":Nmbr",    i);
                qEdit.bindValue(":Tpdu",    "3");
                qEdit.bindValue(":Incl",    apRead.Enabled);
                qEdit.exec();
            }   // if( apRead.Type == 15 )
        } //  for(int i=0; i < vecPrtoAll.count(); i++)

        // Экспорт заданий
        QVector<Task> vecTaskAll(taskFromDb());

        for(int i=0; i < vecTaskAll.count(); i++)
        {
            Task task;
            task = vecTaskAll.at(i);

            qEdit.prepare("INSERT INTO Calc_Task ( Npp, Nmbr, Task, Path, Incl) VALUES (:Npp, :Nmbr, :Task, :Path, :Incl)");
            qEdit.bindValue(":Npp",  i);
            qEdit.bindValue(":Nmbr", i);
            qEdit.bindValue(":Task", QString(QString::number(task.Type)).append(";").append(task.Data).append(";0") );
            qEdit.bindValue(":Path", task.Path);
            qEdit.bindValue(":Incl", task.Enabled);
            qEdit.exec();

            task.clear();
        }
        // Опции
        qEdit.exec("UPDATE Prmt SET Prmt.Koef_r = " + str(koef()) );
    }
    dbPKAEMO.database("expPKAEMO").close();
}


/* ------------------------------------ Коэфициент ------------------------------------ */
float Project::koef()
{
    QSqlQuery query(QSqlDatabase::database("project"));

    query.exec("SELECT * FROM Options");
    query.next();

    return query.value("Koef").toFloat();
}


/* ------------------------------------ Добавить задание ------------------------------------ */
bool Project::addTask(Task task)
{
    QSqlQuery query(QSqlDatabase::database("project"));

    if (task.Id == -1) {
        query.exec("SELECT Number FROM TaskCalc ORDER BY Number DESC");
        query.first();
        int Number(query.value(0).toInt() + 1);

        query.prepare("INSERT INTO TaskCalc (Enabled, TYPE, TASK, Path, Number) "
                      "VALUES(:enabled, :type, :task, :path, :number) " );
        query.bindValue(":enabled", task.Enabled);
        query.bindValue(":type",    task.Type);
        query.bindValue(":task",    task.Data);
        query.bindValue(":path",    task.Path);
        query.bindValue(":number",  Number);
    } else {
        query.prepare ("UPDATE TaskCalc SET Enabled = :enabled, TYPE = :type, TASK = :task, Path = :path, Number = :number WHERE id = :id");
        query.bindValue(":enabled", task.Enabled);
        query.bindValue(":type",    task.Type);
        query.bindValue(":task",    task.Data);
        query.bindValue(":path",    task.Path);
        query.bindValue(":number",  task.Number);
        query.bindValue(":id",  task.Id);
    }
    return query.exec();
}


/* ------------------------------------ Число в строку ------------------------------------ */
QString Project::str(double dNum)
{
    return QString::number(dNum);
}

// ------------------------------------ Кавычки ------------------------------------ /
QString Project::quotedStr(const QString str)
{
    QString result;
    return result.append("'").append(str).append("'");
}

bool Project::saveAntennasToCsv(const QString fileName)
{
    QFile fileCsv(fileName);
    if ( !fileCsv.open(QIODevice::WriteOnly) )
        return false;

    QTextStream out(&fileCsv);
    out.setCodec("UTF-8");
    out << QString("Название;Сектор;Частота;КУ;Размер;Мощность на входе;X;Y;Высота;Азимут;Угол наклона").toUtf8()
        << "\n";

    QVector<Prto> prto ( prtoFromDb() );
    QStringList line;
    for (int i = 0; i < prto.count(); i++) {
        line.clear();
        if ( prto.at(i).Enabled ) {
            line.append( prto.at(i).Name );
            line.append( prto.at(i).Owner );
            line.append( QString::number(prto.at(i).Freq) );
            line.append( QString::number(prto.at(i).Gain) );
            line.append( QString::number(prto.at(i).Height) );
            line.append( QString::number(prto.at(i).PowerTotal) );
            line.append( QString::number(prto.at(i).X) );
            line.append( QString::number(prto.at(i).Y) );
            line.append( QString::number(prto.at(i).Z) );
            line.append( QString::number(prto.at(i).Azimut) );
            line.append( QString::number(prto.at(i).Tilt) );
            out << line.join(";") << "\n";
        }
    }

    fileCsv.close();
    return true;
}
