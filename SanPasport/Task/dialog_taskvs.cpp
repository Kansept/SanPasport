#include "dialog_taskvs.h"
#include "ui_dialog_taskvs.h"

#include "project.h"


Task g_taskVS;

Dialog_TaskVs::Dialog_TaskVs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_TaskVs)
{
    ui->setupUi(this);

    connect( ui->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(pushButton_Cancel()) );
    connect( ui->pushButton_Ok, SIGNAL(clicked()), this, SLOT(pushButton_Ok()) );

    ui->lineEdit_Azimut->validatorSpase(true);
}

Dialog_TaskVs::~Dialog_TaskVs()
{
    delete ui;
}


void Dialog_TaskVs::pushButton_Cancel()
{
    close();
}

void Dialog_TaskVs::pushButton_Ok()
{
    QString qsSendVs;
    QStringList qslnAzimut(ui->lineEdit_Azimut->text().split(" ", QString::SkipEmptyParts));
    Task tskVs;

    for(int i = 0; i < qslnAzimut.size(); i++)
    {
        qsSendVs.append( ui->lineEdit_Xpos->text() ).append(";");                       // 0
        qsSendVs.append( ui->lineEdit_Ypos->text() ).append(";");                       // 1
        qsSendVs.append( ui->lineEdit_Rmin->text() ).append(";");                       // 2
        qsSendVs.append( ui->lineEdit_Rmax->text() ).append(";");                       // 3
        qsSendVs.append( ui->lineEdit_Rd->text() ).append(";");                         // 4
        qsSendVs.append( ui->lineEdit_Hmin->text() ).append(";");                       // 5
        qsSendVs.append( ui->lineEdit_Hmax->text() ).append(";");                       // 6
        qsSendVs.append( ui->lineEdit_Hd->text() ).append(";");                         // 7
        qsSendVs.append( QString::number(qslnAzimut.at(i).toFloat()) ).append(";");     // 8
        qsSendVs.append( "0" );

        tskVs.Id = g_taskVS.Id;
        tskVs.Number = g_taskVS.Number;
        tskVs.Enabled = g_taskVS.Enabled;
        tskVs.Type = 2;
        tskVs.Params = qsSendVs;

        Project cbd;
        cbd.addTask(tskVs);
        emit sendTaskVs();

        qsSendVs.clear();
        tskVs.clear();
    }

    close();
}


void Dialog_TaskVs::showEvent(QShowEvent *event)
{
    if(event->isAccepted())
    {
        clearUI();
        g_taskVS.clear();
    }
}


void Dialog_TaskVs::clearUI()
{
    g_taskVS.Id = -1;
    QSettings *stngVs = new QSettings((QCoreApplication::applicationDirPath()) + "//configSanPasport.ini",QSettings::IniFormat);

    stngVs->beginGroup("SanPasport");
    ui->lineEdit_Xpos->setText("0");
    ui->lineEdit_Ypos->setText("0");
    ui->lineEdit_Rmin->setText( stngVs->value("VsRmin", "0").toString() );
    ui->lineEdit_Rmax->setText( stngVs->value("VsRmax", "100").toString() );
    ui->lineEdit_Rd->setText( stngVs->value("VsRd", "0.5").toString() );
    ui->lineEdit_Hmin->setText( stngVs->value("VsHmin", "0").toString() );
    ui->lineEdit_Hmax->setText( stngVs->value("VsHmax", "40").toString() );
    ui->lineEdit_Hd->setText( stngVs->value("VsHd", "0.5").toString() );
    ui->lineEdit_Azimut->setText("0");
    stngVs->endGroup();

    delete stngVs;
}


void Dialog_TaskVs::insertData(Task tsk)
{
    clearUI();
    QStringList stlTask(tsk.Params.split(";"));

    ui->lineEdit_Xpos->setText( stlTask.at(0) );
    ui->lineEdit_Ypos->setText( stlTask.at(1) );
    ui->lineEdit_Rmin->setText( stlTask.at(2) );
    ui->lineEdit_Rmax->setText( stlTask.at(3) );
    ui->lineEdit_Rd->setText( stlTask.at(4) );
    ui->lineEdit_Hmin->setText( stlTask.at(5) );
    ui->lineEdit_Hmax->setText( stlTask.at(6) );
    ui->lineEdit_Hd->setText( stlTask.at(7) );
    ui->lineEdit_Azimut->setText( stlTask.at(8) );

    g_taskVS.Id = tsk.Id;
    g_taskVS.Number = tsk.Number;
    g_taskVS.Enabled = tsk.Enabled;
}
