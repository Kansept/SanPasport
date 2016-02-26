#ifndef SITPLAN_H
#define SITPLAN_H

#include "MyWidget/mymodeltask.h"
#include "MyWidget/plotzoz.h"
#include "Dialog/dialog_configdocument.h"
#include "Dialog/dialog_configzo.h"
#include "Dialog/dialog_configgrid.h"
#include "config.h"

#include "qwt_plot_canvas.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_renderer.h"
#include "qwt_plot_svgitem.h"
#include "qwt_transform.h"

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsSvgItem>
#include <QComboBox>
#include <QFontComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QGridLayout>
#include <QTreeWidgetItem>
#include <QPrinter>
#include <QTableView>
#include <QSettings>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <math.h>
#include <QStandardItemModel>
#include <QSvgRenderer>
#include <QImageWriter>
#include <QProgressBar>
#include <QFutureWatcher>
#include <QStyledItemDelegate>

namespace Ui {
class SitPlan;
}


/* ---------------- Делегат для страниц ----------------- */
class mDelegateHeight:public QStyledItemDelegate{
    Q_OBJECT
public:
    mDelegateHeight(QWidget *parent=0):QStyledItemDelegate(parent){}
    QString displayText(const QVariant &value, const QLocale &locale) const
    {
        QString mytext("Высота среза " + value.toString() + " м.");

        return QStyledItemDelegate::displayText(mytext, locale);
    }
};


class Dialog_ConfigDocument;
class Dialog_ConfigGrid;
class Dialog_ConfigZo;

class SitPlan : public QMainWindow
{
    Q_OBJECT
    Dialog_ConfigDocument *dlg_ConfigDocument;
    Dialog_ConfigGrid *dlg_ConfigGrid;
    Dialog_ConfigZo *dlg_ConfigZo;

public:
    explicit SitPlan(QWidget *parent = 0);
    ~SitPlan();


public slots:
    void zozLoad(QStringList srtlFiles);
    void zozClear();
    void zozProgressBarMax(const int numMax);
    void zozLoadFromProject();

private slots:
    /// Ситуационный план
    void planOpen();
    void planLoad(const QString f);
    void planForHeight();
    void planForWidth();
    void planForCanvas();
    void planLook(bool b);
    void planAutoSize(bool b);
    void planRotate90CW();
    void planRotate90CWW();
    void planRotate180();
    void planVisible(bool checked);

    /// Зона ограниченной застройки
    void zozOpen();
    void zozReading(const QString f);
    void zozActivate(const QModelIndex &index);
    void zozRemove();
    void zozPlotDraw(const QVector<double> values, const float fZoSide);
    void zozPlotScale(const QString arg1);
    void zozGridVisible(bool checked);
    void zozMove();
    void zozPositionMouse(QPointF point);
    void zozProgressBar(int num);
    void zozKeyPresed(int numKey, int numModifierKey);  // Горячие клавиши

    /// Заголовок
    void titleVisible(bool checked);
    void titleInsertHeight();
    void titleInsertScale();
    void titleInsertBr();
    void titleFontChange();
    void titleCursorPositionChanged(bool bMask);

    /// Страницы
    void pageSize(const QString &arg1);
    void pagePortrait();
    void pageLandscape();
    void pageViewFirst();
    void pageViewBack();
    void pageViewNext();
    void pageViewLast();
    void pageView(const QString arg);
    void pageMoveUp();
    void pageMoveDown();
    void pageMoveTop();
    void pageMoveBottom();

    /// Печать
    void printEnabled(const bool b);
    void printPreview();
    void printBegin(QPrinter *printer);
    void printQuick();
    void printPDF();

    /// Настройки
    void settingRead();
    void settingApplyZoz(QVariant qvarZoConfig);
    void settingApplyGrid(QVariant qvarConfigGrid);
    void settingApplyPage(QVariant qvarConfigPage);
    void settingApply();
    void settingWrite();

    /// Диалоги
    void showConfigZo();
    void showConfigPage();
    void showConfigGrid();
    void showConfigGridFont();

    /// Меню - Вид
    void viewModeSelect();
    void viewModeDrag();
    void viewZoomOriginal();
    void viewZoomBest();
    void viewZoomIn();
    void viewZoomOut();
    void viewZoomCustom();

    /// Вспомогательные
    float cmToPixel(const float x);
    float pixelToM(const float x);

    void contextMenuGrapicsZoz(QPoint pos);
private:
    Ui::SitPlan *ui;
    QSettings *settings;
    float g_fZOScale;

    /// ТулБар
    QAction *PageLandscape;
    QAction *PagePortrait;
    QAction *TitleInsertHeight;
    QAction *TitleInsertScale;
    QToolBar *ToolBarDocument;
    QToolBar *ToolBarText;
    QToolBar *ToolBarImage;
    QToolBar *ToolBarNavigator;
    QToolBar *ToolBarView;
    QComboBox *comboBox_CanvasSize;
    QComboBox *comboBox_sceneScale;
    QComboBox *comboBox_SetZOScale;
    QLineEdit *lineEdit_CurrentPage;
    QLabel *label_AllPage;
    QSlider *zoomSlider;
    QWidget *widget;

    QGraphicsScene *scene;
    QGraphicsRectItem *rectangle;
    QGraphicsItem *pixCenter;
    QGraphicsTextItem * text;
    QGraphicsProxyWidget *proxyPlot;
    QGraphicsSvgItem *svgSitPlan;

    PlotZO *plotZoz;

    QSqlTableModel *modelFilesZO;
    QSqlTableModel *modelOptionsSitPlan;

    /// Статус Бар
    QLabel *sb1;
    QLabel *sb2;
    QLabel *sb3;
    QLabel *sb4;
    QLabel *sb5;
    QLabel *sb6;
    QProgressBar *pbar;

protected:
    void showEvent(QShowEvent *event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent * event);

signals:
    void pbarVal(int);
};

#endif // SITPLAN_H
