#include "dialog_parametrs.h"
#include "ui_dialog_parametrs.h"

Dialog_Parametrs::Dialog_Parametrs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_Parametrs)
{
    ui->setupUi(this);
 // Кнопка Отмена
     connect( ui->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(close()) );
 // Кнопка Ок
     connect( ui->pushButton_Ok, SIGNAL(clicked()), this, SLOT(setParam()) );
}

Dialog_Parametrs::~Dialog_Parametrs()
{
    delete ui;
}

void Dialog_Parametrs::loadParam(const float koef)
{
    ui->doubleSpinBox_Koef->setValue(koef);
}


void Dialog_Parametrs::setParam()
{
    QString qsSendOptions;
    qsSendOptions.clear();
    qsSendOptions.append(ui->doubleSpinBox_Koef->text());

    SendOptionsFromDlgOption(qsSendOptions);

    close();
}
