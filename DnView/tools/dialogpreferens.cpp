#include "dialogpreferens.h"
#include "ui_dialogpreferens.h"

#include <QSettings>
#include <QDir>
#include <qcolordialog.h>
#include <QColor>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

QColor qcColorHoriz;
QColor qcColorVert;

DialogPreferens::DialogPreferens(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPreferens)
{
    ui->setupUi(this);

    settingsPref = new QSettings((QCoreApplication::applicationDirPath()) + "//configDnView.ini",QSettings::IniFormat);

    connect(ui->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(close()));                 // Закрыть
    connect(ui->pushButton_Ok, SIGNAL(clicked()), this, SLOT(action_pushButton_Ok()));      // Применить

    QString qsStyle;

    settingsPref->beginGroup("settings_DnView");
         qcColorHoriz = settingsPref->value("ColorHorizDN", "#00ff00").value<QColor>();
         qcColorVert = settingsPref->value("ColorVertDN", "#ff0000").value<QColor>();
         int nTemplate = settingsPref->value("DnViewTemplate", 0).toInt();
    settingsPref->endGroup();

    qsStyle = "background-color: ";
    ui->toolButton_ColorHorizDN->setStyleSheet( qsStyle.append(qcColorHoriz.name()) );

    qsStyle = "background-color: ";
    ui->toolButton_ColorVertDN->setStyleSheet( qsStyle.append(qcColorVert.name()) );

    QSettings settings_Inegr ("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
    if (settings_Inegr.value("/*/shell/ViewDN/command/.").toString() != NULL)  {ui->checkBox_ContextMenu->setChecked(true);}
         else {ui->checkBox_ContextMenu->setChecked(false);}
    if (settings_Inegr.value("ViewDN.file/shell/open/command/.").toString() != NULL)  {ui->checkBox_fileAssociation->setChecked(true);}
         else {ui->checkBox_fileAssociation->setChecked(false);}

    // Выбор списка шаблонов
    ui->comboBox_Template->addItem("Полярные графики");
    ui->comboBox_Template->addItem("Декартовые графики");

    ui->comboBox_Template->setCurrentIndex(nTemplate);

}


DialogPreferens::~DialogPreferens()
{
    delete ui;
}


void DialogPreferens::on_toolButton_ColorHorizDN_clicked()
{
    QString qsStyle = "background-color: ";
    qcColorHoriz = QColorDialog::getColor(Qt::white);
    if(qcColorHoriz.isValid()) {
        qsStyle.append(qcColorHoriz.name());
        ui->toolButton_ColorHorizDN->setStyleSheet(qsStyle);
    }
}


void DialogPreferens::on_toolButton_ColorVertDN_clicked()
{
    QString qsStyle = "background-color: ";
    qcColorVert = QColorDialog::getColor(Qt::white);
    if(qcColorVert.isValid()) {
        qsStyle.append(qcColorVert.name());
        ui->toolButton_ColorVertDN->setStyleSheet(qsStyle);
    }
}


void DialogPreferens::action_pushButton_Ok()
{
    // ------ Ассоциировать с pln ----- //
    if(ui->checkBox_fileAssociation->isChecked()) {
       QSettings setting_fileAssociation_yes ("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
       setting_fileAssociation_yes.setValue (".pln/.", "ViewDN.file");
       setting_fileAssociation_yes.setValue ("ViewDN.file/.", tr("Antenna Radiation Diagram"));
       setting_fileAssociation_yes.setValue ("ViewDN.file/shell/open/command/.",
                          "\"" + QDir::toNativeSeparators (QCoreApplication::applicationFilePath()) + "\"" + " \"%1\"");
       setting_fileAssociation_yes.sync();
    }
    // ------- Удалить ассоциации с pln ------- //
    if( !(ui->checkBox_fileAssociation->isChecked()) ) {
          QSettings setting_fileAssociation_no ("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
          setting_fileAssociation_no.remove (".pln");
          setting_fileAssociation_no.remove ("ViewDN.file");
          setting_fileAssociation_no.sync();
    }
    // ----- Добавить в контекстного меню ----- //
    if(ui->checkBox_ContextMenu->isChecked()) {
        QSettings setting_ContextMenu_yes ("HKEY_CLASSES_ROOT\\*\\shell\\ViewDN", QSettings::NativeFormat);
      //  setting_ContextMenu_yes.setValue ("Icon", (QCoreApplication::applicationDirPath()) + "//DNView.exe");
        setting_ContextMenu_yes.setValue ("command/.", "\"" + QDir::toNativeSeparators (QCoreApplication::applicationFilePath()) + "\"" + " \"%1\"");
        setting_ContextMenu_yes.sync();
    }
    // ----- Удалить из контекстного меню ----- //
    if( !(ui->checkBox_ContextMenu->isChecked()) ) {
        QSettings setting_ContextMenu_no ("HKEY_CLASSES_ROOT\\*\\shell", QSettings::NativeFormat);
        setting_ContextMenu_no.remove ("ViewDN");
        setting_ContextMenu_no.sync();
    }
    settingsPref->beginGroup("settings_DnView");
        if(qcColorHoriz != ""){ settingsPref->setValue("ColorHorizDN", qcColorHoriz.name()); }
        if(qcColorVert != "") { settingsPref->setValue("ColorVertDN", qcColorVert.name()); }
        settingsPref->setValue("DnViewTemplate", ui->comboBox_Template->currentIndex());
    settingsPref->endGroup();

    emit sendPrefOk();

    close();
}
