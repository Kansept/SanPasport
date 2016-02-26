#include "dialog_configgrid.h"
#include "ui_dialog_configgrid.h"
#include <QColorDialog>
#include <QDebug>

QColor g_colorGridColor;

Dialog_ConfigGrid::Dialog_ConfigGrid(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_ConfigGrid)
{
    ui->setupUi(this);

    connect( ui->pushButton_Ok, SIGNAL(clicked()), this, SLOT(saveSettings())  );
    connect( ui->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(close()) );
    connect( ui->toolButton_GridColor, SIGNAL(clicked()), this, SLOT(setGridColor()) );
}


Dialog_ConfigGrid::~Dialog_ConfigGrid()
{
    delete ui;
}


void Dialog_ConfigGrid::setGridColor()
{
    QString qsStyle = "background-color: ";
    g_colorGridColor = QColorDialog::getColor(Qt::red);
    if(g_colorGridColor.isValid())
    {
        qsStyle.append(g_colorGridColor.name());
        ui->toolButton_GridColor->setStyleSheet("");
        ui->toolButton_GridColor->setStyleSheet(qsStyle);
    }
}


void Dialog_ConfigGrid::loadSettings(ConfigSitPlan confZoConfig)
{
    g_colorGridColor = confZoConfig.GridPen.color();
    ui->toolButton_GridColor->setStyleSheet("");
    QString qsStyle = "background-color: ";
    qsStyle.append(confZoConfig.GridPen.color().name());
    ui->toolButton_GridColor->setStyleSheet(qsStyle);

    if(confZoConfig.GridPen.style() == 1) { ui->comboBox_PenStyle->setCurrentIndex(0); }
    if(confZoConfig.GridPen.style() == 2) { ui->comboBox_PenStyle->setCurrentIndex(1); }
    if(confZoConfig.GridPen.style() == 3) { ui->comboBox_PenStyle->setCurrentIndex(2); }

    ui->checkBox_VisibleGridAxisLeft->setChecked(confZoConfig.GridAxisVisibleLeft);
    ui->checkBox_VisibleGridAxisRight->setChecked(confZoConfig.GridAxisVisibleRight);
    ui->checkBox_VisibleGridAxisBottom->setChecked(confZoConfig.GridAxisVisibleBottom);
    ui->checkBox_VisibleGridAxisTop->setChecked(confZoConfig.GridAxisVisibleTop);
    ui->spinBox_GridSize->setValue(confZoConfig.GridSize);
    ui->spinBox_PenWidth->setValue(confZoConfig.GridPen.width());
    ui->spinBox_GridStep->setValue(confZoConfig.GridStep);
}


void Dialog_ConfigGrid::saveSettings()
{
        ConfigSitPlan zcZoConfig;

        zcZoConfig.GridAxisVisibleLeft = ui->checkBox_VisibleGridAxisLeft->isChecked();
        zcZoConfig.GridAxisVisibleRight = ui->checkBox_VisibleGridAxisRight->isChecked();
        zcZoConfig.GridAxisVisibleBottom = ui->checkBox_VisibleGridAxisBottom->isChecked();
        zcZoConfig.GridAxisVisibleTop = ui->checkBox_VisibleGridAxisTop->isChecked();
        zcZoConfig.GridSize = ui->spinBox_GridSize->value();
        zcZoConfig.GridStep = ui->spinBox_GridStep->value();

        QPen penGrid;
        penGrid.setWidthF(ui->spinBox_PenWidth->value());
        penGrid.setColor(g_colorGridColor);
        if(ui->comboBox_PenStyle->currentIndex() == 0) { penGrid.setStyle(Qt::SolidLine); }
        if(ui->comboBox_PenStyle->currentIndex() == 1) { penGrid.setStyle(Qt::DashLine); }
        if(ui->comboBox_PenStyle->currentIndex() == 2) { penGrid.setStyle(Qt::DashDotLine); }
        zcZoConfig.GridPen = penGrid;

        QVariant vclass = QVariant::fromValue(zcZoConfig);

    emit sendOkDialog_ConfigGrid(vclass);
    close();
}
