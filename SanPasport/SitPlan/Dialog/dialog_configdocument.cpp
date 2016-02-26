#include "dialog_configdocument.h"
#include "ui_dialog_configdocument.h"


Dialog_ConfigDocument::Dialog_ConfigDocument(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_ConfigDocument)
{
    ui->setupUi(this);

    connect( ui->pushButton_Ok, SIGNAL(clicked()), this, SLOT(saveSettings()) );
    connect( ui->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(close()) );
}

Dialog_ConfigDocument::~Dialog_ConfigDocument()
{
    delete ui;
}


void Dialog_ConfigDocument::loadSettings(ConfigSitPlan confPageConfig)
{
   ui->comboBox_PageSize->setCurrentText(confPageConfig.PaperSize);
   ui->doubleSpinBox_PageDpi->setValue(confPageConfig.PaperDpi);

   if (confPageConfig.PaperOrientation == "Landscape") {
          ui->radioButton_PageLandscape->setChecked(true);
          ui->radioButton_PagePortrait->setChecked(false);
    } else {
          ui->radioButton_PageLandscape->setChecked(false);
          ui->radioButton_PagePortrait->setChecked(true);
    }
   ui->spinBox_PageMargin->setValue(confPageConfig.PaperMargin);
}


void Dialog_ConfigDocument::saveSettings()
{
        ConfigSitPlan zcPageConfig;

        zcPageConfig.PaperSize = ui->comboBox_PageSize->currentText();
        zcPageConfig.PaperDpi = ui->doubleSpinBox_PageDpi->value();
        zcPageConfig.PaperMargin = ui->spinBox_PageMargin->value();

        if(ui->radioButton_PageLandscape->isChecked()) {
            zcPageConfig.PaperOrientation = "Landscape";
        } else {
            zcPageConfig.PaperOrientation = "Portrait";
        }

        QVariant vclass = QVariant::fromValue(zcPageConfig);

    emit sendOkDialog_ConfigPage(vclass);
    close();
}
