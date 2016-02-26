#include "projectionpkaemo4.h"

projectionPKAEMO4::projectionPKAEMO4()
{
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QString>
#include <QTextStream>
#include <QTextCodec>
#include <QMessageBox>
#include <QDebug>

using namespace std;

projectionPKAEMO4::projectionPKAEMO4(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

  sb1 = new QLabel(statusBar());
  sb2 = new QLabel(statusBar());

  ui->statusBar->setSizeGripEnabled(false);
  ui->statusBar->addWidget(sb1, 1);
  ui->statusBar->addWidget(sb2, 1);

  ui->toolButton_FlePath->setDisabled(true);

  sb1->setText("Files: ");
  sb2->setText("Lines: ");

    connect(ui->pushButton_Load,SIGNAL(clicked()),this,SLOT(openFiles()));
    connect(ui->pushButton_start, SIGNAL(clicked()), this, SLOT(unionFiles()));
    connect(ui->toolButton_FlePath, SIGNAL(clicked()), this, SLOT(setPath()));

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("cp-1251"));
}

projectionPKAEMO4::~MainWindow()
{
    delete ui;
}
// Выбор файлов
bool projectionPKAEMO4::openFiles()
{
    QStringList qslFiles = QFileDialog::getOpenFileNames(this,
                                    tr("Find Files"), QDir::currentPath());

    if (qslFiles.size()==0) {
        return false;
    } else {
        loadFiles(qslFiles);
        return true;
    }

}

// Выбор папки
bool projectionPKAEMO4::setPath() {

    QString qsDir = QFileDialog::getExistingDirectory(0,
                               tr("Open Directory"), "C:", QFileDialog::ShowDirsOnly
                               | QFileDialog::DontResolveSymlinks);
    if (qsDir == "") {
        return false;
    } else {
        ui->lineEdit->setText(qsDir);
        return true;
    }

}

// ------------------------------------------------Загрузка файла
void projectionPKAEMO4::loadFiles(QStringList qslFiles) {

    ui->tableWidget_files->clear();

    ui->tableWidget_files->setRowCount(qslFiles.size());

    for (int i = 0; i<qslFiles.size(); i++)
    {
        QFileInfo qfiF = qslFiles.at(i);
        QTableWidgetItem* iName = new QTableWidgetItem(qfiF.fileName());
        ui->tableWidget_files->setItem(i,0,iName);       // Название
        QTableWidgetItem* iPath = new QTableWidgetItem(qfiF.absolutePath());
        ui->tableWidget_files->setItem(i,1,iPath);       // Путь
    }
    ui->tableWidget_files->resizeColumnsToContents();
    ui->lineEdit->setText(ui->tableWidget_files->item(0,1)->text());



    ui->pushButton_start->setEnabled(true);
    ui->toolButton_FlePath->setEnabled(true);
}

//--------------Считывание i-ой строки из файла--------------------
QByteArray projectionPKAEMO4::strMaxItem(QByteArray qbaSrez, QByteArray qbaRead) {

    QByteArray result = qbaRead;

    QString strSrez = qbaSrez;
    QString strRead = qbaRead;

    QStringList strlstSrez = strSrez.split(" ", QString::SkipEmptyParts);
    QStringList strlstRead = strRead.split(" ", QString::SkipEmptyParts);

    if (2<=strlstSrez.size()) {

          if   ( strlstSrez.at(2).simplified().toDouble() > strlstRead.at(2).simplified().toDouble() ) {
              result = qbaSrez; }
          else {
              result = qbaRead; }
    }

    return result;
}

bool projectionPKAEMO4::unionFiles() {

    QTextCodec::setCodecForTr(QTextCodec::codecForName("CP-1251"));

    int iAmtFiles = (ui->tableWidget_files->rowCount()); // переменная - Количество Файлов

    QString *qsFiles = new QString[iAmtFiles];
    for(int i=0; i<iAmtFiles; i++){
        qsFiles[i] = ui->tableWidget_files->item(i,1)->text()+"\\"+ui->tableWidget_files->item(i,0)->text();
    } // переменная - Массив имён Файлов

//-------------------------------------- Проверка файла
    int iLines = 0;
    int iSumLines = 0;
    QString linestr;

    for (int iFiles=0; iFiles<iAmtFiles; iFiles++)   {

    QFile open_file(qsFiles[iFiles]);
    iLines = 0;

        if(open_file.open(QIODevice::ReadOnly)) {

            while(!open_file.atEnd()) {
                linestr = open_file.readLine();
                iLines++;
            } //конец while(!stream.atEnd())

            if (iFiles != 0) {

              if (iSumLines != iLines) {
                  QMessageBox::warning(this, tr("Error"), tr("Неправильный формат файлов!"));
                  sb1->setText("Files: "+(QString::number(iAmtFiles)) );    // Количество файлов
                  sb2->setText("Lines: Ощибка!" );
                  return false;

              } // конец if (iSumLines != iLines)

              if (iSumLines == iLines) {

                  sb1->setText("Files: "+(QString::number(iAmtFiles)) );    // Количество файлов
                  sb2->setText("Lines: "+(QString::number(iLines)) );       // Количество строк
              } // конец if (iSumLines != iLines)

            } // конец if (iFiles != 0)

                 iSumLines = iLines;

                 open_file.close();

            }   // конец if(open_file.open(QIODevice::ReadOnly))

    }   // конец  for (int iFiles=0; i<iAmtFiles; i++)

//--------------------------------------------------------
//-----------   Сохраняем файл  --------------------------
if (QFile (ui->lineEdit->text()+"\\srez.dat").exists()) {

    int ifileExists = QMessageBox::warning(0,
                                 "Предупреждение",
                                 "Файл проекции существует, "
                                 "\n заменить?",
                                 "Yes",
                                 "No",
                                 QString(),
                                 1,
                                 1
                                );
    if(ifileExists == 1) { return false; }


}

QFile (ui->lineEdit->text()+"\\srez.dat").remove();

QByteArray qbaSrez ="";
QByteArray qbaRead ="";
QByteArray qbaSum ="(_|_)";

int intStr = 0;

    for (int i=0; i<iAmtFiles; i++) {

        QFile fileSrez(ui->lineEdit->text()+"\\srez.dat");

        QFile fileTemp(ui->lineEdit->text()+"\\temp.dat");

        QFile fileRead(qsFiles[i]);

        if(fileSrez.open(QIODevice::ReadWrite) && fileRead.open(QIODevice::ReadOnly) && fileTemp.open(QIODevice::ReadWrite))   {

             intStr = 0;

             while(!fileRead.atEnd()) {

                if (i==0)  {

                    if (intStr == 0) {
                        qbaSrez = "Проекция\n";
                        fileSrez.write(qbaSrez);
                        qbaSrez = fileSrez.readLine();
                        qbaRead = fileRead.readLine();
                    }

                    if (intStr != 0) {
                        qbaRead = fileRead.readLine();
                        fileSrez.write(qbaRead);
                    }

                 }

                 if (i != 0) {

                     if (intStr == 0) {

                         qbaSrez = "Проекция\n";
                         fileTemp.write(qbaSrez);
                         qbaSrez = fileSrez.readLine();
                         qbaRead = fileRead.readLine();
                     }

                     if (intStr != 0) {

                         qbaSrez = fileSrez.readLine();
                         qbaRead = fileRead.readLine();

                         qbaSum = strMaxItem(qbaSrez, qbaRead);

                         fileTemp.write(qbaSum);

                     }

                  }

                  intStr=1;
          } // конец while(!fileRead.atEnd())

             fileSrez.close();
             fileTemp.close();
             fileRead.close();

         if (i != 0) {
            fileSrez.remove();
            fileTemp.rename(ui->lineEdit->text()+"\\srez.dat");
         } if (i == 0) {fileTemp.remove();}

         } // конец if(fileSrez.open(QIODevice::ReadWrite) && fileRead.open(QIODevice::ReadOnly))

    }  // конец  for (int i=0; i<iAmtFiles; i++)

delete[] qsFiles;

return true;

}
