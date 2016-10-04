#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "SitPlan/sitplan.h"
#include "project.h"
#include "Task/dialog_taskpoint.h"
#include "Task/dialog_taskzo.h"
#include "Task/dialog_taskvs.h"
#include "Dialog/dialog_editant.h"
#include "Dialog/dialog_parametrs.h"
#include "Dialog/dialog_preferens.h"
#include "Threads/ThreadCalcZoz.h"
#include "Models/ModelAntennas.h"
#include "Models/ModelTasks.h"

#include <QDebug>
#include <QString>
#include <QMainWindow>
#include <QSqlTableModel>
#include <QStyledItemDelegate>
#include <QDebug>
#include <QFileSystemModel>
#include <QPushButton>
#include <QHeaderView>
#include <QDate>
#include <QTime>


namespace Ui {

class MainWindow;
}

namespace TaskHeader
{
    enum TaskHeader {
        Id,
        Enabled,
        Type,
        Params,
        Path,
        Sort
    };
}

namespace AntennaHeader
{
    enum AntennaHeader {
        Id,
        Enabled,
        Name,
        Owner,
        Frequency,
        Gain,
        Height,
        Polarization,
        PowerTotal,
        PowerTrx,
        CountTrx,
        FeederLeght,
        FeederLoss,
        Ksvn,
        LossOther,
        X,
        Y,
        Z,
        Azimut,
        Tilt,
        Sort,
        RadHorizontal,
        RadVertical,
        Pdu,
        Type
    };
}

class Dialog_EditAnt;
class Dialog_Parametrs;
class Dialog_TaskZo;
class Dialog_TaskVS;
class Dialog_TaskPoint;
class SitPlan;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Dialog_TaskZo *dialogTaskZoz;
    Dialog_TaskVs *dlg_TaskVs;
    Dialog_TaskPoint *dlg_TaskPoint;
    Dialog_EditAnt *dlg_EditAnt;
    Dialog_Parametrs *dlg_Parametrs;
    Dialog_Preferens *dlg_Preferens;
    SitPlan *win_SitPlan;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    ModelAntennas *modelAntenna;
    ModelTasks *modelTask;
    QSqlTableModel *modelOptions;
    QFileSystemModel *modelDb;
    QMenu *menuAntenna;
    Project *project;
    // Статус Бар
    QLabel *sb1;
    QPushButton *sbKoef;
    QProgressBar *pbar;
    ThreadCalcZoz *Thread;

    void initStatusBar();

signals:
    void sendIdEditAnt(Antenna qsIdSend);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
     // Общие
     void dragEnterEvent(QDragEnterEvent* event);      // Реализация Drag'n'Drop
     void dropEvent(QDropEvent* event);                // Реализация Drag'n'Drop
     void enableUI(bool b);                            // Вкл/Выкл элементы интерфейса
     // ФАЙЛ
     void fileNew();                             // Новый проект
     void fileOpen();                            // Открыть проект
     void fileLoadModel(const QSqlDatabase db);  // Загрузить модель
     void fileSave();                            // Сохранить проект
     void fileSaveAs();                          // Сохранить проект как
     void fileSaveCalc();                        // Сохранить результаты расчётов
     void fileImportPkaemo4_DialogOpen();        // Импорт файла ПКАЭМО - Диалог открытия файла
     void fileImportPkaemo4(const QString pathFile);    // Импорт файла ПКАЭМО
     void fileExportPkaemo4();                   // Экспорт файла ПКАЭМО
     bool fileClose();                           // Закрыть проект
     // ПРТО
     void enableAntenna();                     // Вкл/Выкл антенну
     void enableAllAntennas();                  // Включить все антенны
     void disableAllAntennas();                 // Отключить все антенны
     void addAntenna(const Antenna antenna);        // Добавить антенну в проект
     void addAntennaPPC();
     void dublicateAntenna();                  // Дублировать антенну
     bool removeAntenna();                     // Удалить антенну
     void editAntenna();                       // Редактировать антенну
     void saveAsPattern();                     // Сохранить антенну в файл
     bool AntennaFromFile(const QString &f);   // Добавить из файла
     void dbPrtoInsert();                   // Открыть из БД
     Antenna getAntennaFromModel(const int row);    // Прочитать ПРТО из модели
     void OpenFileAntenna();                   // Открыть файл
     void selected_ant();
     void contextMenuAntenna(const QPoint &pos);
     void loadAntennaPPC();                    // Загружаем РРС из фала
     void AntennaKeyPresed(int numKey, int numModifierKey);    // Горячие клавиши
     void moveUpAntenna();                     // Переместить вверх
     void moveDownAntenna();                   // Переместить вниз
     void moveAntenna(int LogicIndex, int OldVisualIndex, int NewVisualIndex); // Переместить
     void exportAntennasToCsv();
     // База данных
     void dbPrtoView();                     // Просмотр ДН
     void dbPath(const QString strPath);    // Сменить папку базы данных
     void dbBrowse();                       // Открыть в проводнике
     void dbOpenOutside();                  // Открыть в проводнике
     void dbSetRoot(QModelIndex indexRoot); // Открыть папку
     void dbUp();                           // Перейти на уровень вверх
     void dbFilter(bool b);                 // Фильтр
     void dbSearch();                       // Поиск
     void dbSearchClear();                  // Очистить поиск
     // ЗАДАНИЯ
     void taskView();                 // Просмотр
     void taskEnable();               // Вкл.Выкл
     void taskEnableAll();            // Включить все задания
     void taskDisableAll();           // Отключить все задания
     void taskEdit();                 // Редактировать задание
     bool taskRemove();               // Удалить задание
     Task taskFromModel(int intRow);  // Прочитать задание из модели
     void taskVertFromPrto();         // Верткальные сечения на основе ПРТО
     void tasZoFromPrto();            // Горизонтальные сечения на основе ПРТО
     void taskKeyPresed(int numKey, int numModifierKey);    // Горячие клавиши
     void contextMenuTask(const QPoint &pos);               // Контексное меню
     void taskMoveUp();               // Переместить вверх
     void taskMoveDown();             // Переместить вниз
     void taskMoved(int LogicIndex, int OldVisualIndex, int NewVisualIndex); // Переместить
     void sortTasks();
     // РАСЧЕТ
     void calcStart();                               // Старт расчёта
     void UICalc(bool b);                            // Заморозка UI
     void calcResult(QString strResult, int idTask); // Записать результат
     void progresBar(int value);                     // Прогресс бар
     void showDialogParametrs();                        // Параметры расчета
     void signal_SendOptions(QString qsSendOptions); // Параметры расчета
     // ГРАФИКА
     void graphcsOpenZo();
     void graphcsSitPlan(); // Открыть ситуационный план
     // О программе
     void helpPpcCsv();
     void helpAbout();
     // Дополнительно
     QString quotedStr(const QString str);  // Кавычки

public slots:
     void saveEditantPattern(Antenna antenna);
     void taskInsert();
     void fileOpen(const QString pathFile);
     QString currentPath();
};



#endif // MAINWINDOW_H
