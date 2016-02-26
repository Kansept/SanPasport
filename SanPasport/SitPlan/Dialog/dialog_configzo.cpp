#include "dialog_configzo.h"
#include "ui_dialog_configzo.h"

#include <QColorDialog>
#include <QDebug>

QColor g_colorZoColor;

Dialog_ConfigZo::Dialog_ConfigZo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_ConfigZo)
{
    ui->setupUi(this);

    connect( ui->toolButton_ZoColor, SIGNAL(clicked()), this, SLOT(setZoColor()) );
    connect( ui->pushButton_Ok, SIGNAL(clicked()), this, SLOT(saveSettings()) );
    connect( ui->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(close()) );
}


Dialog_ConfigZo::~Dialog_ConfigZo()
{
    delete ui;
}


void Dialog_ConfigZo::setZoColor()
{
       QString qsStyle = "background-color: ";
       g_colorZoColor = QColorDialog::getColor(Qt::red);
       if(g_colorZoColor.isValid()) {
           qsStyle.append(g_colorZoColor.name());
           ui->toolButton_ZoColor->setStyleSheet("");
           ui->toolButton_ZoColor->setStyleSheet(qsStyle);
       }
}


void Dialog_ConfigZo::loadSettings(ConfigSitPlan confZoConfig)
{
    g_colorZoColor = confZoConfig.ZoColor;

    ui->doubleSpinBox_ZoWidth->setValue(confZoConfig.ZoPenWidth);

    ui->toolButton_ZoColor->setStyleSheet("");
    QString qsStyle = "background-color: ";
    qsStyle.append(confZoConfig.ZoColor.name());
    ui->toolButton_ZoColor->setStyleSheet(qsStyle);

    ui->comboBox_ZoScale->setCurrentText(confZoConfig.ZoScale);
    ui->checkBox_ZoSmooth->setChecked(confZoConfig.ZoSmooth);
}


void Dialog_ConfigZo::saveSettings()
{
        ConfigSitPlan zcZoConfig;

        zcZoConfig.ZoPenWidth = ui->doubleSpinBox_ZoWidth->value();
        zcZoConfig.ZoColor = g_colorZoColor.name();
        zcZoConfig.ZoScale = ui->comboBox_ZoScale->currentText();
        zcZoConfig.ZoSmooth = ui->checkBox_ZoSmooth->isChecked();

        QVariant vclassZO = QVariant::fromValue(zcZoConfig);

    emit sendOkDialog_ConfigZo(vclassZO);
    close();
}
