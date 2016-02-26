#include "dialogconvert.h"
#include "ui_dialogconvert.h"
#include <math.h>

DialogConvert::DialogConvert(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConvert)
{
    ui->setupUi(this);

    connect(ui->lineEdit_inWatt, SIGNAL(textChanged(QString)), this, SLOT(wattToDBm(QString)) );
    connect(ui->lineEdit_inDBm, SIGNAL(textChanged(QString)), this, SLOT(dBmToWatt(QString)) );
    connect(ui->lineEdit_inFreq, SIGNAL(textChanged(QString)), this, SLOT(freqToDlvoln(QString)) );
    connect(ui->lineEdit_inDlvoln, SIGNAL(textChanged(QString)), this, SLOT(dlvolnToFreq(QString)) );
}


DialogConvert::~DialogConvert()
{
    delete ui;
}


void DialogConvert::on_pushButton_clicked()
{
    close();
}


/* ------- Ватт в dBm ------- */
void DialogConvert::wattToDBm(QString arg1)
{
    ui->lineEdit_outDBm->setText(QString::number(10*log10(arg1.toFloat()/0.001)));
}


/* ------- dBm в Ватт ------- */
void DialogConvert::dBmToWatt(QString arg1)
{
    ui->lineEdit_outWatt->setText(QString::number(0.001*pow(10,(arg1.toFloat()/10))));
}

/* ------- Частота в длинну волны ------- */
void DialogConvert::freqToDlvoln(QString arg1)
{
    ui->lineEdit_outDlvoln->setText(QString::number((3*pow(10,8))/(arg1.toFloat()*pow(10,6))));
}


/* ------- Длинну волны в частоту ------- */
void DialogConvert::dlvolnToFreq(QString arg1)
{
    ui->lineEdit_outFreq->setText(QString::number((3*pow(10,8))/(arg1.toFloat()*pow(10,6))));
}
