#include "plotzoz.h"
#include <QLayout>

PlotSpectr::PlotSpectr()
{
    setRenderThreadCount(0);  // use system specific thread count

    QList<double> contourLevels;
    contourLevels += 0.99;
    setContourLevels( contourLevels );

    setAlpha(0);
    setDisplayMode( QwtPlotSpectrogram::ContourMode, true );
    setCachePolicy( QwtPlotRasterItem::PaintCache );
    setRenderHint(QwtPlotItem::RenderAntialiased);
}


PlotRasterData::PlotRasterData()
{
    setInterval( Qt::XAxis, QwtInterval( -100, 100, QwtInterval::ExcludeMaximum ) );
    setInterval( Qt::YAxis, QwtInterval( -100, 100, QwtInterval::ExcludeMaximum ) );
    setInterval( Qt::ZAxis, QwtInterval( 0.1, 2.0 ) );
    setResampleMode( static_cast<QwtMatrixRasterData::ResampleMode>( 1 ) );     // Сглаживание
}


PlotZozPicker::PlotZozPicker( QWidget *canvas ): QwtPlotPicker( canvas )
{
    setAxis(QwtPlot::xBottom, QwtPlot::yLeft);
    setTrackerMode(QwtPlotPicker::AlwaysOn);
    setRubberBandPen(QColor(Qt::green));
    setRubberBand(QwtPicker::CrossRubberBand);
    setTrackerPen(QColor(Qt::black));
}

void PlotZozPicker::drawTracker( QPainter *painter )
{
    if(painter->isActive()) {
        QString text;
        text.sprintf("%.2f, %.2f",
                     trackerText(trackerPosition()).text().split(",",QString::SkipEmptyParts).at(0).toFloat(),
                     trackerText(trackerPosition()).text().split(",",QString::SkipEmptyParts).at(1).toFloat()
                     );
        emit position(text);
    }
}


PlotZozMarker::PlotZozMarker(const int LineStyle)
{
    if (QwtPlotMarker::VLine == LineStyle) {
        setLineStyle( QwtPlotMarker::VLine );
        setLabelAlignment( Qt::AlignLeft | Qt::AlignTop );
    }
    else if (QwtPlotMarker::HLine == LineStyle) {
        setLineStyle( QwtPlotMarker::HLine );
        setLabelAlignment(Qt::AlignRight | Qt::AlignTop);
    }
    setLinePen( Qt::red, 1, Qt::DotLine );
}

void PlotZozMarker::setLabelValue(const float Value, const QFont Font)
{
    if(lineStyle() == QwtPlotMarker::HLine)
        setValue( 0.0, Value );
    else if(lineStyle() == QwtPlotMarker::VLine)
        setValue( Value, 0.0 );

    QString strMark;
    strMark.sprintf("%.1f", Value);
    QwtText qwttMark(strMark);
    qwttMark.setColor(Qt::black);
    qwttMark.setFont(Font);
    setLabel(qwttMark);
}

void PlotZozMarker::setFont(const QFont Font)
{
    QwtText qwttMark(label());
    qwttMark.setFont(Font);
    setLabel(qwttMark);
}


PlotZoz::PlotZoz(QWidget *)
{
    // Прозрачность полотна
    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setFrameStyle(QFrame::NoFrame);
    canvas->setPaintAttribute(QwtPlotCanvas::BackingStore, false);
    canvas->setPaintAttribute(QwtPlotCanvas::Opaque, false);
    canvas->setAttribute(Qt::WA_OpaquePaintEvent, false);
    canvas->setAutoFillBackground( false );
    setCanvas(canvas);
    plotLayout()->setAlignCanvasToScales(true);

    // Отключаем щкалы деления
    for (int i=0; i<QwtPlot::axisCnt; i++) {
        axisScaleDraw(i)->enableComponent(QwtScaleDraw::Ticks, false);
        axisScaleDraw(i)->enableComponent(QwtScaleDraw::Backbone, false);
    }

    pltSpectrogram = new PlotSpectr();
    pltRasterData = new PlotRasterData();

    pltGrid = new QwtPlotGrid;
    pltGrid->enableXMin(true);       // разрешаем отображение линий сетки, соответствующих вспомогательным делениям нижней шкалы
    pltGrid->enableYMin(true);       // разрешаем отображение линий сетки, соответствующих вспомогательным делениям нижней шкалы
    pltGrid->setMajorPen(QPen(Qt::gray,0,Qt::DotLine));  // черный для основных делений
    pltGrid->setMinorPen(QPen(Qt::gray,0,Qt::DotLine));  // серый для вспомогательных
    pltGrid->attach(this);

    setAxisMaxMajor(0,20);
    setAxisMaxMinor(0,0);
    setAxisMaxMajor(1,20);
    setAxisMaxMinor(1,0);
    setAxisMaxMajor(2,20);
    setAxisMaxMinor(2,0);
    setAxisMaxMajor(3,20);
    setAxisMaxMinor(3,0);

    canvas->setCursor(Qt::ArrowCursor);
    setMouseTracking(true);
    for (int i=0; i<QwtPlot::axisCnt; i++)
        axisWidget(i)->setMouseTracking(true);
}


/* ------- Положение мыши ------- */
void PlotZoz::mouseMoveEvent(QMouseEvent *event)
{
    if ( !canvas()->geometry().contains(event->pos().x(), event->y()) )
        emit sgnlOutCanvas();
}


void PlotZoz::resizeEvent(QResizeEvent* ev)
{
    QFrame::resizeEvent( ev );
    updateLayout();
}


/* ------- Вкл/Выкл Оси ------- */
void PlotZoz::setAxisEnableForAll(bool b)
{
    for (int i=0; i<4; i++)
        enableAxis(i,b);
}


/* ------- Шрифт сетки ------- */
void PlotZoz::setAxisFontForAll(QFont fntAxis)
{
    for (int i=0; i<QwtPlot::axisCnt; i++)
        setAxisFont(i, fntAxis);
}


/* ------- Шрифт сетки ------- */
void PlotZoz::setLegendFont(QFont fntAxis)
{
    legend()->setFont(fntAxis);
}


/* ------- Масштаб Сетки ------- */
void PlotZoz::setAxisScaleForAll(float fSide, int numStep)
{
    for (int i=0; i<4; i++)
        setAxisScale(i,-fSide, fSide, numStep);
}


void PlotZoz::setAxisVisible(bool left, bool right, bool bottom, bool top)
{
    enableAxis(QwtPlot::yLeft, left);
    enableAxis(QwtPlot::yRight, right);
    enableAxis(QwtPlot::xBottom, bottom);
    enableAxis(QwtPlot::xTop, top);
}


void PlotZoz::setData(QVector<double> vecData, float fZoSide, QColor clrPlot, float fPen)
{
    pltRasterData->setValueMatrix( vecData, sqrt(vecData.size()) );
    pltSpectrogram->setRenderThreadCount( 0 ); // use system specific thread count
    pltSpectrogram->setData(pltRasterData);

    if (fZoSide != 0) {
        pltRasterData->setInterval( Qt::XAxis, QwtInterval( -fZoSide, fZoSide, QwtInterval::ExcludeMaximum ) );
        pltRasterData->setInterval( Qt::YAxis, QwtInterval( -fZoSide, fZoSide, QwtInterval::ExcludeMaximum ) );
    }
    pltSpectrogram->setDefaultContourPen(clrPlot, fPen, Qt::SolidLine);
    pltSpectrogram->attach( this );
}


void PlotZoz::setRastDatInterval(float minX, float maxX, float minY, float maxY)
{
    pltRasterData->setInterval( Qt::XAxis, QwtInterval( minX, maxX, QwtInterval::ExcludeMaximum ) );
    pltRasterData->setInterval( Qt::YAxis, QwtInterval( minY, maxY, QwtInterval::ExcludeMaximum ) );
}

 /* ------- Стиль ЗОЗ ------- */
void PlotZoz::setContourPen(QColor clrPlot, float fPen )
{
    pltSpectrogram->setDefaultContourPen(clrPlot, fPen, Qt::SolidLine);
}


/* ------- Видимость сетки ------- */
void PlotZoz::setGridVisible(const bool b)
{
    pltGrid->setVisible(b);
}


/* ------- Видимость сетки ------- */
bool PlotZoz::gridIsVisible()
{
    return pltGrid->isVisible();
}


void PlotZoz::setGridPen(const QPen pen)
{
    pltGrid->setPen(pen);
}

void PlotZoz::setGridStep(const int Step)
{               
    setAxisScale(QwtPlot::yLeft,
                 int(pltRasterData->interval(Qt::YAxis).minValue()),
                 int(pltRasterData->interval(Qt::YAxis).maxValue()),
                 Step );

    setAxisScale(QwtPlot::xBottom,
                 int(pltRasterData->interval(Qt::XAxis).minValue()),
                 int(pltRasterData->interval(Qt::XAxis).maxValue()),
                 Step );
}


double PlotZoz::zozValue(float x, float y)
{
    return pltRasterData->value(x,y);
}

float PlotZoz::valueXMax()
{
    int valueXMin (pltRasterData->interval(Qt::XAxis).minValue());
    int valueXMax (pltRasterData->interval(Qt::XAxis).maxValue());
    int valueYMin (pltRasterData->interval(Qt::YAxis).minValue());
    int valueYMax (pltRasterData->interval(Qt::YAxis).maxValue());

    for (float i = valueXMax; i > valueXMin; i-=0.1)
        for (float j = valueYMax; j > valueYMin; j-=0.1)
            if (zozValue(i,j) > 1)
                return i;
    return 0;
}

float PlotZoz::valueYMin()
{
    int valueXMin (pltRasterData->interval(Qt::XAxis).minValue());
    int valueXMax (pltRasterData->interval(Qt::XAxis).maxValue());
    int valueYMin (pltRasterData->interval(Qt::YAxis).minValue());
    int valueYMax (pltRasterData->interval(Qt::YAxis).maxValue());

    for (float i = valueYMin; i < valueYMax; i+=0.1)
        for (float j = valueXMin; j < valueXMax; j+=0.1)
            if (zozValue(j,i) > 1)
                return i;
    return 0;
}
