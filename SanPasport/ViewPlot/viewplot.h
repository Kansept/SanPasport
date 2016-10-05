#ifndef VIEWPLOT_H
#define VIEWPLOT_H

#include "prto.h"
#include "zoz.h"
#include "MyWidget/qsingleitemsquarelayout.h"
#include "MyWidget/plotzoz.h"
#include "ViewPlot/dialog/dialog_confizgraph.h"
#include <qwt_color_map.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_renderer.h>
#include <qwt_legend.h>
#include <qwt_plot_rescaler.h>
#include <qwt_picker_machine.h>
#include <qwt_scale_widget.h>
#include <QMainWindow>
#include <QSettings>
#include <QLabel>
#include <QDebug>

class Dialog_ConfizGraph;

class Grid: public QwtPlotGrid
{
public:
    Grid()
    {
        enableXMin( true );
        setMajorPen( Qt::gray, 0, Qt::DotLine );
        setMinorPen( Qt::gray, 0, Qt::DotLine );
    }

    virtual void updateScaleDiv( const QwtScaleDiv &xScaleDiv,
        const QwtScaleDiv &yScaleDiv )
    {
        QwtScaleDiv scaleDiv( xScaleDiv.lowerBound(), xScaleDiv.upperBound() );

        scaleDiv.setTicks( QwtScaleDiv::MinorTick, xScaleDiv.ticks( QwtScaleDiv::MinorTick ) );
        scaleDiv.setTicks( QwtScaleDiv::MajorTick, xScaleDiv.ticks( QwtScaleDiv::MediumTick ) );

        QwtPlotGrid::updateScaleDiv( scaleDiv, yScaleDiv );
    }
};



namespace Ui {
class ViewPlot;
}



class ViewPlot : public QMainWindow
{
    Q_OBJECT

    Dialog_ConfizGraph *dlg_ConfizGraph;

public:
    explicit ViewPlot(QWidget *parent = 0);
    ~ViewPlot();

public slots:
    void plotLoadZo(const QString f);
    void plotOpen();
    void showConfizGraph();
    void plotPos(QString strPosition);

    void plotPosReset();
    void plotSquare();


    void showPlot();
    void contextMenuView(const QPoint &pos);
protected:
    void showEvent(QShowEvent *ev);

private slots:
    void copyToBuffer();
    void ploting(Zoz val);
    void plotExport();
    void settingFont();
    void settingRead();

private:
    Ui::ViewPlot *ui;
    QSettings *stngViewPlot;
    QSingleItemSquareLayout* mainLayout;
    QClipboard *clipboard;
    QLabel *sb1;
    QwtPlotRescaler* rescaler;
    PlotZozPicker* picker;
    PlotZozMarker *markerH;
    PlotZozMarker *markerV;
    QwtLegend *qwtLeg;
    QwtPlotCurve *curve;
    QPainter *fPainter;
};

#endif // VIEWPLOT_H
