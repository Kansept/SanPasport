#include "dialog_taskzo.h"
#include "ui_dialog_taskzo.h"
#include <QRegExp>
#include <QDebug>
#include <QSettings>

#include "project.h"


Task g_taskZoz;

Dialog_TaskZo::Dialog_TaskZo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_TaskZo)
{
    ui->setupUi(this);

    connect( ui->pushButton_Ok, SIGNAL(clicked()), this, SLOT(pushButton_Ok()) );
    connect( ui->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(pushButton_Cancel()) );

    ui->lineEdit_Height->validatorSpase(true);

}

Dialog_TaskZo::~Dialog_TaskZo()
{
    delete ui;
}


void Dialog_TaskZo::showEvent(QShowEvent *event)
{
    if(event->isAccepted()) {
        clearUI();
        g_taskZoz.clear();
    }
}


void Dialog_TaskZo::pushButton_Ok()
{
    QString qsSendZo;
    QStringList qslHeight;
    Task tskZo;

    qslHeight = ui->lineEdit_Height->text().split(" ", QString::SkipEmptyParts);

    for(int i = 0; i < qslHeight.size(); i++)
    {
        qsSendZo.append( ui->lineEdit_Xmin->text() ).append(";");                   // 0
        qsSendZo.append( ui->lineEdit_Xmax->text() ).append(";");                   // 1
        qsSendZo.append( ui->lineEdit_Xd->text() ).append(";");                     // 2
        qsSendZo.append( ui->lineEdit_Ymin->text() ).append(";");                   // 3
        qsSendZo.append( ui->lineEdit_Ymax->text() ).append(";");                   // 4
        qsSendZo.append( ui->lineEdit_Yd->text() ).append(";");                     // 5
        qsSendZo.append( QString::number(qslHeight.at(i).toFloat()) ).append(";");  // 6
        if(ui->radioButton_Graphics->isChecked())                                   // 7
            qsSendZo.append( "0" );
        else if(ui->radioButton_Text->isChecked())
            qsSendZo.append( "3" );
        else
            qsSendZo.append( "0" );


        tskZo.Id = g_taskZoz.Id;
        tskZo.Sort = g_taskZoz.Sort;
        tskZo.Enabled = g_taskZoz.Enabled;
        tskZo.Type = 1;
        tskZo.Params = qsSendZo;

        Project cbd;
        cbd.addTask(tskZo);
        emit sendTaskZo();

        qsSendZo.clear();
        tskZo.clear();
    }
    close();
}


void Dialog_TaskZo::pushButton_Cancel()
{
    close();
}


void Dialog_TaskZo::clearUI()
{
    g_taskZoz.Id = -1;
    QSettings *stngZo = new QSettings((QCoreApplication::applicationDirPath()) + "//configSanPasport.ini",QSettings::IniFormat);

    stngZo->beginGroup("SanPasport");
    ui->lineEdit_Xmin->setText( stngZo->value("ZoXmin", "-100").toString() );
    ui->lineEdit_Xmax->setText( stngZo->value("ZoXmax", "100").toString() );
    ui->lineEdit_Xd->setText( stngZo->value("ZoXd", "1").toString() );
    ui->lineEdit_Ymin->setText( stngZo->value("ZoYmin", "-100").toString() );
    ui->lineEdit_Ymax->setText( stngZo->value("ZoYmax", "100").toString() );
    ui->lineEdit_Yd->setText( stngZo->value("ZoYd", "1").toString() );
    ui->lineEdit_Height->setText("0");
    stngZo->endGroup();
    ui->radioButton_Graphics->setChecked(true);

    delete stngZo;
}


void Dialog_TaskZo::insertData(Task tsk)
{
    clearUI();
    QStringList stlTask(tsk.Params.split(";"));

    ui->lineEdit_Xmin->setText( stlTask.at(0) );
    ui->lineEdit_Xmax->setText( stlTask.at(1) );
    ui->lineEdit_Xd->setText( stlTask.at(2) );
    ui->lineEdit_Ymin->setText( stlTask.at(3) );
    ui->lineEdit_Ymax->setText( stlTask.at(4) );
    ui->lineEdit_Yd->setText( stlTask.at(5) );
    ui->lineEdit_Height->setText( stlTask.at(6) );
    if(stlTask.at(7) == "0")
        ui->radioButton_Graphics->setChecked(true);
    else if(stlTask.at(7) == "3")
        ui->radioButton_Text->setChecked(true);
    else
        ui->radioButton_Graphics->setChecked(true);

    g_taskZoz.Id = tsk.Id;
    g_taskZoz.Sort = tsk.Sort;
    g_taskZoz.Enabled = tsk.Enabled;
}

void Dialog_TaskZo::insertHeight(const QString strZ)
{
    ui->lineEdit_Height->setText(strZ);
}
