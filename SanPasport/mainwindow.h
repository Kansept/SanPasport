#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "SitPlan/sitplan.h"
#include "createdb.h"
#include "MyWidget/mymodeltask.h"

#include "Task/dialog_taskpoint.h"
#include "Task/dialog_taskzo.h"
#include "Task/dialog_taskvs.h"
#include "Dialog/dialog_editant.h"
#include "Dialog/dialog_parametrs.h"
#include "Dialog/dialog_preferens.h"

#include <QDebug>
#include <QString>
#include <QMainWindow>
#include <QSqlTableModel>
#include <QStyledItemDelegate>
#include <QDebug>
#include <QFileSystemModel>
#include <QPushButton>
#include <QHeaderView>
#include <QThread>
#include <QDate>
#include <QTime>


namespace Ui {

class MainWindow;
}

// ---------------- ДЕЛЕГАТ ДЛЯ ЗАДАНИЙ -----------------
class mDelegate:public QStyledItemDelegate{
    Q_OBJECT
public:
    mDelegate(QWidget *parent=0):QStyledItemDelegate(parent){}
    QString displayText(const QVariant &value, const QLocale &locale) const{


        QString mytext = value.toString();
        QStringList splstr;

        splstr = mytext.split(";");
        mytext.clear();

        // Для расчета ЭМП в точке
        if(splstr.size() == 3)
        {
            mytext.append("Расчёт уровней ЭМП с координатами, ");
            mytext.append("X = ").append(splstr.at(0)).append(" м, ");
            mytext.append("Y = ").append(splstr.at(1)).append(" м, ");
            mytext.append("Z = ").append(splstr.at(2)).append(" м");
        }
        // Для расчета Зоны ограничения
        if(splstr.size() == 8)
        {
            mytext.append("Расчёт зоны ограничения, ");
            mytext.append("Высота среза = ").append(splstr.at(6)).append(" м, ");
            mytext.append("Xmin = ").append(splstr.at(0)).append(" м, ");
            mytext.append("Xmax = ").append(splstr.at(1)).append(" м, ");
            mytext.append("dX = ").append(splstr.at(2)).append(" м, ");
            mytext.append("Ymin = ").append(splstr.at(3)).append(" м, ");
            mytext.append("Ymax = ").append(splstr.at(4)).append(" м, ");
            mytext.append("dY = ").append(splstr.at(5)).append(" м; ");
            if(splstr.at(7) == "0")
                mytext.append("Расчет границ зоны");
            else if(splstr.at(7) == "3")
                mytext.append("Текстовый расчёт");
        }
        // Для расчета Вертикального сечения
        if(splstr.size() == 10)
        {
            mytext.append("Расчёт вертикального сечения, ");
            mytext.append("Азимут = ").append(splstr.at(8)).append(" град. ");
            mytext.append("X = ").append(splstr.at(0)).append(" м, ");
            mytext.append("Y = ").append(splstr.at(1)).append(" м, ");
            mytext.append("Rmin = ").append(splstr.at(2)).append(" м, ");
            mytext.append("Rmax = ").append(splstr.at(3)).append(" м, ");
            mytext.append("dR = ").append(splstr.at(4)).append(" м, ");
            mytext.append("Hmin = ").append(splstr.at(5)).append(" м, ");
            mytext.append("Hmax = ").append(splstr.at(6)).append(" м, ");
            mytext.append("dH = ").append(splstr.at(7)).append(" м; ");
            if(splstr.at(9) == "0")
                mytext.append("Расчет границ зоны");
            else if(splstr.at(9) == "3")
                mytext.append("Текстовый расчёт");
        }
        return QStyledItemDelegate::displayText(mytext, locale);
    }
};
// ---------------- ДЕЛЕГАТ ДЛЯ ЗАДАНИЙ -----------------
class mDelegateStat:public QStyledItemDelegate{
    Q_OBJECT
public:
    mDelegateStat(QWidget *parent=0):QStyledItemDelegate(parent){}
    QString displayText(const QVariant &value, const QLocale &locale) const{
        QString mytext = value.toString();
        mytext = (mytext == "1")? "да" : "нет";
        return QStyledItemDelegate::displayText(mytext, locale);
    }
};

// ---------------- ДЕЛЕГАТ ДЛЯ ЗАДАНИЙ -----------------
class mDelegatePath:public QStyledItemDelegate{
    Q_OBJECT
public:
    mDelegatePath(QWidget *parent=0):QStyledItemDelegate(parent){}
    QString displayText(const QVariant &value, const QLocale &locale) const{

        QString mytext = value.toString();

        if(mytext == "")
            mytext = "";
        else if(mytext != "")
        {
            if(QFile(mytext).exists())
                mytext = "готово";
            else if(mytext == "calc")
                mytext = "расчет...";
            else
                mytext = "не найден";
        }

        return QStyledItemDelegate::displayText(mytext, locale);
    }
};

/// ----------------- Поток ----------------- ///
class CalcZozThread : public QThread
{
    Q_OBJECT

    QVector<task> g_vecTask;
    QVector<Prto> g_vecPrto;
    bool bStop;

public:
    explicit CalcZozThread(QObject *parent = 0)  { }

protected:
    void run()
    {
        for(int countTask = 0; countTask < g_vecTask.count(); countTask++)
        {
            task taskZoz(g_vecTask.at(countTask));

            QStringList strlTaskZoz(taskZoz.Data.split(";"));
            Prto prtoCalc;
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

            if(!QDir("Result").exists())
                QDir().mkdir("Result");

            QFile fileResult("Result/zo_h" + QString::number(fHeight) + "_" + QDate::currentDate().toString("yyyyMMdd")
                             + "_" + QTime::currentTime().toString("hhmmss") + ".dat");

            QTextStream txtstremResult(&fileResult);
            txtstremResult.setCodec("Windows-1251");

            if(fileResult.open(QIODevice::WriteOnly))
            {
                doneResult("calc", taskZoz.Id);
                emit startCalc(-1);
                txtstremResult << QString::fromWCharArray(L"ЗО. h = ") << QString::number(fHeight)
                               << QString::fromWCharArray(L" м\n");

                for(int k=0; k < g_vecPrto.count(); k++)
                {
                    prtoCalc = g_vecPrto.at(k);
                    prtoCalc.NormalzationPattern();
                    g_vecPrto.replace(k, prtoCalc);
                }

                for ( float x = Xmin; x <= Xmax; x += Xd )
                {
                    count++;

                    for ( float y = Ymin; y <= Ymax; y += Yd )
                    {
                        fPDU = 0;

                        for(int k=0; k < g_vecPrto.count(); k++)
                        {
                            prtoCalc.clear();
                            prtoCalc = g_vecPrto.at(k);

                            if(bStop)
                            {
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
    void loadTask(QVector<task> vecTask, QVector<Prto> vecPrto)
    {
        g_vecTask = vecTask;
        g_vecPrto = vecPrto;
        bStop = false;
    }
public slots:
    void stopCalc()
    {
        bStop = true;
    }

signals:
    void startCalc(int value);
    void progressChanged(int progress);
    void done(int value);
    void doneResult(QString strResult, int idTask);
};


class Dialog_EditAnt;
class Dialog_Parametrs;
class Dialog_TaskZo;
class Dialog_TaskVS;
class Dialog_TaskPoint;
class SitPlan;


namespace TblPrtoHeader {
    enum TblPrtoHeader { Id = 1, Name = 2};
}



class MainWindow : public QMainWindow
{
    Q_OBJECT

    Dialog_TaskZo *dlg_TaskZo;
    Dialog_TaskVs *dlg_TaskVs;
    Dialog_TaskPoint *dlg_TaskPoint;
    Dialog_EditAnt *dlg_EditAnt;
    Dialog_Parametrs *dlg_Parametrs;
    Dialog_Preferens *dlg_Preferens;
    SitPlan *win_SitPlan;

public:
    explicit MainWindow(QWidget *parent = 0);
    createDb *prjFile;
    ~MainWindow();



private:
    Ui::MainWindow *ui;

    myModelPrto *modelPrto;
    myModelTask *modelTask;
    QSqlTableModel *modelOptions;
    QFileSystemModel *modelDb;
    QMenu *menuAnt;
    createDb *frmt;

    // Статус Бар
    QLabel *sb1;
    QPushButton *sbKoef;
    QProgressBar *pbar;

    CalcZozThread *Thread;



signals:
    void sendIdEditAnt(Prto qsIdSend);

protected:
    void closeEvent(QCloseEvent *event);

private slots:

     /// Общие
     void dragEnterEvent(QDragEnterEvent* event);      // Реализация Drag'n'Drop
     void dropEvent(QDropEvent* event);                // Реализация Drag'n'Drop
     void EnableUI(bool b);                            // Вкл/Выкл элементы интерфейса

     /// ФАЙЛ
     void fileNew();                             // Новый проект
     void fileOpen();                            // Открыть проект
     void fileLoadModel(const QSqlDatabase db);  // Загрузить модель
     void fileSave();                            // Сохранить проект
     void fileSaveAs();                          // Сохранить проект как
     void fileSaveCalc();                        // Сохранить результаты расчётов
     void fileImportPkaemo4_DialogOpen();        // Импорт файла ПКАЭМО - Диалог открытия файла
     void fileImportPkaemo4(const QString f);    // Импорт файла ПКАЭМО
     void fileExportPkaemo4();                   // Экспорт файла ПКАЭМО
     bool fileClose();                           // Закрыть проект

     /// ПРТО
     void prtoEnable();                     // Вкл/Выкл антенну
     void prtoEnableAll();                  // Включить все антенны
     void prtoDisableAll();                 // Отключить все антенны
     void prtoAdd(const Prto adIns);        // Добавить антенну в проект
     void prtoAddPPC();
     void prtoDublicate();                  // Дублировать антенну
     bool prtoRemove();                     // Удалить антенну
     void prtoEdit();                       // Редактировать антенну
     void prtoSaveAs();                     // Сохранить антенну в файл
     bool prtoFromFile(const QString &f);   // Добавить из файла
     void dbPrtoInsert();                   // Открыть из БД
     Prto prtoFromModel(const int iRow);    // Прочитать ПРТО из модели
     void prtoOpenFile();                   // Открыть файл
     void selected_ant();
     void contextMenuPrto(const QPoint &pos);
     void prtoLoadPPC();                    // Загружаем РРС из фала
     void prtoKeyPresed(int numKey, int numModifierKey);    // Горячие клавиши
     void prtoMoveUp();                     // Переместить вверх
     void prtoMoveDown();                   // Переместить вниз
     void prtoMoved(int LogicIndex, int OldVisualIndex, int NewVisualIndex); // Переместить
     void prtoExportCSV();

     /// База данных
     void dbPrtoView();                     // Просмотр ДН
     void dbPath(const QString strPath);    // Сменить папку базы данных
     void dbBrowse();                       // Открыть в проводнике
     void dbOpenOutside();                  // Открыть в проводнике
     void dbSetRoot(QModelIndex indexRoot); // Открыть папку
     void dbUp();                           // Перейти на уровень вверх
     void dbFilter(bool b);                 // Фильтр
     void dbSearch();                       // Поиск
     void dbSearchClear();                  // Очистить поиск

     /// ЗАДАНИЯ
     void taskView();                 // Просмотр
     void taskEnable();               // Вкл.Выкл
     void taskEnableAll();            // Включить все задания
     void taskDisableAll();           // Отключить все задания
     void taskEdit();                 // Редактировать задание
     bool taskRemove();               // Удалить задание
     task taskFromModel(int intRow);  // Прочитать задание из модели
     void taskVertFromPrto();         // Верткальные сечения на основе ПРТО
     void tasZoFromPrto();            // Горизонтальные сечения на основе ПРТО
     void taskKeyPresed(int numKey, int numModifierKey);    // Горячие клавиши
     void contextMenuTask(const QPoint &pos);               // Контексное меню
     void taskMoveUp();               // Переместить вверх
     void taskMoveDown();             // Переместить вниз
     void taskMoved(int LogicIndex, int OldVisualIndex, int NewVisualIndex); // Переместить

     /// РАСЧЕТ
     void calcStart();                               // Старт расчёта
     void UICalc(bool b);                            // Заморозка UI
     void calcResult(QString strResult, int idTask); // Записать результат
     void progresBar(int value);                     // Прогресс бар
     void showDlgParametrs();                        // Параметры расчета
     void signal_SendOptions(QString qsSendOptions); // Параметры расчета

     /// ГРАФИКА
     void graphcsOpenZo();
     void graphcsSitPlan(); // Открыть ситуационный план

     /// О программе
     void helpPpcCsv();
     void helpAbout();

     /// Дополнительно
     QString quotedStr(const QString str);  // Кавычки



public slots:
     void saveEditantPattern(Prto dan);
     void taskInsert();
     void fileOpen(const QString fOpen);
     QString currentPath();
};



#endif // MAINWINDOW_H
