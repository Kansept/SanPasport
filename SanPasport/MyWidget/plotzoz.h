#ifndef PLOTZOZ_H
#define PLOTZOZ_H
#include <math.h>
#include <QVector>
#include <QResizeEvent>

#include <qwt_raster_data.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot.h>
#include "qwt_plot_layout.h"
#include <qwt_raster_data.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_color_map.h>
#include <qwt_plot_grid.h>
#include "qwt_plot_canvas.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_renderer.h"
#include "qwt_plot_svgitem.h"
#include "qwt_transform.h"
#include "qwt_legend.h"

#include "qwt_plot_dict.h"
#include "qwt_plot_layout.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_engine.h"
#include "qwt_text_label.h"
#include "qwt_legend_data.h"
#include "qwt_plot_marker.h"
#include <qwt_plot_picker.h>

class PlotRasterData: public QwtMatrixRasterData
{
public:
    PlotRasterData();
};


class PlotSpectr: public QwtPlotSpectrogram
{
public:
    PlotSpectr();
};

/* ------- Выбор координат ------- */
class PlotZozPicker : public QwtPlotPicker
{
Q_OBJECT

public:
    PlotZozPicker( QWidget *canvas );

// Незабыть убрать const в qwt_picker.h
    virtual void drawTracker( QPainter *painter );

signals:
    void position(QString strPos);
};

class PlotZozMarker: public QwtPlotMarker
{
public:
    PlotZozMarker(const int LineStyle);
public:
    void setLabelValue(const float Value, const QFont Font);
    void setFont(const QFont Font);
};


class PlotZoz: public QwtPlot
{
    Q_OBJECT

public:
    PlotZoz(QWidget * = NULL);

public slots:
    void setAxisEnableForAll(bool b);                                   // Вкл/Выкл Оси
    void setAxisFontForAll(QFont fntAxis);                              // Шрифт сетки
    void setLegendFont(QFont fntAxis);                                  // Шрифт легенды
    void setAxisScaleForAll(float fSide, int numStep);                  // Масштаб Сетки
    void setAxisVisible(bool left, bool right, bool bottom, bool top);  // Видимость Осей
    void setGridVisible(const bool b);   // Видимость сетки
    bool gridIsVisible();                // Видимость сетки
    void setGridPen(const QPen pen);
    void setGridStep(const int Step);
    void setData(QVector<double> vecData, float fZoSide = 0, QColor clrPlot = Qt::red, float fPen = 1);
    void setRastDatInterval(float minX, float maxX, float minY, float maxY);
    void setContourPen(QColor clrPlot, float fPen );                    // Стиль ЗОЗ
    double zozValue(float x, float y);
    float valueXMax();
    float valueYMin();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *ev);

signals:
    void sgnlOutCanvas();

private:
    QwtPlotGrid *pltGrid;
    PlotRasterData *pltRasterData;
    QwtPlotSpectrogram *pltSpectrogram;

};
#endif // PLOTZOZ_H



