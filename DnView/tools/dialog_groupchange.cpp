#include "dialog_groupchange.h"
#include "ui_dialog_groupchange.h"
#include "prto.h"

#include <QFileDialog>
#include <QDebug>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QMessageBox>

Dialog_GroupChange::Dialog_GroupChange(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_GroupChange)
{
    ui->setupUi(this);
    connect( ui->checkBox_Replase, SIGNAL(toggled(bool)), this, SLOT(setReplase(bool)) );
    connect( ui->toolButton_AddPatterns, SIGNAL(clicked()), this, SLOT(openFiles()) );
    connect( ui->pushButton_PathSave, SIGNAL(clicked()), this, SLOT(setChangeDir()) );
    connect( ui->toolButton_Clear, SIGNAL(clicked()), this, SLOT(clearData()) );
    connect( ui->pushButton_Start, SIGNAL(clicked()), this, SLOT(startConvert()) );
    connect( ui->toolButton_DeletePatterns, SIGNAL(clicked()), this, SLOT(removePatterns()) );

    ui->checkBox_Replase->setChecked(false);
    ui->pushButton_Start->setEnabled(false);
    ui->pushButton_PathSave->setEnabled(false);
    ui->checkBox_Replase->setEnabled(false);
    ui->tableWidget_Patterns->horizontalHeader()->setVisible(true);

    QStringList qslTWHeader;
    qslTWHeader.append("Имя");
    qslTWHeader.append("Путь");

    ui->tableWidget_Patterns->setColumnCount(qslTWHeader.count());
    ui->tableWidget_Patterns->setHorizontalHeaderLabels(qslTWHeader);

    setAcceptDrops(true);
    ui->toolButton_AddPatterns->setFocus();
}


Dialog_GroupChange::~Dialog_GroupChange()
{
    delete ui;
}


// --------------------------------------------------- Реализация Drag'n'Drop --------------------------------------------------- //
void Dialog_GroupChange::dragEnterEvent(QDragEnterEvent* event) {
    if(event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}


// --------------------------------------------------- Реализация Drag'n'Drop --------------------------------------------------- //
void Dialog_GroupChange:: dropEvent(QDropEvent* event) {
   QList<QUrl> urls = event->mimeData()->urls();
   if(urls.isEmpty())
       return;

   QStringList strlstFiles;
   for(int i=0; i < urls.size(); i++) { strlstFiles.append(urls.at(i).toLocalFile()); }

   if(strlstFiles.isEmpty())
       return;
   else
       loadFiles(strlstFiles);
}

void Dialog_GroupChange::showEvent(QShowEvent *event)
{
    if(event->isAccepted())
    {
        clearData();
    }
}


// --------------------------------------------------- Открыть файлы --------------------------------------------------- //
bool Dialog_GroupChange::openFiles()
{   
    QStringList qslFiles = QFileDialog::getOpenFileNames(this,
                                                         tr("Загрузить диаграммы направленности"), "",
                                                         tr("Patterns (*.msi);;Text file (*.txt);;All files (*)")
                                                         );
    if(qslFiles.isEmpty()) { return false; }

    else { loadFiles(qslFiles); return true; }
}


// --------------------------------------------------- Загрузить файлы --------------------------------------------------- //
void Dialog_GroupChange::loadFiles(const QStringList qslFiles)
{
    int intRow = ui->tableWidget_Patterns->rowCount();
    ui->tableWidget_Patterns->setRowCount(qslFiles.size()+intRow);

    for (int i = 0; i < qslFiles.size(); i++) {
        QFileInfo qfiF = qslFiles.at(i);

        QTableWidgetItem* iName = new QTableWidgetItem(qfiF.fileName());
        ui->tableWidget_Patterns->setItem(i+intRow,0,iName);       // Название

        QTableWidgetItem* iPath = new QTableWidgetItem(qfiF.absolutePath());
        ui->tableWidget_Patterns->setItem(i+intRow,1,iPath);       // Путь

    }
    ui->tableWidget_Patterns->resizeColumnsToContents();

    ui->pushButton_Start->setEnabled(true);
    ui->pushButton_PathSave->setEnabled(true);
    ui->checkBox_Replase->setEnabled(true);
}


// --------------------------------------------------- Удалить диаграмму ----------------------------------------------- //
void Dialog_GroupChange::removePatterns()
{
       ui->tableWidget_Patterns->removeRow(ui->tableWidget_Patterns->selectionModel()->currentIndex().row());
}


// --------------------------------------------------- Сменить директорию ----------------------------------------------- //
bool Dialog_GroupChange::setChangeDir()
{
    QString qsDir = QFileDialog::getExistingDirectory(0,
                           tr("Open Directory"), "C:", QFileDialog::ShowDirsOnly
                           | QFileDialog::DontResolveSymlinks);
    if (qsDir == "") {
        return false;
    } else {
        ui->lineEdit_Path->setText(qsDir);
        return true;
    }
}


// --------------------------------------------------- Заменить файлы --------------------------------------------------- //
void Dialog_GroupChange::setReplase(const bool b)
{
    if(b == true)
    {
       ui->pushButton_PathSave->setEnabled(false);
       ui->lineEdit_Path->setEnabled(false);
    }
    else
    {
        ui->pushButton_PathSave->setEnabled(true);
        ui->lineEdit_Path->setEnabled(true);
    }
}


// --------------------------------------------------- Очистить форму --------------------------------------------------- //
void Dialog_GroupChange::clearData()
{
    ui->tableWidget_Patterns->clearContents();
    ui->tableWidget_Patterns->setRowCount(0);
    ui->lineEdit_Path->setText("");
    ui->checkBox_Replase->setChecked(false);
    ui->pushButton_Start->setEnabled(false);
    ui->pushButton_PathSave->setEnabled(false);
    ui->checkBox_Replase->setEnabled(false);
}


// --------------------------------------------------- Старт конвертации --------------------------------------------------- //
void Dialog_GroupChange::startConvert()
{
    if(ui->checkBox_Replase->isChecked() == false && ui->lineEdit_Path->text() == "") {
        QMessageBox::warning(this, tr("Error"), tr("Выберите папку назначения!"));
        return;
    }

    Prto pattern;
    QStringList strlstFiles;
    for(int i=0; i < ui->tableWidget_Patterns->rowCount(); i++) {
       strlstFiles.append( ui->tableWidget_Patterns->item(i,1)->text()+"/"+ui->tableWidget_Patterns->item(i,0)->text() );
       pattern.clear();

       pattern.loadMsi(strlstFiles.at(i));

       if(ui->checkBox_Replase->isChecked() == true) {
          pattern.saveAsMsi(ui->tableWidget_Patterns->item(i,1)->text()+"/"+ui->tableWidget_Patterns->item(i,0)->text());
       } else {
          pattern.saveAsMsi(ui->lineEdit_Path->text()+"/"+ui->tableWidget_Patterns->item(i,0)->text().split(".").first()+".msi");
       }
    }
    QMessageBox::information(this, tr("Сообщение"), tr("Успешно завершено!"));
}


