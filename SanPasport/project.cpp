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

bool Project::init()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "project");
    db.setDatabaseName(":memory:");

    if (!db.database("project").open())
        return false;

    QSqlQuery query(db);

    // Таблица с заданиями
    query.exec(
        "CREATE TABLE tasks ( "
            "  id INTEGER PRIMARY KEY " // 0
            ", enabled INTEGER " // 1
            ", type INTEGER "    // 2
            ", params TEXT "     // 3
            ", path TEXT "       // 4
            ", sort REAL "       // 5
        ")"
    );

    // Таблица с параметрами
    query.exec("CREATE TABLE options (koef REAL, kpp REAL)");
    query.exec("INSERT INTO options VALUES(1.22, 3, 0)");

    // Таблица с Антенами
    query.exec(
        "CREATE TABLE antennas ("
            "  id INTEGER PRIMARY KEY " // 0
            ", enabled TEXT "           // 1
            ", name TEXT "              // 2
            ", owner TEXT "             // 3
            ", frequency REAL "         // 4
            ", gain REAL "              // 5
            ", height REAL "            // 6
            ", polarization TEXT "      // 7
            ", power_total REAL "       // 8
            ", power_trx REAL "         // 9
            ", count_trx INT "          // 10
            ", feeder_leght REAL "      // 11
            ", feeder_loss REAL "       // 12
            ", ksvn REAL "              // 13
            ", loss_other REAL "        // 14
            ", x REAL "                 // 15
            ", y REAL "                 // 16
            ", z REAL "                 // 17
            ", azimut REAL "            // 18
            ", tilt REAL "              // 19
            ", sort REAL "              // 20
            ", rad_horizontal TEXT "    // 21
            ", rad_vertical TEXT "      // 22
            ", pdu INT"                 // 23
            ", type INT"                // 24
        ")"
    );

    // Таблица СитПлан
    query.exec(
        "CREATE TABLE files_zoz ("
            "  id INTEGER PRIMARY KEY "
            ", status TEXT "
            ", height INTEGER "
            ", path TEXT "
            ", data TEXT "
            ", size REAL"
        ")"
    );

    //  Таблица с параметрами
    query.exec(
        "CREATE TABLE options_sitplan ("
            "  title TEXT "
            ", zoz_pen REAL "
            ", zoz_color TEXT "
            ", zoz_grind INT "
            ", album_count INT"
        ")"
    );
    query.exec(
        " INSERT INTO options_sitplan"
        "   (title, zoz_pen, zoz_color, zoz_grind, album_count) "
        " VALUES"
        "   ('Ситуационный план ([S]).<br/> Высота среза [H] м', 4, '', 0, 0)"
    );

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
    QVector<Antenna> antennas;
    QVector<Task> tasks;

    Antenna antenna;

    QSqlDatabase dbPKAEMO = QSqlDatabase::addDatabase("QODBC","PKAEMO");
    dbPKAEMO.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DSN='';CharSet='UTF8';DBQ="+f);

    if (dbPKAEMO.database("PKAEMO").open()) {
        QSqlQuery query(dbPKAEMO);
        QStringList strlstRadH;
        QStringList strlstRadV;
        QStringList strlstParam;

        query.exec("SELECT * FROM Sost ORDER BY Nmbr Asc");
        while (query.next()) {
            antenna.clear();
            antenna.Enabled = query.value(13).toBool();
            antenna.Name = query.value(2).toString();
            antenna.Owner = query.value(3).toString();
            antenna.Frequency = query.value(4).toFloat();
            antenna.Number = query.value(11).toInt();

            strlstParam = query.value(6).toString().split(";");
            // для панельных антенн
            if (query.value(1).toInt() == 100) {
                antenna.Gain = query.value(5).toFloat();               // 0 - Тип передатчика (0 - Связной 1 — Вещятельный 2 — Телевизионный ), 0
                antenna.PowerTrx = strlstParam.at(1).toFloat();    // 1 - Мощность передатчика Вт
                // 2 - ?
                // 3 - ?
                antenna.FeederLeght = strlstParam.at(4).toFloat(); // 4 - Длина магистрального фидера, м
                antenna.FeederLoss = strlstParam.at(5).toFloat();  // 5 - Погонное ослабление дБ/м
                antenna.KSVN = strlstParam.at(6).toFloat();        // 6 - КСВН
                antenna.LossOther = strlstParam.at(7).toFloat();   // 7 - Прочие потери
                antenna.CountTrx = strlstParam.at(8).toInt();           // 8 - Количество предатчиков
                // 9 - Тип модуляции
                antenna.Height = strlstParam.at(10).toFloat();     // 10 - Максимальный линейный размер
                antenna.X = strlstParam.at(11).toFloat();          // 11 - Координата X
                antenna.Y = strlstParam.at(12).toFloat();          // 12 - Координата Y
                antenna.Z = strlstParam.at(13).toFloat();          // 13 - Высота подвеса
                antenna.Azimut = strlstParam.at(14).toFloat();     // 14 - Установочный азимут
                antenna.Tilt = strlstParam.at(15).toFloat();       // 15 - Угол наклона
                // 16 - ?
                // 17 - Время начала
                // 18 - Вреия конца
                // 19 - Тип антенны (0 - Всенаправленная 1 — Панельная 2 - Волновой канал (Уда-Яги) 3 — Логопериодическая), 1
                // 20 - Ширина ДН по уровню 3
                // 21 - Длинна антенны (для Логопериодических)
                // 22 - Нижняя частота рабочего диапозона (для Логопериодических)
                // 23 - Верхняя частота рабочего диапозона (для Логопериодических)

                // Читаем диаграммы направленности
                strlstRadH = query.value(10).toString().split(";");
                for(int i = 0; i<strlstRadH.size(); i++)
                {
                    antenna.AzimutHorizontal[i] = i;
                    antenna.RadHorizontal[i] =  0 - strlstRadH.at(i).toFloat();
                }
                strlstRadV = query.value(9).toString().split(";");

                for(int i = 0; i<strlstRadV.size(); i++)
                {
                    antenna.AzimutVertical[i] = i;
                    antenna.RadVertical[i] = -strlstRadV.at(i).toFloat();
                }
                antenna.mirorrHorizontal();
                antenna.mirorrsVertical();
                antenna.Type = query.value(1).toInt();
                strlstRadH = query.value(10).toString().split(";");
                antenna.PowerTotal = antenna.calcPower();
            }  // конец if для панельных антенн

            // для РРС
            if (query.value(1).toInt() == 15) {
                antenna.Gain = strlstParam.at(0).toFloat();
                antenna.Height = strlstParam.at(1).toFloat();
                antenna.PowerTrx = strlstParam.at(11).toFloat();
                antenna.FeederLeght = strlstParam.at(12).toFloat();
                antenna.FeederLoss = strlstParam.at(13).toFloat();
                antenna.KSVN = strlstParam.at(14).toFloat();
                antenna.LossOther = strlstParam.at(15).toFloat();
                antenna.CountTrx = strlstParam.at(16).toFloat();
                antenna.Tilt = strlstParam.at(18).toFloat();
                antenna.X = strlstParam.at(19).toFloat();
                antenna.Y = strlstParam.at(20).toFloat();
                antenna.Azimut = strlstParam.at(21).toFloat();
                antenna.Z = strlstParam.at(22).toFloat();
                antenna.Type = 15;
                antenna.PowerTotal = antenna.calcPower();
            }
            antennas.append(antenna);
        } // while ( q.next() )

        // Задания
        Task tskRead;
        query.exec("SELECT * FROM Calc_Task ORDER BY Nmbr Asc");
        while ( query.next() ) {
            QStringList strlTask(query.value(2).toString().split(";"));

            tskRead.Enabled = query.value(4).toBool();
            tskRead.Path = query.value(3).toString();
            tskRead.Type = strlTask.first().toInt();
            tskRead.Number = query.value(1).toInt();
            strlTask.removeFirst();
            strlTask.removeLast();
            tskRead.Params = strlTask.join(";");
            tasks.append(tskRead);

            strlTask.clear();
        }
        query.exec("SELECT * FROM Prmt");
        query.next();

        dbPKAEMO.database("PKAEMO").close();

        init();

        for (int i=0; i<antennas.size(); i++)
            Project().addAntenna(antennas.at(i));

        for (int i=0; i<tasks.size(); i++)
            Project().addTask(tasks.at(i));
    }
}

/* ------------------------------------ Добавить ПРТО ------------------------------------ */
bool Project::addAntenna(Antenna antenna)
{
    QString qsDanHoriz = "";
    QString qsDanVert = "";

    for (int i=0; i<=360; i++) {
        qsDanHoriz = qsDanHoriz.append( QString::number(antenna.RadHorizontal[i]) );
        qsDanHoriz = qsDanHoriz.append(";");
        qsDanVert = qsDanVert.append( QString::number(antenna.RadVertical[i]) );
        qsDanVert = qsDanVert.append(";");
    }

    QSqlQuery query(QSqlDatabase::database("project"));

    query.exec("SELECT sort FROM antennas ORDER BY sort DESC");
    query.first();
    int Number(query.value(0).toInt() + 1);

    query.prepare(" INSERT INTO antennas (enabled, name, owner, frequency, gain, height, sort, power_total, power_trx, "
                  "   feeder_leght, feeder_loss, ksvn, loss_other, count_trx, x, y, z, azimut, tilt, "
                  "   rad_horizontal, rad_vertical, type) "
                  " VALUES (:enabled, :name, :owner, :frequency, :gain, :height, :sort, :power_total, :power_trx, "
                  "   :feeder_leght, :feeder_loss, :ksvn, :loss_other, :count_trx, :x, :y, :Z, :azimut, :tilt, "
                  "   :rad_horizontal,  :rad_vertical, :type) ");

    query.bindValue(":enabled",        antenna.Enabled);
    query.bindValue(":name",           antenna.Name);
    query.bindValue(":owner",          antenna.Owner);
    query.bindValue(":frequency",      antenna.Frequency);
    query.bindValue(":gain",           antenna.Gain);
    query.bindValue(":height",         antenna.Height);
    query.bindValue(":sort",           Number);
    query.bindValue(":power_total",    antenna.PowerTotal);
    query.bindValue(":power_trx",      antenna.PowerTrx);
    query.bindValue(":feeder_leght",   antenna.FeederLeght);
    query.bindValue(":feeder_loss",    antenna.FeederLoss);
    query.bindValue(":ksvn",           antenna.KSVN);
    query.bindValue(":loss_other",     antenna.LossOther);
    query.bindValue(":count_trx",      antenna.CountTrx);
    query.bindValue(":x",              antenna.X);
    query.bindValue(":y",              antenna.Y);
    query.bindValue(":z",              antenna.Z);
    query.bindValue(":azimut",         antenna.Azimut);
    query.bindValue(":tilt",           antenna.Tilt);
    query.bindValue(":rad_horizontal", qsDanHoriz);
    query.bindValue(":rad_vertical",   qsDanVert);
    query.bindValue(":type",           antenna.Type);

    return query.exec();
}

bool Project::editAntenna(Antenna antenna)
{
    QString qsDanHoriz = "";
    QString qsDanVert = "";

    for (int i=0; i<=360; i++) {
        qsDanHoriz = qsDanHoriz.append( QString::number(antenna.RadHorizontal[i]) );
        qsDanHoriz = qsDanHoriz.append(";");
        qsDanVert = qsDanVert.append( QString::number(antenna.RadVertical[i]) );
        qsDanVert = qsDanVert.append(";");
    }

    QSqlQuery query(QSqlDatabase::database("project"));

    query.prepare(
        " UPDATE antennas SET "
        "   type = :type, enabled = :enabled, name = :name, owner = :owner, frequency = :frequency, gain = :gain, "
        "   height = :height, sort = :sort, power_total = :power_total, power_trx = :power_trx, "
        "   feeder_leght = :feeder_leght, feeder_loss = :feeder_loss, ksvn = :ksvn, loss_other = :loss_other, "
        "   count_trx = :count_trx, x = :x, y = :y, z = :z, azimut = :azimut, tilt = :tilt, "
        "   rad_horizontal = :rad_horizontal, rad_vertical = :rad_vertical, type = :type "
        " WHERE id = :id"
    );
    query.bindValue(":type",           antenna.Type);
    query.bindValue(":enabled",        antenna.Enabled);
    query.bindValue(":name",           antenna.Name);
    query.bindValue(":sort",           antenna.Number);
    query.bindValue(":owner",          antenna.Owner);
    query.bindValue(":frequency",      antenna.Frequency);
    query.bindValue(":gain",           antenna.Gain);
    query.bindValue(":height",         antenna.Height);
    query.bindValue(":power_total",    antenna.PowerTotal);
    query.bindValue(":power_trx",      antenna.PowerTrx);
    query.bindValue(":count_trx",      antenna.CountTrx);
    query.bindValue(":feeder_leght",   antenna.FeederLeght);
    query.bindValue(":feeder_loss",    antenna.FeederLoss);
    query.bindValue(":ksvn",           antenna.KSVN);
    query.bindValue(":loss_other",     antenna.LossOther);
    query.bindValue(":x",              antenna.X);
    query.bindValue(":y",              antenna.Y);
    query.bindValue(":z",              antenna.Z);
    query.bindValue(":azimut",         antenna.Azimut);
    query.bindValue(":tilt",           antenna.Tilt);
    query.bindValue(":rad_horizontal", antenna.RadHorizontalToString());
    query.bindValue(":rad_vertical",   antenna.RadVerticalToString());
    query.bindValue(":id",             antenna.Id);

    return query.exec();
}


/* ------------------------------------ Добавить ПРТО РРС ------------------------------------ */
void Project::addAntennaPPC()
{
    Antenna antenna;
    antenna.Enabled = true;
    antenna.Name = "PPC";
    antenna.Frequency = 10000;
    antenna.Gain = 40;
    antenna.Height = 0.3;
    antenna.PowerTrx = 0.1;
    antenna.CountTrx = 1;
    antenna.Tilt = 0;
    antenna.X = 0;
    antenna.Y = 0;
    antenna.Azimut = 0;
    antenna.Z = 0;
    antenna.Type = 15;

    addAntenna(antenna);
}


/* ------------------------------------ Все ПРТО в вектор ------------------------------------ */
QVector<Antenna> Project::prtoFromDb()
{
    QSqlQuery query(QSqlDatabase::database("project"));
    QVector<Antenna> antennas;
    Antenna antenna;

    query.exec("SELECT * FROM antennas ORDER BY sort ASC");
    while (query.next()) {
        antenna.clear();

        antenna.Id = query.value("id").toInt();
        antenna.Number = query.value("sort").toInt();
        antenna.Enabled = query.value("enabled").toBool();
        antenna.Name = query.value("name").toString();
        antenna.Owner = query.value("owner").toString();
        antenna.Frequency = query.value("frequency").toFloat();
        antenna.Gain = query.value("gain").toFloat();
        antenna.Height = query.value("height").toFloat();
        antenna.Type = query.value("type").toInt();

        antenna.PowerTotal = query.value("power_total").toFloat();
        antenna.PowerTrx = query.value("power_trx").toFloat();
        antenna.FeederLeght = query.value("feeder_leght").toFloat();
        antenna.FeederLoss = query.value("feeder_loss").toFloat();
        antenna.KSVN = query.value("ksvn").toFloat();
        antenna.LossOther = query.value("loss_other").toFloat();
        antenna.CountTrx = query.value("count_trx").toInt();

        antenna.X = query.value("x").toFloat();
        antenna.Y = query.value("y").toFloat();
        antenna.Z = query.value("z").toFloat();
        antenna.Azimut = query.value("azimut").toFloat();
        antenna.Tilt = query.value("tilt").toFloat();

        QStringList radHorizontal;
        QStringList radVertical;
        radHorizontal = query.value("rad_horizontal").toString().split(";");
        radVertical = query.value("rad_vertical").toString().split(";");

        for (int i = 0; i < 360; i++) {
            antenna.AzimutHorizontal[i] = i;
            antenna.AzimutVertical[i] = i;
            antenna.RadHorizontal[i] = radHorizontal.at(i).toFloat();
            antenna.RadVertical[i] = radVertical.at(i).toFloat();
        }
        antennas.append(antenna);
    }
    return antennas;
}


/* ------------------------------------ Все Задания в вектор ------------------------------------ */
QVector<Task> Project::taskFromDb()
{
    QSqlQuery query(QSqlDatabase::database("project"));
    QVector<Task> tasks;
    Task task;

    query.exec("SELECT * FROM tasks ORDER BY sort ASC");

    while (query.next()) {
        task.clear();
        task.Id = query.value("id").toInt();
        task.Enabled = query.value("enabled").toBool();
        task.Number = query.value("sort").toInt();
        task.Type = query.value("type").toInt();
        task.Params = query.value("params").toString();
        task.Path = query.value("path").toString();
        tasks.append(task);
    }
    return tasks;
}

/* ------------------------------------ Все Задания в вектор ------------------------------------ */
QVector<Task> Project::getTasks(int type)
{
    QSqlQuery query(QSqlDatabase::database("project"));
    QVector<Task> tasks;
    Task task;
    query.exec("SELECT * FROM tasks WHERE type = " + QString::number(type) + " ORDER BY sort ASC");

    while (query.next()) {
        task.clear();
        task.Id = query.value("id").toInt();
        task.Enabled = query.value("enabled").toBool();
        task.Number = query.value("sort").toInt();
        task.Type = query.value("type").toInt();
        task.Params = query.value("params").toString();
        task.Path = query.value("path").toString();
        tasks.append(task);
    }
    return tasks;
}

/* ------------------------------------ Экспорт в ПКАЭМО ------------------------------------ */
void Project::exportToPkaemo(const QString fileCopy)
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
        Antenna apRead;
        QVector<Antenna> vecPrtoAll(prtoFromDb());

        for(int i=0; i < vecPrtoAll.count(); i++)
        {
            apRead.clear();
            apRead = vecPrtoAll.at(i);

            QString qsParam;
            qsParam.clear();

            if( apRead.Type == 100 )
            {
                qsParam.append("0").append(";");                                     // 0 - Тип передатчика (0 - Связной 1 — Вещятельный 2 — Телевизионный ), 0
                qsParam.append( QString::number(apRead.PowerTrx) ).append(";");      // 1 - Мощность передатчика Вт
                qsParam.append("0").append(";");                                     // 2 - ?
                qsParam.append("0").append(";");                                     // 3 - ?
                qsParam.append( QString::number(apRead.FeederLeght) ).append(";");   // 4 - Длина магистрального фидера, м
                qsParam.append( QString::number(apRead.FeederLoss) ).append(";");    // 5 - Погонное ослабление дБ/м
                qsParam.append( QString::number(apRead.KSVN) ).append(";");          // 6 - КСВН
                qsParam.append( QString::number(apRead.LossOther) ).append(";");     // 7 - Прочие потери
                qsParam.append( QString::number(apRead.CountTrx) ).append(";");           // 8 - Количество предатчиков
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

                apRead.mirorrHorizontal();
                apRead.mirorrsVertical();

                qEdit.prepare("INSERT INTO Sost ( id_type, Name, Vlad, Freq, KU, Params, Dn_vert, Dn_gor, Nmbr, Tpdu, Incl) "
                              " VALUES (:id_type, :Name, :Vlad, :Freq, :KU, :Params, :Dn_vert, :Dn_gor, :Nmbr, :Tpdu, :Incl)");
                qEdit.bindValue(":id_type", apRead.Type);
                qEdit.bindValue(":Name",    apRead.Name);
                qEdit.bindValue(":Vlad",    apRead.Owner);
                qEdit.bindValue(":Freq",    apRead.Frequency);
                qEdit.bindValue(":KU",      apRead.Gain);
                qEdit.bindValue(":Params",  qsParam);
                qEdit.bindValue(":Dn_vert", apRead.RadVerticalToString2());
                qEdit.bindValue(":Dn_gor",  apRead.RadHorizontalToString2());
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
                qsParam.append( QString::number(apRead.PowerTrx) ).append(";");      // 11 - Мощность ПРД
                qsParam.append( QString::number(apRead.FeederLeght) ).append(";");   // 12 - Длина фидера
                qsParam.append( QString::number(apRead.FeederLoss) ).append(";");    // 13 - Погонное осл
                qsParam.append( QString::number(apRead.KSVN) ).append(";");          // 14 - КСВН
                qsParam.append( QString::number(apRead.LossOther) ).append(";");     // 15 - Прочие потери
                qsParam.append( QString::number(apRead.CountTrx) ).append(";");           // 16 - Колво ПРД
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
                qEdit.bindValue(":Freq",    apRead.Frequency);
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
            qEdit.bindValue(":Task", QString(QString::number(task.Type)).append(";").append(task.Params).append(";0") );
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

    query.exec("SELECT * FROM options");
    query.next();

    return query.value("koef").toFloat();
}


/* ------------------------------------ Добавить задание ------------------------------------ */
bool Project::addTask(Task task)
{
    QSqlQuery query(QSqlDatabase::database("project"));

    if (task.Id == -1) {
        query.exec("SELECT sort FROM tasks ORDER BY sort DESC");
        query.first();
        int Number(query.value(0).toInt() + 1);

        query.prepare("INSERT INTO tasks (enabled, type, params, path, sort) "
                      "VALUES(:enabled, :type, :params, :path, :number) " );
        query.bindValue(":enabled", task.Enabled);
        query.bindValue(":type",    task.Type);
        query.bindValue(":params",    task.Params);
        query.bindValue(":path",    task.Path);
        query.bindValue(":sort",  Number);
    } else {
        query.prepare ("UPDATE taks SET "
                       "enabled = :enabled, type = :type, params = :params, path = :path, sort = :sort "
                       "WHERE id = :id");
        query.bindValue(":enabled", task.Enabled);
        query.bindValue(":type",    task.Type);
        query.bindValue(":params",    task.Params);
        query.bindValue(":path",    task.Path);
        query.bindValue(":sort",  task.Number);
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

    QVector<Antenna> antennas ( prtoFromDb() );
    QStringList line;
    for (int i = 0; i < antennas.count(); i++) {
        line.clear();
        if ( antennas.at(i).Enabled ) {
            line.append( antennas.at(i).Name );
            line.append( antennas.at(i).Owner );
            line.append( QString::number(antennas.at(i).Frequency) );
            line.append( QString::number(antennas.at(i).Gain) );
            line.append( QString::number(antennas.at(i).Height) );
            line.append( QString::number(antennas.at(i).PowerTotal) );
            line.append( QString::number(antennas.at(i).X) );
            line.append( QString::number(antennas.at(i).Y) );
            line.append( QString::number(antennas.at(i).Z) );
            line.append( QString::number(antennas.at(i).Azimut) );
            line.append( QString::number(antennas.at(i).Tilt) );
            out << line.join(";") << "\n";
        }
    }

    fileCsv.close();
    return true;
}
