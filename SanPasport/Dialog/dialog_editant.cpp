#include "dialog_editant.h"
#include "ui_dialog_editant.h"

#include "qdata.h"

#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>

#include <qwt_scale_engine.h>

QModelIndex g_qsIdTable;
const int N1 = 361;     // Размер массива данных ДН

int GraphScale = 25;    // Масштаб графика

Antenna g_adnEdit;
Antenna g_adnEditNoEdit;

Dialog_EditAnt::Dialog_EditAnt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_EditAnt)
{
    ui->setupUi(this);

    // ------- создаем сетку ------- //
    gridPolar = new QwtPolarGrid();

    for (int sc=0; sc < QwtPolar::ScaleCount; sc++)     // для всех шкал
    {
        gridPolar->showMinorGrid(sc);    // разрешаем отображение линий сетки, соответствующих вспомогательным делениям шкалы
        // назначаем цвет, толщину и стиль для линий сетки для
        gridPolar->setMajorGridPen(sc,QPen(Qt::lightGray));                  // основных делений
        gridPolar->setMinorGridPen(sc,QPen(Qt::lightGray,0,Qt::DotLine));    // и вспомогательных
    }

    gridPolar->showAxis(QwtPolar::AxisRight,false);       // запрещаем отображать радиальную шкалу справа
    gridPolar->showAxis(QwtPolar::AxisTop,true);          // разрешаем отображать радиальную шкалу сверху
    gridPolar->attach(ui->PlotPolar);

    connect( ui->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(close()) );

    connect( ui->lineEdit_PowerPRD, SIGNAL(textChanged(QString)), this, SLOT(powerCalc()) );
    connect( ui->lineEdit_PRD, SIGNAL(textChanged(QString)), this, SLOT(powerCalc()) );
    connect( ui->lineEdit_LeghtAFT, SIGNAL(textChanged(QString)), this, SLOT(powerCalc()) );
    connect( ui->lineEdit_Loss, SIGNAL(textChanged(QString)), this, SLOT(powerCalc()) );
    connect( ui->lineEdit_KSVN, SIGNAL(textChanged(QString)), this, SLOT(powerCalc()) );
    connect( ui->lineEdit_LossOther, SIGNAL(textChanged(QString)), this, SLOT(powerCalc()) );

    connect( ui->toolButton_DnCw, SIGNAL(clicked()), this, SLOT(dnCw()) );
    connect( ui->toolButton_DnCcw, SIGNAL(clicked()), this, SLOT(dnCcw()) );
    connect( ui->toolButton_DnMirror, SIGNAL(clicked()), this, SLOT(dnMirror()) );
    connect( ui->toolButton_DnSymmetryTop, SIGNAL(clicked()), this, SLOT(dnSymmetryTop()) );
    connect( ui->toolButton_DnSymmetryBottom, SIGNAL(clicked()), this, SLOT(dnSymmetryBottom()) );
    connect( ui->toolButton_DnPreload, SIGNAL(clicked()), this, SLOT(dnPreload()) );
    connect( ui->toolButton_DnReset, SIGNAL(clicked()), this, SLOT(dnReset()) );
}


Dialog_EditAnt::~Dialog_EditAnt()
{
    delete ui;
}


void Dialog_EditAnt::on_pushButton_Ok_clicked()
{
    getDataForm();
    sendEditAntFromDlgEdit(g_adnEdit);
    g_adnEdit.clear();

    close();
}


// ----------------------------------- Открываем форму считываем данные ----------------------------------- //
void Dialog_EditAnt::insertDataForm(Antenna adnEdit)
{
    g_adnEdit.clear();
    g_adnEditNoEdit = adnEdit;
    g_adnEdit = adnEdit;

    this->setWindowTitle( "Параметры технического средства: " + adnEdit.Name );

    ui->lineEdit_Name->setText( adnEdit.Name );
    ui->lineEdit_Owner->setText( adnEdit.Owner );
    ui->lineEdit_Freq->setText( QString::number(adnEdit.Frequency) );
    ui->lineEdit_Gain->setText( QString::number(adnEdit.Gain) );
    ui->lineEdit_Height->setText( QString::number(adnEdit.Height) );
    ui->lineEdit_PowerPRD->setText( QString::number(adnEdit.PowerTrx) );
    ui->lineEdit_PRD->setText( QString::number(adnEdit.CountTrx) );
    ui->lineEdit_LeghtAFT->setText( QString::number(adnEdit.FeederLeght) );
    ui->lineEdit_Loss->setText( QString::number(adnEdit.FeederLoss) );
    ui->lineEdit_KSVN->setText( QString::number(adnEdit.KSVN) );
    ui->lineEdit_LossOther->setText( QString::number(adnEdit.LossOther) );
    ui->lineEdit_X->setText( QString::number(adnEdit.X) );
    ui->lineEdit_Y->setText( QString::number(adnEdit.Y) );
    ui->lineEdit_Z->setText( QString::number(adnEdit.Z) );
    ui->lineEdit_Az->setText( QString::number(adnEdit.Azimut) );
    ui->lineEdit_ElTilt->setText( QString::number(adnEdit.Tilt) );

    ui->lineEdit_PowerTotal->setText( QString::number(adnEdit.PowerTrx) );

    powerCalc();
    initPolarGraph(adnEdit);

    if(adnEdit.Type == 15) {
        ui->groupBox_Dn->setEnabled(false); }
    else {
        ui->groupBox_Dn->setEnabled(true); }
}


// ----------------------------------- Прочитать данные с формы ----------------------------------- //
void Dialog_EditAnt::getDataForm()
{
    g_adnEdit.Name = ui->lineEdit_Name->text();
    g_adnEdit.Owner = ui->lineEdit_Owner->text();
    g_adnEdit.Frequency = ui->lineEdit_Freq->text().toFloat();
    g_adnEdit.Gain = ui->lineEdit_Gain->text().toFloat();;
    g_adnEdit.Height = ui->lineEdit_Height->text().toFloat();
    g_adnEdit.PowerTotal = ui->lineEdit_PowerTotal->text().toFloat();
    g_adnEdit.PowerTrx = ui->lineEdit_PowerPRD->text().toFloat();
    g_adnEdit.CountTrx = ui->lineEdit_PRD->text().toFloat();
    g_adnEdit.FeederLeght = ui->lineEdit_LeghtAFT->text().toFloat();
    g_adnEdit.FeederLoss = ui->lineEdit_Loss->text().toFloat();
    g_adnEdit.KSVN = ui->lineEdit_KSVN->text().toFloat();
    g_adnEdit.LossOther = ui->lineEdit_LossOther->text().toFloat();
    g_adnEdit.X = ui->lineEdit_X->text().toFloat();
    g_adnEdit.Y = ui->lineEdit_Y->text().toFloat();
    g_adnEdit.Z = ui->lineEdit_Z->text().toFloat();
    g_adnEdit.Azimut = ui->lineEdit_Az->text().toFloat();
    g_adnEdit.Tilt = ui->lineEdit_ElTilt->text().toFloat();
}


// ----------------------------------- Инициализация полярного графика ----------------------------------- //
void Dialog_EditAnt::initPolarGraph(Antenna dan)
{
    dan.AzimutHorizontal[360]=360;
    dan.AzimutVertical[360]=360;
    dan.RadHorizontal[360]=dan.RadHorizontal[0];
    dan.RadVertical[360]=dan.RadVertical[0];

    ui->PlotPolar->detachItems(0,1);

    QColor qcHorizDN = "#ff0000";
    QColor qcVertDN = "#00ff00";

    double marker3db[361];
    for (int i=0; i<=360; i++)
        marker3db[i]=-3;

    gridPolar = new QwtPolarGrid();                            // создаем сетку

    for (int sc=0; sc < QwtPolar::ScaleCount; sc++)     // для всех шкал
    {
        gridPolar->showMinorGrid(sc);            // разрешаем отображение линий сетки, соответствующих вспомогательным делениям шкалы
        gridPolar->setMajorGridPen(sc,QPen(Qt::lightGray));                  // основных делений
        gridPolar->setMinorGridPen(sc,QPen(Qt::lightGray,0,Qt::DotLine));    // и вспомогательных
    }

    // markers
    QwtPolarMarker *marker = new QwtPolarMarker();
    marker->setPosition( QwtPointPolar( 269.5, -3 ) );
    marker->setSymbol( new QwtSymbol( QwtSymbol::HLine, QBrush( Qt::transparent ), QPen( Qt::green ), QSize( 0, 0 ) ) );
    marker->setLabelAlignment( Qt::AlignLeft | Qt::AlignTop );
    QwtText text( "-3" );
    text.setColor( Qt::black );
    QColor bg( Qt::white );
    bg.setAlpha( 200 );
    text.setBackgroundBrush( QBrush( bg ) );
    marker->setLabel( text );
    marker->attach( ui->PlotPolar );

    gridPolar->showAxis(QwtPolar::AxisRight,false);       // запрещаем отображать радиальную шкалу справа
    gridPolar->showAxis(QwtPolar::AxisTop,true);          // разрешаем отображать радиальную шкалу сверху

    gridPolar->attach(ui->PlotPolar);                                 // связываем созданную сетку с графиком

    ui->PlotPolar->setScale(QwtPolar::Radius,-GraphScale,0,1);        // радиальной шкалы
    ui->PlotPolar->setScale(QwtPolar::Azimuth,360,0,30);              // и азимутальной

    CurvPolarVert = new QwtPolarCurve(QString("Vertical"));           // создаем первую кривую с наименованием "DNA 1"
    CurvPolarVert->setPen(QPen(qcVertDN,2));                          // назначаем толщину 2 и цвет прорисовки - темно-зеленый

    CurvPolarHoriz = new QwtPolarCurve(QString("Horizontal"));        // создаем вторую кривую с наименованием "DNA 2"
    CurvPolarHoriz->setPen(QPen(qcHorizDN,2));                        // назначаем цвет прорисовки - красный

    CurvPolar3db = new QwtPolarCurve(QString("Marker3db"));           // создаем вторую кривую с наименованием "DNA 2"
    CurvPolar3db->setPen(QPen(Qt::red,0));                            // назначаем цвет прорисовки - красный
    CurvPolar3db->setPen(QPen(Qt::DashDotLine));
    CurvPolar3db->setStyle(QwtPolarCurve::Lines);
    CurvPolar3db->setData(new QData(dan.AzimutHorizontal,marker3db,N1));       // передаем кривым подготовленные данные
    CurvPolar3db->attach(ui->PlotPolar);

    CurvPolarVert->setData(new QData(dan.AzimutVertical,dan.RadVertical,N1));     // передаем кривым подготовленные данные
    CurvPolarHoriz->setData(new QData(dan.AzimutHorizontal,dan.RadHorizontal,N1));  // передаем кривым подготовленные данные

    CurvPolarHoriz->attach(ui->PlotPolar);
    CurvPolarVert->attach(ui->PlotPolar);

    ui->PlotPolar->replot();
}


// ----------------------------------- Вычислить мощность ----------------------------------- //
void Dialog_EditAnt::powerCalc()
{

    g_adnEdit.PowerTrx = ui->lineEdit_PowerPRD->text().toFloat();
    g_adnEdit.CountTrx = ui->lineEdit_PRD->text().toFloat();
    g_adnEdit.FeederLeght = ui->lineEdit_LeghtAFT->text().toFloat();
    g_adnEdit.FeederLoss = ui->lineEdit_Loss->text().toFloat();
    g_adnEdit.KSVN = ui->lineEdit_KSVN->text().toFloat();
    g_adnEdit.LossOther = ui->lineEdit_LossOther->text().toFloat();

    ui->lineEdit_PowerTotal->setText( QString::number(g_adnEdit.calcPower()) );
}


// ----------------------------------- Трансформация ДН ----------------------------------- //
void Dialog_EditAnt::dnCw()
{
    g_adnEdit.rotate(1,1);
    initPolarGraph(g_adnEdit);
}
void Dialog_EditAnt::dnCcw()
{
    g_adnEdit.rotate(-1,-1);
    initPolarGraph(g_adnEdit);
}
void Dialog_EditAnt::dnMirror()
{
    g_adnEdit.mirorrHorizontal();
    g_adnEdit.mirorrsVertical();
    initPolarGraph(g_adnEdit);
}
void Dialog_EditAnt::dnSymmetryTop()
{
    g_adnEdit.symmetryTop(1,1);
    initPolarGraph(g_adnEdit);
}
void Dialog_EditAnt::dnSymmetryBottom()
{
    g_adnEdit.symmetryBottom(1,1);
    initPolarGraph(g_adnEdit);
}
void Dialog_EditAnt::dnPreload()
{
    g_adnEdit.preloadPatternVert();
    initPolarGraph(g_adnEdit);
}
void Dialog_EditAnt::dnReset()
{
    for(int i = 0; i < 360; i++)
    {
        g_adnEdit.RadHorizontal[i] = g_adnEditNoEdit.RadHorizontal[i];
        g_adnEdit.RadVertical[i] = g_adnEditNoEdit.RadVertical[i];
    }
    initPolarGraph(g_adnEdit);
}
