#include "dialog_confizgraph.h"
#include "ui_dialog_confizgraph.h"

#include <QDebug>
#include <QShowEvent>


Dialog_ConfizGraph::Dialog_ConfizGraph(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_ConfizGraph)
{
    ui->setupUi(this);

    stngViewPlot = new QSettings((QCoreApplication::applicationDirPath()) + "//configSanPasport.ini",QSettings::IniFormat);

    connect( ui->pushButton_Ok, SIGNAL(clicked()), SLOT(accept()) );
    connect( ui->pushButton_Cancel, SIGNAL(clicked()), SLOT(reject()) );
    connect( ui->toolButton_GridColor, SIGNAL(clicked()), SLOT(gridColor()) );
    connect( ui->toolButton_ZozColor, SIGNAL(clicked()), SLOT(zozColor()) );

    ui->comboBox_GridStyle->addItems(QStringList() << "–––––––––––––" << "– – – – – – – – – "
                                                   << "·················· " << "– · – · – · – · – ·");
}


Dialog_ConfizGraph::~Dialog_ConfizGraph()
{
    delete ui;
}


void Dialog_ConfizGraph::showEvent(QShowEvent *event)
{
    if(!event->isAccepted()) {
        return; }

    stngViewPlot->beginGroup("ViewPlot");
    ui->toolButton_GridColor->colorBackGround( stngViewPlot->value("GridColor", "#7c7c7c").value<QColor>() );
    ui->lineEdit_GridStep->setText(stngViewPlot->value("GridStep", 0).toString());
    ui->comboBox_GridStyle->setCurrentIndex(stngViewPlot->value("GridStyle", 0).toInt());
    ui->doubleSpinBox_GridWidth->setValue(stngViewPlot->value("GridWidth", 1).toFloat());

    ui->toolButton_ZozColor->colorBackGround( stngViewPlot->value("ZozColor", "#FF0000").value<QColor>() );
    ui->doubleSpinBox_ZozWidth->setValue(stngViewPlot->value("ZozWidth", 1).toFloat());
    stngViewPlot->endGroup();
}


/* ------- Применить ------- */
void Dialog_ConfizGraph::accept()
{
    stngViewPlot->beginGroup("ViewPlot");
    stngViewPlot->setValue( "GridColor", ui->toolButton_GridColor->currentColor().name() );
    stngViewPlot->setValue( "GridStep", ui->lineEdit_GridStep->text() );
    stngViewPlot->setValue( "GridStyle", QString::number(ui->comboBox_GridStyle->currentIndex()) );
    stngViewPlot->setValue( "GridWidth", QString::number(ui->doubleSpinBox_GridWidth->value()) );

    stngViewPlot->setValue( "ZozColor", ui->toolButton_ZozColor->currentColor().name() );
    stngViewPlot->setValue( "ZozWidth", QString::number(ui->doubleSpinBox_ZozWidth->value()) );
    stngViewPlot->endGroup();
    stngViewPlot->sync();

    done(Accepted);
}


/* ------- Цвет сетки ------- */
void Dialog_ConfizGraph::gridColor()
{
    ui->toolButton_GridColor->setColor();
}


/* ------- Цвет ЗОЗ ------- */
void Dialog_ConfizGraph::zozColor()
{
    ui->toolButton_ZozColor->setColor();
}
