#include "viewplot.h"
#include "ui_viewplot.h"

#include <math.h>
#include <QBuffer>
#include <QClipboard>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontDialog>
#include <QMimeData>
#include <QSvgGenerator>
#include <QImageWriter>
#include <QLabel>
#include <QResizeEvent>
#include <QTimer>
#include <QGraphicsEffect>

#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_dyngrid_layout.h>
#include <qwt_legend_label.h>
#include <qwt_legend.h>
#include <qwt_scale_div.h>
#include <qwt_plot.h>


ViewPlot::ViewPlot(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ViewPlot)
{
    ui->setupUi(this);

    this->setWindowFlags( Qt::Dialog |  Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint |
                          Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
    ui->centralwidget->setStyleSheet("background-color: white;");
    stngViewPlot = new QSettings((QCoreApplication::applicationDirPath()) + "//configSanPasport.ini",QSettings::IniFormat);

    // Строка состояния
    sb1 = new QLabel(statusBar());
    ui->statusbar->setSizeGripEnabled(false);
    ui->statusbar->addWidget(sb1, 1);
    sb1->setText("");

    connect( ui->action_Open, SIGNAL(triggered()), SLOT(plotOpen()) );
    connect( ui->action_FileExport, SIGNAL(triggered()), SLOT(plotExport())  );
    connect( ui->action_CopyToBuffer, SIGNAL(triggered()), SLOT(copyToBuffer()) );
    connect( ui->action_Exit, SIGNAL(triggered()), SLOT(close()) );
    connect( ui->action_ConfizGraph, SIGNAL(triggered()), SLOT(showConfizGraph()) );
    connect( ui->action_Font, SIGNAL(triggered()), SLOT(settingFont()) );
    connect( ui->frame, SIGNAL(sgnlRazmer()), SLOT(plotSquare()) );

    // Подписи к осям
    ui->qwtPlot_spectrogram->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtScaleDraw::Labels, false);
    ui->qwtPlot_spectrogram->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtScaleDraw::Labels, false);
    ui->qwtPlot_spectrogram->setAxisEnableForAll(true);
    // Позиция курсора
    picker = new PlotZozPicker( ui->qwtPlot_spectrogram->canvas() );
    connect( picker, SIGNAL(position(QString)), SLOT(plotPos(QString)) );
    connect( ui->qwtPlot_spectrogram, SIGNAL(sgnlOutCanvas()), SLOT(plotPosReset())  );
    plotPosReset();
    // Кривая для легенды
    curve = new QwtPlotCurve();
    curve->setPen( Qt::red, 2 );
    curve->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );
    curve->setLegendIconSize(QSize(30,2));
    curve->attach(ui->qwtPlot_spectrogram);
    // Легенда
    qwtLeg = new QwtLegend();
    qwtLeg->contentsWidget()->layout()->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->qwtPlot_spectrogram->insertLegend( qwtLeg, QwtPlot::RightLegend);
    // Маркеры
    markerV = new PlotZozMarker(QwtPlotMarker::VLine);
    markerH = new PlotZozMarker(QwtPlotMarker::HLine);
    // Костыль - чтоб не мигал ратянутый график
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
    effect->setOpacity(0);
    ui->frame->setGraphicsEffect(effect);

    // Контекстное меню
    ui->frame->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( ui->frame, SIGNAL(customContextMenuRequested(QPoint)), SLOT(contextMenuView(QPoint)) );
}


ViewPlot::~ViewPlot()
{
    delete ui;
}

void ViewPlot::showEvent(QShowEvent *ev)
{
    if(ev->isAccepted())
        QTimer::singleShot( 100, this, SLOT(showPlot()) );
}


void ViewPlot::showPlot()
{
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect;
    effect->setOpacity(1);
    ui->frame->setGraphicsEffect(effect);
    delete effect;
}


/* ------- Контекстное меню ------- */
void ViewPlot::contextMenuView(const QPoint &pos)
{
    QMenu menuContext;
    menuContext.addAction(ui->action_FileExport);
    menuContext.addSeparator();
    menuContext.addAction(ui->action_CopyToBuffer);
    menuContext.addAction(ui->action_ConfizGraph);
    menuContext.addAction(ui->action_Font);
    menuContext.exec( ui->frame->mapToGlobal(pos) );
}


/* ------- Сохранение соотношения сторпон графика ------- */
void ViewPlot::plotSquare()
{
    int numWidth (ui->qwtPlot_spectrogram->canvas()->width());
    int numHeight (ui->qwtPlot_spectrogram->canvas()->height());
    QRect recFrame (ui->frame->geometry());
    QRect rec(ui->qwtPlot_spectrogram->geometry());
    int pol;

    ui->qwtPlot_spectrogram->setGeometry(recFrame);

    if(numWidth > numHeight)
        rec.setWidth( rec.width() - (numWidth - numHeight));
    else
        rec.setHeight( rec.height() - (numHeight - numWidth));

    ui->qwtPlot_spectrogram->setGeometry(rec);

    // Центрирование
    rec = ui->qwtPlot_spectrogram->geometry();

    pol = (recFrame.width()-rec.width())/2;
    rec.setX( rec.x() + pol - 2 );
    rec.setWidth( rec.width() + pol );

    pol = (recFrame.height()-rec.height())/2;
    rec.setY( rec.y() + pol - 2);
    rec.setHeight( rec.height() + pol );

    ui->qwtPlot_spectrogram->setGeometry(rec);
}


void ViewPlot::plotPosReset()
{
    ui->label->setText(" ");
}


void ViewPlot::plotPos(QString strPosition)
{
    QStringList strlPos = strPosition.split(",",QString::SkipEmptyParts);
    float Xpos,Ypos,R,Angle;

    Xpos = strlPos.at(0).toFloat();
    Ypos = strlPos.at(1).toFloat();

    QString strPDU;
    strPDU.sprintf("%.2f", ui->qwtPlot_spectrogram->zozValue(Xpos, Ypos));

    R = sqrt(pow(Xpos,2) + pow(Ypos,2));
    if(Xpos != 0)
    {
        Angle = (atan2(Ypos,Xpos)*180)/3.14;
        Angle = -(Angle - 90);

        if(Angle < 0)
            Angle = Angle + 360;
    } else
        Angle = 0;

    QString strR,strAzimut;
    strR.sprintf("%.2f", R);
    strAzimut.sprintf("%.2f", Angle);

    if( QFileInfo(sb1->text()).fileName().left(2).toLower() == "vs" )
        ui->label->setText( "R = " + QString::number(Xpos) + " м; H = " +  QString::number(Ypos) + " м" + "; ПДУ = " + strPDU );
    else if(QFileInfo(sb1->text()).fileName().left(2).toLower() == "zo")
        ui->label->setText( "X = " + QString::number(Xpos) + " м; Y =" + QString::number(Ypos) + " м; R = " +
                           strR + " м; Аз.-т = " +strAzimut + " град.; ПДУ = " + strPDU );
}


/* ------- Показать диалог настроек графика ЗОЗ ------- */
void ViewPlot::showConfizGraph()
{
    dlg_ConfizGraph = new Dialog_ConfizGraph(this);

    if(dlg_ConfizGraph->exec() == QDialog::Accepted)  {
        settingRead(); }
}


/* ------- Открыть файл ЗОЗ ------- */
void ViewPlot::plotOpen()
{
    QStringList qslf = QFileDialog::getOpenFileNames(this, tr("Открыть фаил Расчета"), "", tr("ZO support files (*.dat);;Text file (*.txt);;All files (*)"));
    if(qslf.isEmpty()) { return; }
    plotLoadZo(qslf.first());
}


/* ------- Загрузка ЗОЗ ------- */
void ViewPlot::plotLoadZo(const QString f)
{
    ZO zoZona;
    zoZona.readZoz(f);
    ploting(zoZona);
    sb1->setText(f);

}


/* ------- Экспорт ЗОЗ ------- */
void ViewPlot::plotExport()
{
    QStringList filter;
    filter << "Image (*.jpg)" << "Image (*.png)" << "PDF Documents (*.pdf)" << "SVG Documents (*.svg)" << "Postscript Documents (*.ps)" ;
    QString fileName = QFileDialog::getSaveFileName(this, "Export File Name", "", filter.join( ";;" ), NULL, QFileDialog::DontConfirmOverwrite );

    if ( !fileName.isEmpty() ) {
        QwtPlotRenderer renderer;
        renderer.renderDocument( ui->qwtPlot_spectrogram, fileName, QSizeF( 300, 300 ), 85 );
    }
}


/* ------- Рисование графика ЗОЗ ------- */
void ViewPlot::ploting(ZO val)
{
    // Легенда
    if(val.Type == "zo")
        curve->setTitle("ЗО. h = " + val.Height + " м");
    if(val.Type == "vs")
        curve->setTitle("Аз-т. " + val.Height + " град.");

    // Шаг сетки
    ui->qwtPlot_spectrogram->setAxisScale(QwtPlot::xBottom, int(val.MinX), int(val.MaxX), (abs(int(val.MinX)) + abs(int(val.MaxX)))/10 );
    ui->qwtPlot_spectrogram->setAxisScale(QwtPlot::yLeft, int(val.MinY), int(val.MaxY), (abs(int(val.MinY)) + abs(int(val.MaxY)))/10 );
    // Ось X
    QwtText qwttTopLabel;
    if(val.Type == "vs")
        qwttTopLabel.setText("H, м");
    else
        qwttTopLabel.setText("Y, м");
    qwttTopLabel.setRenderFlags(Qt::AlignLeft);
    ui->qwtPlot_spectrogram->setAxisTitle(QwtPlot::xTop, qwttTopLabel);
    // Ось Y
    QwtText qwttRightLabel;
    if(val.Type == "vs")
        qwttRightLabel.setText("R, м");
    else
        qwttRightLabel.setText("X, м");

    qwttRightLabel.setRenderFlags(Qt::AlignLeft | Qt::AlignLeft);
    ui->qwtPlot_spectrogram->setAxisTitle(QwtPlot::yRight, qwttRightLabel);
    ui->qwtPlot_spectrogram->axisWidget(QwtPlot::yRight)->setAlignment(QwtScaleDraw::BottomScale);
    ui->qwtPlot_spectrogram->axisWidget(QwtPlot::yRight)->setFixedWidth(qwtLeg->geometry().width());
    ui->qwtPlot_spectrogram->axisWidget(QwtPlot::yRight)->setContentsMargins(5,0,0,0);

    // Вносим данные в график
    ui->qwtPlot_spectrogram->setData(val.Values);
    ui->qwtPlot_spectrogram->setRastDatInterval(val.MinX, val.MaxX, val.MinY, val.MaxY);

    settingRead();

    QFont fnt = stngViewPlot->value("ViewPlot/GridFont", "Arial").value<QFont>();

    if(val.Type == "vs") {
        markerH->show();
        markerH->attach( ui->qwtPlot_spectrogram );
        markerH->setLabelValue(ui->qwtPlot_spectrogram->valueYMin(),fnt);
        markerV->show();
        markerV->attach( ui->qwtPlot_spectrogram );
        markerV->setLabelValue(ui->qwtPlot_spectrogram->valueXMax(), fnt);
    } else {
        markerH->hide();
        markerV->hide();
    }

    ui->qwtPlot_spectrogram->replot();
    plotSquare();
    QRect rectFrame ( ui->frame->geometry() );
    rectFrame.setX(rectFrame.x() + 1);
    ui->frame->setGeometry(rectFrame);
}


/* ------- Копировать в буфер ЗОЗ ------- */
void ViewPlot::copyToBuffer()
{
    QBuffer b;
    QClipboard *clipboard = QApplication::clipboard();
    QSvgGenerator p;
    p.setOutputDevice(&b);
    p.setSize(QSize(800,800));
    p.setViewBox(QRect(0,0,800,800));

    QPainter painter;
    QwtPlotRenderer renderer;

    painter.begin(&p);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(ui->qwtPlot_spectrogram,&painter,QRect(0,0,800,800));
    painter.end();

    QMimeData * d = new QMimeData();
    d->setData("image/svg+xml",b.buffer());
    d->setData("image/svg+xml-compressed", b.buffer());
    d->setImageData(QPixmap::grabWidget(ui->qwtPlot_spectrogram, 0,0,-1,-1));

    clipboard->setMimeData(d,QClipboard::Clipboard);
}


/* ------- Шрифт ------- */
void ViewPlot::settingFont()
{
    bool bOk;
    QFont fnt = QFontDialog::getFont( &bOk, stngViewPlot->value("ViewPlot/GridFont", QFont("Arial")).value<QFont>() );

    if(bOk) {
        stngViewPlot->setValue( "ViewPlot/GridFont", fnt );
        // Оси
        ui->qwtPlot_spectrogram->setAxisFontForAll(fnt);
        // Легенда
        QwtText crvTitle (curve->title());
        crvTitle.setFont(fnt);
        curve->setTitle(crvTitle);
        // Координаты
        ui->label->setFont(fnt);
        // Маркеры
        markerH->setFont(fnt);
        markerV->setFont(fnt);
    }
    ui->qwtPlot_spectrogram->replot();
    plotSquare();
    QRect rectFrame ( ui->frame->geometry() );
    rectFrame.setX(rectFrame.x() + 1);
    ui->frame->setGeometry(rectFrame);
}


/* ------- прочитать настройки ------- */
void ViewPlot::settingRead()
{
    stngViewPlot->beginGroup("ViewPlot");

    // ЗОЗ
    ui->qwtPlot_spectrogram->setContourPen( stngViewPlot->value("ZozColor", "#ff0000").value<QColor>(),
                                            stngViewPlot->value("ZozWidth", 1).toFloat() );
    // Сетка
    QPen penGrid;
    penGrid.setColor( stngViewPlot->value("GridColor", "#7c7c7c").value<QColor>() );
    penGrid.setWidth( stngViewPlot->value("GridWidth", 1).toInt() );

    // Шрифт
    QFont fnt;
    fnt = stngViewPlot->value("GridFont", "Arial").value<QFont>();
    // Оси
    ui->qwtPlot_spectrogram->setAxisFontForAll(fnt);
    // Легенда
    QwtText crvTitle (curve->title());
    crvTitle.setFont(fnt);
    curve->setTitle(crvTitle);
    // Координаты
    ui->label->setFont(fnt);
    // Маркеры
    markerH->setFont(fnt);
    markerV->setFont(fnt);

    if(stngViewPlot->value("GridStyle", 0).toInt() == 0)
        penGrid.setStyle(Qt::SolidLine);
    else if(stngViewPlot->value("GridStyle", 0).toInt() == 1)
        penGrid.setStyle(Qt::DashLine);
    else if(stngViewPlot->value("GridStyle", 0).toInt() == 2)
        penGrid.setStyle(Qt::DotLine);
    else
        penGrid.setStyle(Qt::DashDotDotLine);

    ui->qwtPlot_spectrogram->setGridPen(penGrid);
    stngViewPlot->endGroup();
}
