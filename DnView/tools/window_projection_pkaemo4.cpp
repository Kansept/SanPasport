#include "window_projection_pkaemo4.h"
#include "ui_window_projection_pkaemo4.h"

#include <QFileDialog>
#include <QString>
#include <QTextStream>
#include <QTextCodec>
#include <QMessageBox>
#include <QDebug>
#include <QTableWidget>

using namespace std;

Window_projection_PKAEMO4::Window_projection_PKAEMO4(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window_projection_PKAEMO4)
{
    ui->setupUi(this);

    sb0 = new QLabel(statusBar());
    sb1 = new QLabel(statusBar());
    sb2 = new QLabel(statusBar());

    ui->statusbar->setSizeGripEnabled(false);
    ui->statusbar->addWidget(sb0, 1);
    ui->statusbar->addWidget(sb1, 1);
    ui->statusbar->addWidget(sb2, 1);

    ui->toolButton_FlePath->setDisabled(true);

    sb0->setText("");
    sb1->setText("Файлов: ");
    sb2->setText("Cтрок: ");

    connect( ui->pushButton_Load,SIGNAL(clicked()),this,SLOT(openFiles()) );
    connect( ui->pushButton_start, SIGNAL(clicked()), this, SLOT(unionFiles()) );
    connect( ui->toolButton_FlePath, SIGNAL(clicked()), this, SLOT(setPath()) );
    connect( ui->pushButton_Clear, SIGNAL(clicked()), this, SLOT(clearData()) );

    ui->tableWidget_files->horizontalHeader()->setVisible(true);

    QStringList qslTWHeader;
    qslTWHeader.append("Имя");
    qslTWHeader.append("Путь");

    ui->tableWidget_files->setColumnCount(qslTWHeader.count());
    ui->tableWidget_files->setHorizontalHeaderLabels(qslTWHeader);

    ui->lineEdit->setReadOnly(true);

    clearData();
}


Window_projection_PKAEMO4::~Window_projection_PKAEMO4()
{
    delete ui;
}



// --------------------------- Очистить данные ---------------------------
void Window_projection_PKAEMO4::clearData() {
    ui->tableWidget_files->clearContents();
    ui->tableWidget_files->setRowCount(0);
    ui->lineEdit->setText("");
    ui->pushButton_start->setEnabled(false);
    ui->toolButton_FlePath->setEnabled(false);
}


// --------------------------- Выбор файлов ---------------------------
bool Window_projection_PKAEMO4::openFiles()
{
    QStringList qslFiles = QFileDialog::getOpenFileNames(this,
                                    tr("Find Files"), QDir::currentPath());

    if (qslFiles.size()==0) {
        return false;
    } else {
        loadFiles(qslFiles);
        return true;
    }

    QStringList qslTWHeader;
    qslTWHeader.append("Имя");
    qslTWHeader.append("Путь");

    ui->tableWidget_files->setColumnCount(qslTWHeader.count());
    ui->tableWidget_files->setHorizontalHeaderLabels(qslTWHeader);
    ui->tableWidget_files->repaint();
}


// --------------------------- Выбор пути сохраняемого файла ---------------------------
bool Window_projection_PKAEMO4::setPath()
{
    QString qsSaveFilePath = QFileDialog::getSaveFileName(this,
                                                 tr("Сохранить как"), "projection",
                                                 tr("ПКАЭМО (*.dat)"));
    if (qsSaveFilePath == "") {
        return false;
    } else {
        ui->lineEdit->setText(qsSaveFilePath);
        return true;
    }
}


// ----------------------------------- Загрузка файла -----------------------------------
void Window_projection_PKAEMO4::loadFiles(QStringList qslFiles)
{
    clearData();
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
    ui->lineEdit->setText(ui->tableWidget_files->item(0,1)->text()+"/projection.dat");
    ui->pushButton_start->setEnabled(true);
    ui->toolButton_FlePath->setEnabled(true);
}


// ----------------------------------- Считывание i-ой строки из файла -----------------------------------
QByteArray Window_projection_PKAEMO4::strMaxItem(QByteArray qbaSrez, QByteArray qbaRead)
{
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


// ----------------------------------- Объединение -----------------------------------
bool Window_projection_PKAEMO4::unionFiles() {

    int iAmtFiles = (ui->tableWidget_files->rowCount()); // переменная - Количество Файлов
    QString qsFileTemp( ui->lineEdit->text().remove( ui->lineEdit->text().size() - ui->lineEdit->text().split("/").last().size(),
                                                     ui->lineEdit->text().split("/").last().size() )+"temp.dat" );

    // переменная - Массив имён Файлов
    QString *qsFiles = new QString[iAmtFiles];
    for(int i=0; i<iAmtFiles; i++){
        qsFiles[i] = ui->tableWidget_files->item(i,1)->text()+"\\"+ui->tableWidget_files->item(i,0)->text();
    }

    // Проверка файла
    int iLines = 0;
    int iSumLines = 0;
    QString linestr;

    for (int iFiles=0; iFiles<iAmtFiles; iFiles++) {
        QFile open_file(qsFiles[iFiles]);
        iLines = 0;
        if(open_file.open(QIODevice::ReadOnly)) {
            while(!open_file.atEnd()) {
                linestr = open_file.readLine();
                iLines++;
            }
            if (iFiles != 0) {
                if (iSumLines != iLines) {
                    QMessageBox::warning(this, tr("Error"), tr("Неправильный формат файлов!"));
                    sb1->setText("Файлов: "+(QString::number(iAmtFiles)) );    // Количество файлов
                    sb2->setText("Строк: Ощибка!" );
                    return false;
                }
                if (iSumLines == iLines) {
                    sb1->setText("Файлов: "+(QString::number(iAmtFiles)) );    // Количество файлов
                    sb2->setText("Строк: "+(QString::number(iLines)) );        // Количество строк
                }
            }
            iSumLines = iLines;
            open_file.close();
        }
    }

    QByteArray qbaSrez = "";
    QByteArray qbaRead = "";
    QByteArray qbaSum = "";
    int intStr = 0;

    for (int i=0; i<iAmtFiles; i++) {
        QFile fileSrez(ui->lineEdit->text());
        QFile fileTemp(qsFileTemp);
        QFile fileRead(qsFiles[i]);

        if(fileSrez.open(QIODevice::ReadWrite) && fileRead.open(QIODevice::ReadOnly) && fileTemp.open(QIODevice::ReadWrite)) {
            intStr = 0;
             while(!fileRead.atEnd()) {
                if(i == 0)  {
                    if(intStr == 0) {
                        qbaSrez = "Проекция\n";
                        fileSrez.write(qbaSrez);
                        qbaSrez = fileSrez.readLine();
                        qbaRead = fileRead.readLine();
                    }
                    if(intStr != 0) {
                        qbaRead = fileRead.readLine();
                        fileSrez.write(qbaRead);
                    }
                }
                if(i != 0) {
                    if(intStr == 0) {
                        qbaSrez = "Проекция\n";
                        fileTemp.write(qbaSrez);
                        qbaSrez = fileSrez.readLine();
                        qbaRead = fileRead.readLine();
                    }
                    if(intStr != 0) {
                        qbaSrez = fileSrez.readLine();
                        qbaRead = fileRead.readLine();
                        qbaSum = strMaxItem(qbaSrez, qbaRead);
                        fileTemp.write(qbaSum);
                    }
                }
                intStr=1;
             }
             fileSrez.close();
             fileTemp.close();
             fileRead.close();
             if (i != 0) {
                  fileSrez.remove();
                  fileTemp.rename(ui->lineEdit->text());
             }
             if (i == 0) {
                 fileTemp.remove();
             }
        }
    }

    delete[] qsFiles;
    sb0->setText("Готово!");
    return true;
}
