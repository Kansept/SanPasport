#include "dialog_preferens.h"
#include "ui_dialog_preferens.h"

#include <QDebug>
#include <QFile>
#include <QFileDialog>

Dialog_Preferens::Dialog_Preferens(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_Preferens)
{
    ui->setupUi(this);

    stngSanPasport = new QSettings((QCoreApplication::applicationDirPath()) + "//configSanPasport.ini",QSettings::IniFormat);

    connect( ui->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(close()) );
    connect( ui->pushButton_Ok, SIGNAL(clicked()), this, SLOT(writeSetting()) );
    connect( ui->pushButton_Ok, SIGNAL(clicked()), this, SLOT(close()) );
    connect( ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(changePage(QListWidgetItem*,QListWidgetItem*)) );
    connect( ui->toolButton_PathDb, SIGNAL(clicked()), SLOT(dBPathChange()) );
    readSetting();
}


Dialog_Preferens::~Dialog_Preferens()
{
    delete ui;
}


void Dialog_Preferens::showEvent(QShowEvent *event)
{
    if(event->isAccepted()) {
        readSetting();
    }
}

void Dialog_Preferens::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current) {
        current = previous; }

    ui->stackedWidget->setCurrentIndex(ui->listWidget->row(current));
}

void Dialog_Preferens::dBPathChange()
{
    QString qsDir = QFileDialog::getExistingDirectory(0,
                           tr("Открыть каталог"), "C:", QFileDialog::ShowDirsOnly
                           | QFileDialog::DontResolveSymlinks);
    if (qsDir != "") {
        ui->lineEdit_DbPath->setText(qsDir); }
}

/* ------- Чтение настроек ------- */
void Dialog_Preferens::readSetting()
{
    stngSanPasport->beginGroup("SanPasport");
    ui->dotlineEdit_ZoXmin->setText( stngSanPasport->value("ZoXmin", "-100").toString() );
    ui->dotlineEdit_ZoXmax->setText( stngSanPasport->value("ZoXmax", "100").toString() );
    ui->dotlineEdit_ZoXd->setText( stngSanPasport->value("ZoXd", "1").toString() );
    ui->dotlineEdit_ZoYmin->setText( stngSanPasport->value("ZoYmin", "-100").toString() );
    ui->dotlineEdit_ZoYmax->setText( stngSanPasport->value("ZoYmax", "100").toString() );
    ui->dotlineEdit_ZoYd->setText( stngSanPasport->value("ZoYd", "1").toString() );
    ui->dotlineEdit_VsRmin->setText( stngSanPasport->value("VsRmin", "0").toString() );
    ui->dotlineEdit_VsRmax->setText( stngSanPasport->value("VsRmax", "100").toString() );
    ui->dotlineEdit_VsRd->setText( stngSanPasport->value("VsRd", "0.5").toString() );
    ui->dotlineEdit_VsHmin->setText( stngSanPasport->value("VsHmin", "0").toString() );
    ui->dotlineEdit_VsHmax->setText( stngSanPasport->value("VsHmax", "40").toString() );
    ui->dotlineEdit_VsHd->setText( stngSanPasport->value("VsHd", "0.5").toString() );

    ui->lineEdit_DbPath->setText( stngSanPasport->value("DbPath", "").toString() );
    stngSanPasport->endGroup();
}


/* ------- Запись настроек ------- */
void Dialog_Preferens::writeSetting()
{
    stngSanPasport->beginGroup("SanPasport");
    stngSanPasport->setValue("ZoXmin", ui->dotlineEdit_ZoXmin->text());
    stngSanPasport->setValue("ZoXmax", ui->dotlineEdit_ZoXmax->text());
    stngSanPasport->setValue("ZoXd", ui->dotlineEdit_ZoXd->text());
    stngSanPasport->setValue("ZoYmin", ui->dotlineEdit_ZoYmin->text());
    stngSanPasport->setValue("ZoYmax", ui->dotlineEdit_ZoYmax->text());
    stngSanPasport->setValue("ZoYd", ui->dotlineEdit_ZoYd->text());
    stngSanPasport->setValue("VsRmin", ui->dotlineEdit_VsRmin->text());
    stngSanPasport->setValue("VsRmax", ui->dotlineEdit_VsRmax->text());
    stngSanPasport->setValue("VsRd", ui->dotlineEdit_VsRd->text());
    stngSanPasport->setValue("VsHmin", ui->dotlineEdit_VsHmin->text());
    stngSanPasport->setValue("VsHmax", ui->dotlineEdit_VsHmax->text());
    stngSanPasport->setValue("VsHd", ui->dotlineEdit_VsHd->text());

    stngSanPasport->setValue("DbPath", ui->lineEdit_DbPath->text());
    stngSanPasport->endGroup();
    stngSanPasport->sync();

    emit dbPath(ui->lineEdit_DbPath->text());
}
