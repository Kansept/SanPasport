#include "dialog_taskpoint.h"
#include "ui_dialog_taskpoint.h"
#include "project.h"


Task g_taskPoint;

Dialog_TaskPoint::Dialog_TaskPoint(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_TaskPoint)
{
    ui->setupUi(this);
    clearUI();


}

Dialog_TaskPoint::~Dialog_TaskPoint()
{
    delete ui;
}

void Dialog_TaskPoint::on_pushButton_Ok_clicked()
{
    Task tskPoint;

    QString qsSendPoint;
    qsSendPoint.append( ui->lineEdit_X->text() ).append(";");
    qsSendPoint.append( ui->lineEdit_Y->text() ).append(";");
    qsSendPoint.append( ui->lineEdit_Z->text() );

    tskPoint.Id = g_taskPoint.Id;
    tskPoint.Number = g_taskPoint.Number;
    tskPoint.Enabled = g_taskPoint.Enabled;
    tskPoint.Type = 3;
    tskPoint.Data = qsSendPoint;

    Project cbd;
    cbd.addTask(tskPoint);
    emit sendTaskPoint();

    clearUI();
    tskPoint.clear();
    close();
}

void Dialog_TaskPoint::on_pushButton_Cancel_clicked()
{
    clearUI();
    close();
}

void Dialog_TaskPoint::clearUI()
{
    g_taskPoint.Id = -1;
    ui->lineEdit_X->setText("0");
    ui->lineEdit_Y->setText("0");
    ui->lineEdit_Z->setText("0");
}


void Dialog_TaskPoint::insertData(Task tsk)
{
    clearUI();
    QStringList stlTask(tsk.Data.split(";"));

    ui->lineEdit_X->setText( stlTask.at(0) );
    ui->lineEdit_Y->setText( stlTask.at(1) );
    ui->lineEdit_Z->setText( stlTask.at(2) );

    g_taskPoint.Id = tsk.Id;
    g_taskPoint.Number = tsk.Number;
    g_taskPoint.Enabled = tsk.Enabled;

}
