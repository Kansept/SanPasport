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

    ModelAntennas *modelPrto;
    ModelTasks *modelTask;
    QSqlTableModel *modelOptions;
    QFileSystemModel *modelDb;
    QMenu *menuAnt;
    Project *project;
    // Статус Бар
    QLabel *sb1;
    QPushButton *sbKoef;
    QProgressBar *pbar;
    ThreadCalcZoz *Thread;

    void initStatusBar();

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
     void fileImportPkaemo4(const QString pathFile);    // Импорт файла ПКАЭМО
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
     Task taskFromModel(int intRow);  // Прочитать задание из модели
     void taskVertFromPrto();         // Верткальные сечения на основе ПРТО
     void tasZoFromPrto();            // Горизонтальные сечения на основе ПРТО
     void taskKeyPresed(int numKey, int numModifierKey);    // Горячие клавиши
     void contextMenuTask(const QPoint &pos);               // Контексное меню
     void taskMoveUp();               // Переместить вверх
     void taskMoveDown();             // Переместить вниз
     void taskMoved(int LogicIndex, int OldVisualIndex, int NewVisualIndex); // Переместить
     void sortTasks();
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
     void fileOpen(const QString pathFile);
     QString currentPath();
};



#endif // MAINWINDOW_H
