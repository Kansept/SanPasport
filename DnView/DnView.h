#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <qwt_legend.h>
#include <qwt_polar_grid.h>
#include <qwt_polar_curve.h>
#include <qwt_polar_panner.h>
#include <qwt_polar_magnifier.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_polar_marker.h>
#include <qwt_symbol.h>
#include <QSettings>
#include <QLabel>
#include <QToolButton>
#include <QTemporaryFile>

#include "prto.h"
#include "userdelegate.h"


// ----------- Зумер -------------------------------------
// #include "qwt_modifer\qwtchartzoom.h"
// #include "qwt_modifer\qwheelzoomsvc.h" //(если надо)
// #include "qwt_modifer\qaxiszoomsvc.h" //(если надо)

namespace Ui {
class MainWindow;
}

class DialogConvert;
class DialogPreferens;
class Window_projection_PKAEMO4;
class Dialog_GroupChange;

class DnView : public QMainWindow
{
    Q_OBJECT
    DialogConvert *dlgConvert;
    DialogPreferens *dlgPreferens;
    Window_projection_PKAEMO4 *win_projection_PKAEMO4;
    Dialog_GroupChange *dlgGroupChange;
    
public:
    explicit DnView(QWidget *parent = 0);
    ~DnView();
    
    QLabel *sb1;
    QLabel *sb2;
    QLabel *sb3;
    QLabel *sb4;
    QToolButton *toolButton_MenuScale;
    QClipboard *clipboard;
    QMimeData *mimeData;

public slots:
    bool OpenFile(const QString &f);   // Загрузка даных из файла
    void CopyToBuf();

private slots:
    void saveSettings();
    void loadSettings();
    void loadSettingsRecentFiles();
    void action_Open();           // Диалог Открытия
    void addRecentFiles(const QString qfopen);    // Файл -> Недавниие
    void action_Save();           // Диалог Сохранения
    void action_SaveAs();         // Диалог Сохранения как
    void action_Export();         // Диалог Экспорта
    void action_AboutThis();      // Диалог О программе
    void action_ColorHorizChange(int iIndex);
// -------------------------------------------------------------------------
    bool loadedPattern();   // Загрузка даных из файла
    Prto getDanGraph();                // Берём данные из таблиц
    void setDanGraph(Prto dan);        // Помощяем данные в таблицы
// ------------------------------------------------------------------------
    void isEditPattern();
    void isEditPatternInMenu();
    void enableEditPatterns(bool b);
    void setPatternSymmetryBottom();
    void setPatternSymmetryTop();
    void setPreload();
    void graphScale25db(bool b);            // Изменить масштаб ДН (25dB)
    void graphScale40db(bool b);            // Изменить масштаб ДН (40dB)
    void graphScaleNorm(bool b);
    void setPatternRotateCw();
    void setPatternRotateCcw();
    void action_PatternMirror();    // Отразить ДН
    void on_actionViewVertDn(bool b);     // Показать/Скрыть Горизонтальную ДН
    void on_actionViewHorizDn(bool b);    // Показать/Скрыть Вертикальную ДН
    void on_actionViewPanelData(bool b);        // Показать/Скрыть панель с данными
    void on_actionViewPanelTransform(bool b);   // Показать/Скрыть панель трансформации
// ------------------------------------------------------------------------
    void initPolarGraph(Prto dan);       // Инициализация Полярного Графика
    void initDecartGraph(Prto dan);      // Инициализация Декартового Графика
    void initAllGraph();
    void EnableUI(bool b);                  // Инициализация Панелек и чекбоксов
    void openInTextEdit();

    void setDataSheet();  // Сформировать паспорт
    void passportWord(Prto prtoPassp);

    void menuPolar_ContextMenu(const QPoint &pos);
    void closeDN();

    // Недавно открытые файлы
    void on_action_RecentFiles0_triggered();
    void on_action_RecentFiles1_triggered();
    void on_action_RecentFiles2_triggered();
    void on_action_RecentFiles3_triggered();
    void on_action_RecentFiles4_triggered();
    void on_action_RecentFiles5_triggered();
    void on_action_RecentFiles6_triggered();
    void on_action_RecentFiles7_triggered();
    void on_action_RecentFiles8_triggered();
    void on_action_RecentFiles9_triggered();

    void initGraph();

private:
    Ui::MainWindow *ui;
    QSettings *settings;
// Полярный график
    QwtPolarGrid *gridPolar;                       // координатная сетка
    QwtPolarCurve *CurvPolarHoriz,*CurvPolarVert,*CurvPolar3db;   // кривые, отображаемые на графике
// Декартовый график
    QwtPlotGrid *gridDec;                           // координатная сетка
    QwtPlotCurve *CurvDecHoriz,*CurvDecVert;        // кривые, отображаемые на графике
    QwtPlotMarker *d_marker1;
    QwtGraphic *canv;
    QMenu *menuPolar;

protected:
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent * event);


};

#endif // MAINWINDOW_H
