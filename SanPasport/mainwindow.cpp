#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QSqlRecord>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyledItemDelegate>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QVector>
#include <QtConcurrent>
#include <QTreeWidgetItem>

#include "ViewPlot/viewplot.h"

using namespace std;

// ------- Глобальные переменные ------- //

QModelIndex G_qmiindex;
QFile G_fOpenProj;
QString G_strDbPath;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Thread = new CalcZozThread();

    // Статус бар - Пусто
    ui->statusBar->setSizeGripEnabled(false);
    sb1 = new QLabel(statusBar());
    sb1->setText(" ");
    // Статус бар - Прогресс бар
    pbar = new QProgressBar(statusBar());
    pbar->setAlignment(Qt::AlignRight);
    pbar->setMaximumHeight(15);
    pbar->setMaximum(100);
    pbar->setEnabled(false);
    // Статус бар - Пусто
    ui->statusBar->setSizeGripEnabled(false);
    sb1 = new QLabel(statusBar());
    sb1->setText(" ");
    // Статус бар - Коэфициент
    sbKoef = new QPushButton("Коэф. ");
    sbKoef->setStyleSheet("border: none;");

    ui->statusBar->addWidget(sb1, 1);
    ui->statusBar->addWidget(pbar, 2);
    ui->statusBar->addWidget(sb1, 5);
    ui->statusBar->addWidget(sbKoef, 1);

    EnableUI(false);

    ui->toolBar_File->layout()->setSpacing(2);
    ui->toolBar_Edit->layout()->setSpacing(2);
    ui->toolBar_Graphics->layout()->setSpacing(2);

    // ------- Диалоги ------- //
    dlg_TaskZo = new Dialog_TaskZo(this);
    dlg_TaskVs = new Dialog_TaskVs(this);
    dlg_TaskPoint = new Dialog_TaskPoint(this);
    dlg_EditAnt = new Dialog_EditAnt(this);
    dlg_Parametrs = new Dialog_Parametrs(this);
    dlg_Preferens = new Dialog_Preferens(this);
    win_SitPlan = new SitPlan(this);

    /// ФАЙЛ
    connect( ui->action_FileProjectNew, SIGNAL(triggered()), this, SLOT(fileNew()) );                          // Новый проект
    connect( ui->action_FileOpen, SIGNAL(triggered()), this, SLOT(fileOpen()) );                               // Файл открыть
    connect( ui->action_FileSave, SIGNAL(triggered()), this, SLOT(fileSave()) );                               // Сохранить проект
    connect( ui->action_FileSaveAs, SIGNAL(triggered()), this, SLOT(fileSaveAs()) );                           // Сохранить проект как
    connect( ui->action_FileSaveCalc, SIGNAL(triggered()), SLOT(fileSaveCalc()) );
    connect( ui->action_FileImportPKAEMO4, SIGNAL(triggered()), this, SLOT(fileImportPkaemo4_DialogOpen()) );  // Импорт из ПКАЭМО
    connect( ui->action_FileExport, SIGNAL(triggered()), this, SLOT(fileExportPkaemo4()) );                    // Экспорт в ПКАЭМО
    connect( ui->action_FileClose, SIGNAL(triggered()), this, SLOT(fileClose()) );                             // Закрыть проект
    connect( ui->action_Exit, SIGNAL(triggered()), this, SLOT(close()) );                                      // Выход
    connect( ui->action_Preferens, SIGNAL(triggered()), dlg_Preferens, SLOT(show()) );    // Настройки программы
    /// ПРТО
    connect( ui->action_PrtoSaveAs, SIGNAL(triggered()), this, SLOT(prtoSaveAs()) );    // Сохранить ПРТО в файл
    connect( ui->action_PrtoLoadPPC, SIGNAL(triggered()), SLOT(prtoLoadPPC()) );        // Загружаем РРС из фала
    connect( ui->tableView_Prto, SIGNAL(keyPressed(int,int)), SLOT(prtoKeyPresed(int, int)) );  // Горячие клавиши
    connect( ui->tableView_Prto, SIGNAL(customContextMenuRequested(QPoint)), SLOT(contextMenuPrto(QPoint))); // Контекстное меню
    connect( ui->action_PrtoAddPPC, SIGNAL(triggered()), SLOT(prtoAddPPC()) );          // Добавить РРС
    connect( ui->action_prtoEnabled, SIGNAL(triggered()), this, SLOT(prtoEnable()) );   // Вкл./Выкл. антенну
    connect( ui->action_PrtoEnableAll, SIGNAL(triggered()), SLOT(prtoEnableAll()) );    // Включить все антенны
    connect( ui->action_PrtoDisableAll, SIGNAL(triggered()), SLOT(prtoDisableAll()) );  // Отключить все антенны
    connect( ui->action_PrtoAdd, SIGNAL(triggered()), this, SLOT(prtoOpenFile()) );
    connect( ui->tableView_Prto, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(prtoEdit()) );
    connect( ui->action_EditAnt, SIGNAL(triggered()), this, SLOT(prtoEdit()) );
    connect( ui->tableView_Prto, SIGNAL(clicked(QModelIndex)), this, SLOT(selected_ant()) );
    connect( this, SIGNAL(sendIdEditAnt(Prto)), dlg_EditAnt, SLOT(insertDataForm(Prto)) );
    connect( dlg_EditAnt, SIGNAL(sendEditAntFromDlgEdit(Prto)), this, SLOT(saveEditantPattern(Prto)) );
    connect( ui->action_EditRemove, SIGNAL(triggered()), this, SLOT(prtoRemove()) );
    connect( ui->tableView_Prto->verticalHeader(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(prtoMoved(int,int,int)) );
    connect( ui->action_PrtoMoveUp, SIGNAL(triggered()), SLOT(prtoMoveUp()) );
    connect( ui->action_PrtoMoveDown, SIGNAL(triggered()), SLOT(prtoMoveDown()) );
    /// ЗАДАНИЯ
    connect( ui->action_TaskView, SIGNAL(triggered()), this, SLOT(taskView()) );          // Показать задание
    connect( ui->action_TaskDelete, SIGNAL(triggered()), this, SLOT(taskRemove()) );      // Удалить задание
    connect( ui->action_TaskEnable, SIGNAL(triggered()), this, SLOT(taskEnable()) );      // Вкл/Выкл задание
    connect( ui->action_TaskEnableAll, SIGNAL(triggered()), SLOT(taskEnableAll()) );      // Включить все задания
    connect( ui->action_TaskDisableAll, SIGNAL(triggered()), SLOT(taskDisableAll()) );    // Отключить все задания
    connect( ui->action_TaskEdit, SIGNAL(triggered()), this, SLOT(taskEdit()) );          // Редактировать задание
    connect( ui->tableView_Task, SIGNAL(doubleClicked(QModelIndex)), SLOT(taskEdit()) );  // Редактировать задание     
    connect( ui->action_TaskCalcZo, SIGNAL(triggered()), dlg_TaskZo, SLOT(show()) );      // Добавить задание - Зона ограничения
    connect( dlg_TaskZo, SIGNAL(sendTaskZo()), SLOT(taskInsert()) );        // Добавить задание - Зона ограничения
    connect( ui->action_TaskCalcVS, SIGNAL(triggered()), dlg_TaskVs, SLOT(show())  );     // Добавить задание - Вертикальное сечение
    connect( dlg_TaskVs, SIGNAL(sendTaskVs()), SLOT(taskInsert()) );        // Добавить задание - Вертикальное сечение
    connect( ui->action_TaskCalcEmpPoint, SIGNAL(triggered()), dlg_TaskPoint, SLOT(show()) ); // Добавить задание - Расчет в точке
    connect( dlg_TaskPoint, SIGNAL(sendTaskPoint()), SLOT(taskInsert()) );      // Добавить задание - Расчет в точке
    connect( ui->action_TaskZoFromPrto, SIGNAL(triggered()), SLOT(tasZoFromPrto()) );             // ЗО сечения на основе ПРТО
    connect( ui->action_TaskVertFromPrto, SIGNAL(triggered()), this, SLOT(taskVertFromPrto()) );  // Верткальные сечения на основе ПРТО
    connect( ui->tableView_Task, SIGNAL(keyPressed(int,int)), SLOT(taskKeyPresed(int, int)) );
    connect( ui->tableView_Task, SIGNAL(customContextMenuRequested(QPoint)), SLOT(contextMenuTask(QPoint)) ); // Контекстное меню
    connect( ui->tableView_Task->verticalHeader(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(taskMoved(int,int,int)) );
    connect( ui->action_TaskMoveUp, SIGNAL(triggered()), SLOT(taskMoveUp()) );
    connect( ui->action_TaskMoveDown, SIGNAL(triggered()), SLOT(taskMoveDown()) );
    /// Параметры проекта
    connect( ui->action_Param, SIGNAL(triggered()), this, SLOT(showDlgParametrs()) );
    connect( dlg_Parametrs, SIGNAL(SendOptionsFromDlgOption(QString)), SLOT(signal_SendOptions(QString)) );
    connect( ui->action_EditDublAnt, SIGNAL(triggered()), this, SLOT(prtoDublicate()) );
    connect( this, SIGNAL(destroyed()), this, SLOT(fileClose()) );
    connect( sbKoef, SIGNAL(clicked()), this, SLOT(showDlgParametrs()) );
    /// База данных
    connect( ui->toolButtonDb_PrtoView, SIGNAL(clicked()), this, SLOT(dbPrtoView()) );          // Просмотр ДН
    connect( ui->toolButtonDb_PrtoInsert, SIGNAL(clicked()), this, SLOT(dbPrtoInsert()) );        // Добавить Прто
    connect( ui->tableWidgetDbSearch, SIGNAL(doubleClicked(QModelIndex)), SLOT(dbPrtoInsert()) );
    connect( ui->treeView_Db, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(dbPrtoInsert()) );   // Добавить Прто
    connect( dlg_Preferens, SIGNAL(dbPath(QString)), SLOT(dbPath(QString)) );
    connect( ui->toolButtonDb_Browse, SIGNAL(clicked()), SLOT(dbBrowse()) );
    connect( ui->toolButtonDb_OpenOutside, SIGNAL(clicked()), SLOT(dbOpenOutside()) );
    connect( ui->treeView_Db, SIGNAL(doubleClicked(QModelIndex)), SLOT(dbSetRoot(QModelIndex)) );
    connect( ui->toolButtonDb_Up, SIGNAL(clicked()), SLOT(dbUp()) );
    connect( ui->toolButtonDb_Filter, SIGNAL(clicked(bool)), SLOT(dbFilter(bool)) );
    connect( ui->lineEditDb_Search, SIGNAL(editingFinished()), SLOT(dbSearch()) );
    /// Графики
    connect( ui->action_GraphcsLoadZo, SIGNAL(triggered()), this, SLOT(graphcsOpenZo()) );   // Открыт файл ЗО
    connect( ui->action_SitPlan, SIGNAL(triggered()), this, SLOT(graphcsSitPlan()) );        // Открыть ситуационный план
    /// Расчет
    connect( ui->action_CalcStart, SIGNAL(triggered()), SLOT(calcStart()) );
    connect( ui->action_CalcStop, SIGNAL(triggered()), SLOT(calcStop()) );
    /// Справка
    connect( ui->action_HelpPpcCsv, SIGNAL(triggered()), SLOT(helpPpcCsv()) );
    connect( ui->action_HelpAbout, SIGNAL(triggered()), SLOT(helpAbout()) );
    // Разрешаем Контекстное меню
    ui->tableView_Prto->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView_Task->setContextMenuPolicy(Qt::CustomContextMenu);

    QSettings *stngSanPasport = new QSettings( (QCoreApplication::applicationDirPath())
                                              + "//configSanPasport.ini",QSettings::IniFormat );
    stngSanPasport->beginGroup("SanPasport");
    G_strDbPath = stngSanPasport->value("DbPath", "").toString();
    stngSanPasport->endGroup();

    // База данных
    modelDb = new QFileSystemModel();
    ui->treeView_Db->setModel(modelDb);
    ui->treeView_Db->setRootIndex(modelDb->setRootPath(G_strDbPath));
    ui->treeView_Db->header()->resizeSection(0,300);
    ui->treeView_Db->hideColumn(1); // Размер
    ui->treeView_Db->hideColumn(2); // Тип
    ui->treeView_Db->hideColumn(3); // Дата
    ui->tableWidgetDbSearch->setVisible(false);

    // Реализация Drag'n'Drop
    setAcceptDrops(true);

    ui->tabWidget->removeTab(1);              // Таб с общими данными
    ui->tabWidget->removeTab(1);              // Таб с графиками
    ui->lineEditDb_Search->setVisible(false); // Строка поиска
}


// ----------------------------------- Деструктор ----------------------------------- //
MainWindow::~MainWindow()
{
    delete ui;
}


/* ----------------------------------- Контекстное меню ----------------------------------- */
void MainWindow::contextMenuPrto(const QPoint &pos)
{
    if( ui->tableView_Prto->selectionModel()->selectedIndexes().size() > 0 )
        ui->menu_Prto->exec( ui->tableView_Prto->mapToGlobal(pos) );
}


void MainWindow::contextMenuTask(const QPoint &pos)
{
    if( ui->tableView_Task->selectionModel()->selectedIndexes().size() > 0 )
        ui->menu_Task->exec( ui->tableView_Task->mapToGlobal(pos) );
}


/* ----------------------------------- Закрытие окна ----------------------------------- */
void MainWindow::closeEvent(QCloseEvent *event)
{
    if(ui->action_FileSave->isEnabled()==false)
        return;

    QMessageBox msgBox(QMessageBox::Question, tr("Закрыть проект"), tr("Сохранить изменения в проекте?"),
                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setButtonText(QMessageBox::Yes, trUtf8("Да"));
    msgBox.setButtonText(QMessageBox::No, trUtf8("Нет"));
    msgBox.setButtonText(QMessageBox::Cancel, trUtf8("Отмена"));

    int iMessage(msgBox.exec());

    if (iMessage==QMessageBox::No)
        event->accept();
    else if(iMessage==QMessageBox::Cancel)
        event->ignore();
    else {
        fileSave();
        event->accept();
    }
}


// ----------------------------------- Реализация Drag'n'Drop ----------------------------------- //
void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if(event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}


// ----------------------------------- Реализация Drag'n'Drop ----------------------------------- //
void MainWindow:: dropEvent(QDropEvent* event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.isEmpty())
        return;

    QString fileName = urls.first().toLocalFile();

    if(fileName.isEmpty())
        return;
    else {
        if( fileName.split(".").last().toLower() == "rto" and ui->menu_Prto->isEnabled() == false )
            fileImportPkaemo4(fileName);

        else if(fileName.split(".").last().toLower() == "msi" and ui->menu_Prto->isEnabled() == true)
           prtoFromFile(fileName);
    }
}


// ----------------------------------- Включить/Выключить UI ----------------------------------- //
void MainWindow::EnableUI(bool b)
{
    ui->action_FileProjectNew->setEnabled(!b);
    ui->action_FileSave->setEnabled(b);
    ui->action_FileSaveAs->setEnabled(b);
    ui->action_FileImportPKAEMO4->setEnabled(!b);
    ui->action_FileExport->setEnabled(b);
    ui->action_PrtoAdd->setEnabled(b);
    ui->action_CalcStop->setEnabled(b);
    ui->dockWidget_antBd->setEnabled(b);
    ui->menu_Prto->setEnabled(b);
    ui->action_EditAnt->setEnabled(b);
    ui->action_EditDublAnt->setEnabled(b);
    ui->action_EditRemove->setEnabled(b);
    ui->action_Param->setEnabled(b);
    ui->action_SitPlan->setEnabled(b);
    ui->menu_Task->setEnabled(b);
    ui->menu_Graphics->setEnabled(b);
    ui->action_FileClose->setEnabled(b);
    ui->action_SitPlan->setEnabled(b);
    ui->menu_Calc->setEnabled(b);
}


// ----------------------------------------------------------------------------------------------------------------- //
// ----------------------------------------------------------- ФАЙЛ ------------------------------------------------ //
// ----------------------------------------------------------------------------------------------------------------- //

// ----------------------------------- ФАЙЛ - Новый проект ----------------------------------- //
void MainWindow::fileNew()
{
    prjFile->dbNew();
    fileLoadModel(QSqlDatabase::database("project"));
}


/* ------- Файл -> Открыть ------- */
void MainWindow::fileOpen()
{
    if(ui->menu_Prto->isEnabled())
        if(!fileClose())
            return;

    QString fOpen = QFileDialog::getOpenFileName( this, tr("Открыть проект ПКАЭМО"), "",
                                                  tr("All support files (*.spp);;") );
    if (fOpen.isEmpty())
        return;

    fileOpen(fOpen);
}


void MainWindow::fileOpen(const QString fOpen)
{
    prjFile->dbNew();
    createDb newProject;
    newProject.dbSaveAs(QSqlDatabase::database("project"), fOpen, false );
    fileLoadModel(QSqlDatabase::database("project"));

    G_fOpenProj.setFileName(fOpen);
    this->setWindowTitle("SanPasport - " + fOpen.split("/").last());
}


/* ------- Загрузка модели ------- */
void MainWindow::fileLoadModel(const QSqlDatabase db)
{
    modelOptions = new QSqlTableModel(0,db.database("project"));
    modelOptions->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelOptions->setTable("Options");
    modelOptions->select();

    // modelPrto
    modelPrto = new myModelPrto(0,db.database("project"));
    modelPrto->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelPrto->setTable("Prto");
    modelPrto->select();

    // Устанавливаем названия столбцов
    modelPrto->setHeaderData(0, Qt::Horizontal, "ИД");
    modelPrto->setHeaderData(1, Qt::Horizontal, "Вкл");
    modelPrto->setHeaderData(2, Qt::Horizontal, "Название");
    modelPrto->setHeaderData(3, Qt::Horizontal, "Сектор");
    modelPrto->setHeaderData(4, Qt::Horizontal, "Частота");
    modelPrto->setHeaderData(5, Qt::Horizontal, "КУ");
    modelPrto->setHeaderData(6, Qt::Horizontal, "Размер");
    modelPrto->setHeaderData(7, Qt::Horizontal, "Поляр.");
    modelPrto->setHeaderData(8, Qt::Horizontal, "Мощность \n на входе");
    modelPrto->setHeaderData(9, Qt::Horizontal, "Мощность \n передатчика");
    modelPrto->setHeaderData(10, Qt::Horizontal, "Кол-во \n передатчков");
    modelPrto->setHeaderData(11, Qt::Horizontal, "Длина \n фидера");
    modelPrto->setHeaderData(12, Qt::Horizontal, "Потери");
    modelPrto->setHeaderData(13, Qt::Horizontal, "КСВН");
    modelPrto->setHeaderData(14, Qt::Horizontal, "Другие \n потери");
    modelPrto->setHeaderData(15, Qt::Horizontal, "X");
    modelPrto->setHeaderData(16, Qt::Horizontal, "Y");
    modelPrto->setHeaderData(17, Qt::Horizontal, "Высота");
    modelPrto->setHeaderData(18, Qt::Horizontal, "Азимут");
    modelPrto->setHeaderData(19, Qt::Horizontal, "Угол \n наклона");
    modelPrto->setHeaderData(20, Qt::Horizontal, "Порядок");
    modelPrto->setHeaderData(21, Qt::Horizontal, "ДН Гориз");
    modelPrto->setHeaderData(22, Qt::Horizontal, "ДН Верт");

    ui->tableView_Prto->setModel(modelPrto);


    ui->tableView_Prto->hideColumn(0);
    ui->tableView_Prto->hideColumn(7);
    ui->tableView_Prto->hideColumn(9);
    ui->tableView_Prto->hideColumn(10);
    ui->tableView_Prto->hideColumn(11);
    ui->tableView_Prto->hideColumn(12);
    ui->tableView_Prto->hideColumn(13);
    ui->tableView_Prto->hideColumn(14);
   // ui->tableView_Prto->hideColumn(20);
//    ui->tableView_Prto->hideColumn(21);
//    ui->tableView_Prto->hideColumn(22);
//    ui->tableView_Prto->hideColumn(23);
//    ui->tableView_Prto->hideColumn(24);

    ui->tableView_Prto->setItemDelegateForColumn(1,new mDelegateStat);
    ui->tableView_Prto->hideColumn(0);

    // Ширина названия столбцов
    ui->tableView_Prto->horizontalHeader()->resizeSection(0,30);
    ui->tableView_Prto->horizontalHeader()->resizeSection(1,50);
    ui->tableView_Prto->horizontalHeader()->resizeSection(2,200);
    ui->tableView_Prto->horizontalHeader()->resizeSection(3,100);
    ui->tableView_Prto->horizontalHeader()->resizeSection(4,50);
    ui->tableView_Prto->horizontalHeader()->resizeSection(5,50);
    ui->tableView_Prto->horizontalHeader()->resizeSection(6,50);
    ui->tableView_Prto->horizontalHeader()->resizeSection(7,50);
    ui->tableView_Prto->horizontalHeader()->resizeSection(8,75);
    ui->tableView_Prto->horizontalHeader()->resizeSection(9,40);
    ui->tableView_Prto->horizontalHeader()->resizeSection(10,40);
    ui->tableView_Prto->horizontalHeader()->resizeSection(11,40);
    ui->tableView_Prto->horizontalHeader()->resizeSection(12,50);
    ui->tableView_Prto->horizontalHeader()->resizeSection(13,60);
    ui->tableView_Prto->horizontalHeader()->resizeSection(14,150);
    ui->tableView_Prto->horizontalHeader()->resizeSection(15,40);
    ui->tableView_Prto->horizontalHeader()->resizeSection(16,40);
    ui->tableView_Prto->horizontalHeader()->resizeSection(17,50);
    ui->tableView_Prto->horizontalHeader()->resizeSection(18,50);
    ui->tableView_Prto->horizontalHeader()->resizeSection(19,60);
    // Сортировка
    ui->tableView_Prto->verticalHeader()->setSectionsMovable(true);
    ui->tableView_Prto->sortByColumn(20, Qt::AscendingOrder);
    ui->tableView_Prto->setSortingEnabled(false);

    // modelTask
    modelTask = new myModelTask(0,db.database("project"));
    modelTask->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelTask->setTable("TaskCalc");
    modelTask->select();
    ui->tableView_Task->setModel(modelTask);
    ui->tableView_Task->setItemDelegateForColumn(3,new mDelegate);
    ui->tableView_Task->setItemDelegateForColumn(1,new mDelegateStat);
    ui->tableView_Task->setItemDelegateForColumn(4,new mDelegatePath);

    modelTask->setHeaderData(0, Qt::Horizontal, "ИД");
    modelTask->setHeaderData(1, Qt::Horizontal, "Вкл");
    modelTask->setHeaderData(2, Qt::Horizontal, "Тип");
    modelTask->setHeaderData(3, Qt::Horizontal, "Параметры");
    modelTask->setHeaderData(4, Qt::Horizontal, "Статус");
    modelTask->setHeaderData(5, Qt::Horizontal, "Порядок");

    ui->tableView_Task->horizontalHeader()->resizeSection(0,40);
    ui->tableView_Task->horizontalHeader()->resizeSection(1,40);
    ui->tableView_Task->horizontalHeader()->resizeSection(2,40);
    ui->tableView_Task->horizontalHeader()->resizeSection(3,1000);
    ui->tableView_Task->hideColumn(0);
    ui->tableView_Task->hideColumn(2);
    ui->tableView_Task->hideColumn(2);
    ui->tableView_Task->hideColumn(5);
    // Сортировка
    ui->tableView_Task->verticalHeader()->setSectionsMovable(true);
    ui->tableView_Task->sortByColumn(5, Qt::AscendingOrder);
    ui->tableView_Task->setSortingEnabled(false);

    sbKoef->setText( "Коэф. " + QSqlRecord(modelOptions->record(0)).value("Koef").toString() );

    EnableUI(true);
}

QString MainWindow::currentPath()
{
    return G_fOpenProj.fileName();
}

/* ------- Файл -> Сохранить ------- */
void MainWindow::fileSave()
{
    if(G_fOpenProj.fileName() == "") {
        fileSaveAs();
        return;
    } else {
        createDb newProject;
        newProject.dbSaveAs(QSqlDatabase::database("project"), G_fOpenProj.fileName(), true );
    }
}


/* ------- Файл -> Сохранить как -------*/
void MainWindow::fileSaveAs()
{
    QString strSaveProject = QFileDialog::getSaveFileName(this,
             tr("Сохранить как..."), "untitled",
             tr("SanPasport (*.spp);;All Files (*)"));


    createDb newProject;
    newProject.dbSaveAs(QSqlDatabase::database("project"), strSaveProject, true );

    G_fOpenProj.setFileName(strSaveProject);
    this->setWindowTitle("SanPasport - " + strSaveProject.split("/").last());
}


/* -------  Сохранить результаты расчётов -------*/
void MainWindow::fileSaveCalc()
{
    if(G_fOpenProj.exists())
    {
        QFile flTask;
        QStringList strlPath(G_fOpenProj.fileName().replace("\\","/").split("/"));
        strlPath.removeLast();
        QString strPathNew(strlPath.join("/")+"/Result/");

        if(!(QDir(strPathNew).exists()))
            QDir().mkdir(strPathNew);

        QVector<task> vecTaskAll = frmt->taskFromDb();

        for(int i=0;  i < vecTaskAll.count(); i++)
        {
            QString strFileName( vecTaskAll.at(i).Path );
            strFileName.replace("\\","/");
            flTask.setFileName(strFileName);

            if(flTask.exists())
                if( !(flTask.copy(strPathNew + strFileName.split("/").last())) )
                {
                    QMessageBox msgBox;
                    msgBox.setText("Ошибка сохранения!");
                    msgBox.exec();
                    return;
                }
        }
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("Проект должен быть сохранён!");
        msgBox.exec();
    }
}


/* ------- Импорт в ПКАЭМО -------*/
void MainWindow::fileImportPkaemo4_DialogOpen()
{
    QString f = QFileDialog::getOpenFileName(this,
                                             tr("Открыть проект ПКАЭМО"), "",
                                             tr("All support files (*.rto);;")
                                             );
    if (!f.isEmpty()) {
        fileImportPkaemo4(f); }
}


// ------------------------------ Импорт данных из ПК АЭМО 4 ------------------------------ //
void MainWindow::fileImportPkaemo4(const QString f)
{
    prjFile->importPKAEMO(f);
    fileLoadModel(QSqlDatabase::database("project"));
    modelOptions->submitAll();
}


/* ------- Экспорт данных в ПК АЭМО 4 ------- */
void MainWindow::fileExportPkaemo4()
{
    QString fileCopy = QFileDialog::getSaveFileName(this, tr("Save As"), "united", tr("(*.rto);; Files PKAEMO(*)"));
    if(!fileCopy.isEmpty())
        prjFile->exportPKAEMO(fileCopy);
}


// ----------------------------------- Закрыть проект ----------------------------------- //
bool MainWindow::fileClose()
{
    QMessageBox msgBox(QMessageBox::Question, tr("Сообщение"), tr("Вы действительно хотите закрыть проект?"),
                       QMessageBox::Yes | QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, trUtf8("Да"));
    msgBox.setButtonText(QMessageBox::No, trUtf8("Нет"));
    int iMessage(msgBox.exec());

    if(iMessage == QMessageBox::No)
        return false;

    QSqlDatabase db;
    db.database("qt_sql_default_connection").close();
    delete modelPrto;
    delete modelOptions;
    delete modelTask;
    EnableUI(false);

    return true;
}



// ----------------------------------------------------------------------------------------------------------------- //
/// --------------------------------------------------------- ПРТО ------------------------------------------------- //
// ----------------------------------------------------------------------------------------------------------------- //

// ----------------------------------- ПРТО - Вкл.Выкл ----------------------------------- //
void MainWindow::prtoEnable()
{ 
    QModelIndex index;
    foreach (index, ui->tableView_Prto->selectionModel()->selectedRows())
    {
        QSqlRecord record = modelPrto->record( index.row() );
        record.setValue("Enabled", !record.value("Enabled").toBool());
        modelPrto->setRecord(index.row(), record);
    }
    modelPrto->submitAll();
}


/* ------- ПРТО - Включить все антенны ------- */
void MainWindow::prtoEnableAll()
{
    for(int row = 0; row < modelPrto->rowCount(); row++)
    {
        QSqlRecord record = modelPrto->record(row);
        record.setValue("Enabled", true);
        modelPrto->setRecord(row, record);
    }
    modelPrto->submitAll();
}


/* ------- ПРТО - Отключить все антенны ------- */
void MainWindow::prtoDisableAll()
{
    for(int row = 0; row < modelPrto->rowCount(); row++)
    {
        QSqlRecord record = modelPrto->record(row);
        record.setValue("Enabled", false);
        modelPrto->setRecord(row, record);
    }
    modelPrto->submitAll();
}


// ----------------------------------- ПРТО - Диалог открытия диаграммы ----------------------------------- //
void MainWindow::prtoOpenFile()
{
    QString f = QFileDialog::getOpenFileName(this,
                                             tr("Открыть фаил ДН"), "",
                                             tr("All support files (*.msi *.pln *.txt *.bdn);;"
                                                "MSI/Nokia (*.msi *.pln);;"
                                                "Text file (*.txt);;"
                                                "Inside (*.bdn);;"
                                                "All files (*)")
                                             );
    prtoFromFile(f);
}



// ----------------------------------- ПРТО - Открыть файл ----------------------------------- //
bool MainWindow::prtoFromFile(const QString &f)
{
    if(ui->menu_Prto->isEnabled() == false) { return false; }
    Prto dan;

    if (!f.isEmpty())
    {
        if (f.split(".").last().toLower() == "msi" || f.split(".").last().toLower() == "pln" || f.split(".").last().toLower() == "txt")
        {
            dan.clear();
            dan.loadMsi(f);
            prtoAdd(dan);
            return true;
        }
        if (f.split(".").last().toLower() == "bdn")
        {
            // loadedBdn(f);
            return true;
        }
        else
        {
            QMessageBox::warning(this, tr("Ошибка"), tr("Не правильный формат файла."));
            return false;
        }
    }

    return true;
}


// ----------------------------------- ПРТО - Сохранить в файл ----------------------------------- //
void MainWindow::prtoSaveAs()
{
    if( ui->tableView_Prto->selectionModel()->selectedIndexes().size() > 0 )
    {
        Prto prtoSave;
        prtoSave = prtoFromModel(ui->tableView_Prto->selectionModel()->selectedRows().first().row());
        QString fileName = QFileDialog::getSaveFileName(this,
                 tr("Save As"), prtoSave.Name,
                 tr("MSI/Nokia (*.msi);;All Files (*)"));
        if(!fileName.isEmpty())
            prtoSave.saveAsMsi(fileName);
    }
}


// =================================== ПРТО - Дудлировать =================================== //
void MainWindow::prtoDublicate()
{
    if( ui->tableView_Prto->selectionModel()->selectedIndexes().size() < 0 ) { return; }

    int numLastRow(ui->tableView_Prto->selectionModel()->selectedRows().last().row());
    QModelIndex index;

    foreach (index, ui->tableView_Prto->selectionModel()->selectedRows())
        prtoAdd( prtoFromModel(index.row()) );

    ui->tableView_Prto->selectRow(numLastRow + 1);
}



// =================================== ПРТО - Удаляем антенну =================================== //
bool MainWindow::prtoRemove()
{  
    if(ui->tableView_Prto->selectionModel()->currentIndex().row() == -1) {
        return false; }

    QMessageBox msgBox(QMessageBox::Question, tr("Удалить"), tr("Вы действительно хотите удалить антенну?"),
                       QMessageBox::Yes | QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, trUtf8("Да"));
    msgBox.setButtonText(QMessageBox::No, trUtf8("Нет"));

    int iMessage(msgBox.exec());

    if(iMessage==QMessageBox::No) {\
        return false; }

    QModelIndex index;

    foreach(index, ui->tableView_Prto->selectionModel()->selectedRows()) {
        modelPrto->removeRow(index.row()); }

    modelPrto->submitAll();
    return true;
}



// ----------------------------------- Вставить новую антенну из файла ----------------------------------- //
void MainWindow::prtoAdd(const Prto adIns)
{
    frmt->prtoAdd(adIns);
    modelPrto->submitAll();
}


// ----------------------------------- ПРТО - добавить РРС ----------------------------------- //
void MainWindow::prtoAddPPC()
{
    frmt->prtoAddPPC();
    modelPrto->submitAll();
}


// ------------------------------------ Чтение ДН из строки таблицы ------------------------------------ //
Prto MainWindow::prtoFromModel(const int iRow)
{
    QSqlRecord record = modelPrto->record(iRow);

    Prto apRead;
    apRead.clear();

    apRead.Id = record.value("id").toInt();
    apRead.Enabled = record.value("Enabled").toBool();
    apRead.Number = record.value("Number").toInt();
    apRead.Name = record.value("Name").toString();
    apRead.Owner = record.value("Owner").toString();
    apRead.Freq = record.value("Freq").toFloat();
    apRead.Gain = record.value("Gain").toFloat();
    apRead.Height = record.value("Height").toFloat();
    apRead.Type = record.value("Type").toInt();

    apRead.PowerTotal = record.value("PowerTotal").toFloat();
    apRead.PowerPRD = record.value("PowerPRD").toFloat();
    apRead.FeederLeght = record.value("FeederLeght").toFloat();
    apRead.FeederLoss = record.value("FeederLoss").toFloat();
    apRead.KSVN = record.value("KSVN").toFloat();
    apRead.LossOther = record.value("LossOther").toFloat();
    apRead.PRD = record.value("PRD").toInt();

    apRead.X = record.value("X").toFloat();
    apRead.Y = record.value("Y").toFloat();
    apRead.Z = record.value("Z").toFloat();
    apRead.Azimut = record.value("Azimut").toFloat();
    apRead.Tilt = record.value("Tilt").toFloat();

    QStringList qslRadHoriz(record.value("RadHoriz").toString().split(";"));
    QStringList qslRadVert(record.value("RadVert").toString().split(";"));

    for(int i = 0; i < 360; i++)
    {
        apRead.AzHoriz[i] = i;
        apRead.AzVert[i] = i;
        apRead.RadHoriz[i] = qslRadHoriz.at(i).toFloat();
        apRead.RadVert[i] = qslRadVert.at(i).toFloat();
    }
    return apRead;
}


// ----------------------------------- Открываем форму для редактирования антенны ---------------------------------- //
void MainWindow::prtoEdit()
{
    if(ui->tableView_Prto->selectionModel()->currentIndex().row() == -1) {
        return; }

    emit sendIdEditAnt(prtoFromModel(ui->tableView_Prto->currentIndex().row()));
    dlg_EditAnt->show();
}



/* ------ Получаем данныем из формы ------ */
void MainWindow::saveEditantPattern(Prto dan)
{
    createDb().prtoEdit(dan);
    modelPrto->submitAll();
    ui->tableView_Prto->selectRow(G_qmiindex.row());
}


/* ------ Загружаем РРС из фала ------ */
void MainWindow::prtoLoadPPC()
{
    QString fOpen = QFileDialog::getOpenFileName(this,
                                             tr("Открыть файл с РРС"), "",
                                             tr("All support files (*.csv);;")
                                             );
    QFile fPPC(fOpen);
    if (fOpen.isEmpty()) { return; }

    QString strAll;


    QStringList srtlAll;
    QStringList srtlLine;
    Prto prtoPPC;

    if(fPPC.open(QIODevice::ReadOnly))
    {
        QTextStream txtstrem(&fPPC);
        txtstrem.setCodec("Windows-1251");
        strAll = txtstrem.readAll();
        strAll.replace(",",".");
        srtlAll = strAll.split(QRegExp("\\n"), QString::SkipEmptyParts);

        for(int i=1; i < srtlAll.size(); i++)
        {
            srtlLine = srtlAll.at(i).split(QRegExp(";"));
            prtoPPC.clear();

            prtoPPC.Owner = srtlLine.at(0);               // Сектор
            prtoPPC.Name = srtlLine.at(1);                // Модель антенны
            prtoPPC.Freq = srtlLine.at(2).toFloat();      // Частота
            prtoPPC.Height = srtlLine.at(3).toFloat();    // Размер
            prtoPPC.Gain = srtlLine.at(4).toFloat();      // КУ
            prtoPPC.Azimut = srtlLine.at(5).toFloat();    // Азимут
            prtoPPC.X = srtlLine.at(6).toFloat();         // X
            prtoPPC.Y = srtlLine.at(7).toFloat();         // Y
            prtoPPC.Z = srtlLine.at(8).toFloat();         // Высота
            prtoPPC.Tilt = srtlLine.at(9).toFloat();      // Угол наклона
            prtoPPC.PowerPRD = srtlLine.at(10).toFloat(); // Мощность

            prtoPPC.Type = 15;
            prtoAdd(prtoPPC);
        }
        fPPC.close();
    }
}

void MainWindow::selected_ant()
{
    G_qmiindex = ui->tableView_Prto->selectionModel()->selectedRows().at(0);
}


/* ----------------------------------- ПРТО - Переместить вверх ----------------------------------- */
void MainWindow::prtoMoveUp()
{
    int curRuw(ui->tableView_Prto->currentIndex().row());
    Prto prtoOld( prtoFromModel(curRuw) );

    prtoOld.Number = curRuw - 0.5;
    createDb().prtoEdit(prtoOld);
    modelPrto->submitAll();

    for(int row = 0; row < modelPrto->rowCount(); row++)
    {
        Prto prt(prtoFromModel(row));
        prt.Number = row + 1;
        createDb().prtoEdit(prt);
    }
    modelPrto->submitAll();

    ui->tableView_Prto->selectRow(curRuw - 1);
}


/* ----------------------------------- ПРТО - Переместить вниз ----------------------------------- */
void MainWindow::prtoMoveDown()
{
    int curRuw(ui->tableView_Prto->currentIndex().row());
    Prto prtoOld( prtoFromModel(curRuw) );

    prtoOld.Number = curRuw + 2.5;
    createDb().prtoEdit(prtoOld);
    modelPrto->submitAll();

    for(int row = 0; row < modelPrto->rowCount(); row++)
    {
        Prto prt(prtoFromModel(row));
        prt.Number = row + 1;
        createDb().prtoEdit(prt);
    }
    modelPrto->submitAll();

    ui->tableView_Prto->clearSelection();
    ui->tableView_Prto->selectRow(curRuw + 1);
}


// ----------------------------------- ПРТО - Переместить ----------------------------------- //
void MainWindow::prtoMoved(int LogicIndex,int OldVisualIndex, int NewVisualIndex)
{
    ui->tableView_Prto->verticalHeader()->blockSignals(true);
    ui->tableView_Prto->verticalHeader()->moveSection(NewVisualIndex, OldVisualIndex);
    ui->tableView_Prto->verticalHeader()->blockSignals(false);

    if(NewVisualIndex > OldVisualIndex)
        for(int i = 0; i < (NewVisualIndex - OldVisualIndex); i++)
            prtoMoveDown();
    else
        for(int i = 0; i < (OldVisualIndex - NewVisualIndex); i++)
            prtoMoveUp();
}


/* ----------------------------------- ПРТО - Горячие клавиши ----------------------------------- */
void MainWindow::prtoKeyPresed(int numKey, int numModifierKey)
{
    switch (numKey)
    {
    case Qt::Key_Delete:
        prtoRemove();
        break;
    case Qt::Key_Insert:
        prtoOpenFile();
        break;
    case Qt::Key_Space:
        prtoEnable();
        break;
    case Qt::Key_Enter:
        prtoEdit();
        break;
    }  
    if(numKey == Qt::Key_D and numModifierKey == Qt::ControlModifier) prtoDublicate();
    else if(numKey == Qt::Key_Up and numModifierKey == Qt::ControlModifier)    prtoMoveUp();
    else if(numKey == Qt::Key_Down and numModifierKey == Qt::ControlModifier)  prtoMoveDown();
    else if(numKey == Qt::Key_Up)   ui->tableView_Prto->selectRow(ui->tableView_Prto->currentIndex().row() - 1);
    else if(numKey == Qt::Key_Down) ui->tableView_Prto->selectRow(ui->tableView_Prto->currentIndex().row() + 1);
}



/// ---------------------------------------------------------------------------------------------------------------- //
/// -------------------------------------------------------- БД ---------------------------------------------------- //
/// ---------------------------------------------------------------------------------------------------------------- //
/* ----------------------------------- БД - Просмотр ДН ----------------------------------- */
void MainWindow::dbPrtoView()
{
    QProcess* proc = new QProcess(this);

    if(ui->treeView_Db->isVisible())
        proc->start( "DnView.exe \"" + (modelDb->filePath(ui->treeView_Db->selectionModel()->currentIndex())) + "\"" );
    else if(ui->tableWidgetDbSearch->isVisible() and ui->tableWidgetDbSearch->currentRow() != -1)
       proc->start( "DnView.exe \"" + ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 1)->text()
                    + "/" + ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 0)->text() + "\"" );
}


/* ------- ПРТО - Открыть из БД ------- */
void MainWindow::dbPrtoInsert()
{
    QString strFile;

    if(ui->treeView_Db->isVisible())
        strFile = modelDb->filePath(ui->treeView_Db->selectionModel()->currentIndex()) ;
    else if(ui->tableWidgetDbSearch->isVisible() and ui->tableWidgetDbSearch->currentRow() != -1)
        strFile = ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 1)->text() + "/" +
                  ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 0)->text();

    if( !QFileInfo(strFile).isDir() ) {
        if ( QFileInfo(strFile).fileName().split(".").last().toLower() == "pdf" ) {
            dbOpenOutside();
        } else {
            prtoFromFile(strFile );
        }
    }
}


/* ----------------------------------- БД - Сменить папку базы данных ----------------------------------- */
void MainWindow::dbPath(const QString strPath)
{
    G_strDbPath = strPath;
    ui->treeView_Db->setRootIndex(modelDb->setRootPath(G_strDbPath));
}


/* ----------------------------------- БД - Открыть в проводнике ----------------------------------- */
void MainWindow::dbBrowse()
{
        QProcess *procBrowse = new QProcess(this);
        QFileInfo  finfBrowse;

        if(ui->treeView_Db->isVisible())
            finfBrowse.setFile(modelDb->filePath(ui->treeView_Db->selectionModel()->currentIndex()));
        else if(ui->tableWidgetDbSearch->isVisible() and ui->tableWidgetDbSearch->currentRow() != -1)
            finfBrowse.setFile(ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 1)->text() + "/" +
                               ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 0)->text());

        if(finfBrowse.isDir())
            procBrowse->start( "explorer.exe /e, \"" + finfBrowse.absoluteFilePath().replace("/","\\") + "\"" );
        else if(finfBrowse.isFile())
            procBrowse->start( "explorer.exe /select, \"" + finfBrowse.absoluteFilePath().replace("/","\\") + "\"" );
}


/* ----------------------------------- БД - Открыть в проводнике ----------------------------------- */
void MainWindow::dbOpenOutside()
{
    QProcess *procOpen = new QProcess(this);
    QFileInfo  finfBrowse;

    if(ui->treeView_Db->isVisible())
        finfBrowse.setFile(modelDb->filePath(ui->treeView_Db->selectionModel()->currentIndex()));
    else if(ui->tableWidgetDbSearch->isVisible() and ui->tableWidgetDbSearch->currentRow() != -1)
        finfBrowse.setFile(ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 1)->text() + "/" +
                           ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 0)->text());
    if(finfBrowse.isFile())
        procOpen->start( "explorer.exe /e, \"" + finfBrowse.absoluteFilePath().replace("/","\\") + "\"" );
}


/* ----------------------------------- БД - Открыть папку ----------------------------------- */
void MainWindow::dbSetRoot(QModelIndex indexRoot)
{
    if(modelDb->isDir(indexRoot))
    {
        ui->treeView_Db->setRootIndex(indexRoot);
        ui->toolButtonDb_Up->setEnabled(true);
    }
}


/* ----------------------------------- БД - Перейти на уровень вверх ----------------------------------- */
void MainWindow::dbUp()
{
    if(ui->treeView_Db->isVisible())
    {
        if(modelDb->filePath(ui->treeView_Db->rootIndex()) != G_strDbPath)
            ui->treeView_Db->setRootIndex(modelDb->parent(ui->treeView_Db->rootIndex()));
    }
    else if(ui->tableWidgetDbSearch->isVisible())
        dbSearchClear();

    if(modelDb->filePath(ui->treeView_Db->rootIndex()) == G_strDbPath)
        ui->toolButtonDb_Up->setEnabled(false);
}


/* ----------------------------------- БД - Фильтр ----------------------------------- */
void MainWindow::dbFilter(bool b)
{
    if(b)
        modelDb->setNameFilters(QStringList() << "*.msi" << "*.txt" << "*.pln");
    else
        modelDb->setNameFilters(QStringList() << "*.*");

    modelDb->setNameFilterDisables(!b);
}


/* ----------------------------------- БД - Очистить поиск ----------------------------------- */
void MainWindow::dbSearchClear()
{
    ui->tableWidgetDbSearch->clearContents();
    ui->tableWidgetDbSearch->setRowCount(0);
    ui->tableWidgetDbSearch->setVisible(false);
    ui->treeView_Db->setVisible(true);
    ui->lineEditDb_Search->setText("");
    ui->toolButtonDb_Filter->setEnabled(true);
}


/* ----------------------------------- БД - Поиск ----------------------------------- */
void MainWindow::dbSearch()
{
    if(ui->lineEditDb_Search->text() == "")
    {
        dbSearchClear();
        return;
    }

    ui->toolButtonDb_Up->setEnabled(true);
    ui->tableWidgetDbSearch->setVisible(true);
    ui->treeView_Db->setVisible(false);

    QString dir(modelDb->filePath(ui->treeView_Db->rootIndex()));
    QString name(ui->lineEditDb_Search->text());
    QStringList strlResult;

    QDirIterator it(dir, QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        it.next();
        if (it.fileName().contains(name))
            if(QFileInfo(it.filePath()).isFile())
                strlResult.append(it.filePath());
    }

    ui->tableWidgetDbSearch->setColumnCount(2);
    ui->tableWidgetDbSearch->setRowCount(strlResult.size());
    ui->tableWidgetDbSearch->setHorizontalHeaderLabels(QStringList() << "Файл" << "Путь");

    for(int i=0; i<strlResult.size(); i++)
    {
        ui->tableWidgetDbSearch->setItem( i, 0, new QTableWidgetItem(QFileInfo(strlResult.at(i)).fileName()) );
        ui->tableWidgetDbSearch->setItem( i, 1,
                                   new QTableWidgetItem(QFileInfo(strlResult.at(i)).absoluteDir().absolutePath()) );
    }
    ui->toolButtonDb_Filter->setEnabled(false);
}


/// ---------------------------------------------------------------------------------------------------------------- //
/// -------------------------------------------------------- ЗАДАНИЕ ----------------------------------------------- //
/// ---------------------------------------------------------------------------------------------------------------- //

// ----------------------------------- ЗАДАНИЕ - Добавить новое задание ----------------------------------- //
void MainWindow::taskInsert()
{
    modelTask->submitAll();
}



// ----------------------------------- ЗАДАНИЕ - прочитать задание из модели ----------------------------------- //
task MainWindow::taskFromModel(int intRow)
{
    QSqlRecord record = modelTask->record(intRow);
    task tsk;

    tsk.Id = record.value("id").toInt();
    tsk.Enabled = record.value("Enabled").toBool();
    tsk.Type = record.value("TYPE").toInt();
    tsk.Data = record.value("TASK").toString();
    tsk.Path = record.value("Path").toString();
    tsk.Number = record.value("Number").toInt();

    return tsk;
}


void MainWindow::taskView()
{
    QSqlRecord record = modelTask->record( ui->tableView_Task->selectionModel()->selectedRows().first().row() );
    QString strPath;

    if(QFile(record.value("Path").toString()).exists())
        strPath = record.value("Path").toString();
    else
        strPath = QFileInfo(G_fOpenProj).absolutePath() + "/Result/"
                + QFileInfo(record.value("Path").toString()).fileName();

    if(QFile(strPath).exists())
    {
        ViewPlot *vpNew = new ViewPlot(this);
        vpNew->plotLoadZo(strPath);
        vpNew->show();
        vpNew->plotSquare();
    }
}


// ----------------------------------- ЗАДАНИЕ - Вкл/Выкл ----------------------------------- //
void MainWindow::taskEnableAll()
{
    for(int row = 0; row < modelTask->rowCount(); row++)
    {
        QSqlRecord record = modelTask->record(row);
        record.setValue("Enabled", true);
        modelTask->setRecord(row, record);
    }
    modelTask->submitAll();
}


// ----------------------------------- ЗАДАНИЕ - Включить все задания ----------------------------------- //
void MainWindow::taskEnable()
{
    QModelIndex index;
    int curRow(ui->tableView_Task->selectionModel()->selectedRows().last().row());
    foreach (index, ui->tableView_Task->selectionModel()->selectedRows())
    {
         QSqlRecord record = modelTask->record( index.row() );
         record.setValue("Enabled", !record.value("Enabled").toBool());
         modelTask->setRecord(index.row(), record);
    }
    modelTask->submitAll();
    ui->tableView_Task->selectRow(curRow);
}


// ----------------------------------- ЗАДАНИЕ - Отключить все задания ----------------------------------- //
void MainWindow::taskDisableAll()
{
    for(int row = 0; row < modelTask->rowCount(); row++)
    {
        QSqlRecord record = modelTask->record(row);
        record.setValue("Enabled", false);
        modelTask->setRecord(row, record);
    }
    modelTask->submitAll();
}


// ----------------------------------- ЗАДАНИЕ - Редактировать ----------------------------------- //
void MainWindow::taskEdit()
{ 
    task tsk(taskFromModel(ui->tableView_Task->currentIndex().row()));

    if(tsk.Type == 1)
    {
        dlg_TaskZo->show();
        dlg_TaskZo->insertData(tsk);
    }
    else if (tsk.Type == 2)
    {
        dlg_TaskVs->show();
        dlg_TaskVs->insertData(tsk);
    }
    else if (tsk.Type == 3)
    {
        dlg_TaskPoint->show();
        dlg_TaskPoint->insertData(tsk);
    }
}

// ----------------------------------- ЗАДАНИЕ - Удаляем задание ----------------------------------- //
bool MainWindow::taskRemove()
{   
    QMessageBox msgBox(QMessageBox::Question, tr("Удалить"), tr("Вы действительно хотите удалить задание?"),
                       QMessageBox::Yes | QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, trUtf8("Да"));
    msgBox.setButtonText(QMessageBox::No, trUtf8("Нет"));

    int iMessage(msgBox.exec());

    if(iMessage==QMessageBox::No)
        return false;

    QModelIndex index;

    foreach(index, ui->tableView_Task->selectionModel()->selectedRows())
        modelTask->removeRow(index.row());

    modelTask->submitAll();
    return true;
}


/* ------- Горизонтальные сечения на основе ПРТО ------- */
void MainWindow::tasZoFromPrto()
{
    Prto prtoTask;
    QList<float> lstTaskZo;

    for ( int i=0; i<modelPrto->rowCount(); i++ )
    {
        prtoTask.clear();
        prtoTask = prtoFromModel(i);
        if ( !lstTaskZo.contains(prtoTask.Z) )
            lstTaskZo.append(prtoTask.Z);
    }
    qStableSort(lstTaskZo.begin(), lstTaskZo.end(), qGreater<float>());

    QStringList strlHeght;
    for ( int j=0; j<lstTaskZo.count(); j++ )
       strlHeght.append( QString::number(lstTaskZo.at(j)) );

    dlg_TaskZo->show();
    dlg_TaskZo->insertHeight( strlHeght.join(" ") );
}


/* ------- Верткальные сечения на основе ПРТО ------- */
void MainWindow::taskVertFromPrto()
{
    Prto prtoTask;
    QString strTaskVS;
    QString strRange;
    task tskVs;

    QSettings *stngVsAll = new QSettings( (QCoreApplication::applicationDirPath())
                                          + "//configSanPasport.ini",QSettings::IniFormat );
    stngVsAll->beginGroup("SanPasport");
    strRange.append( stngVsAll->value("VsRmin", "0").toString() ).append(";");
    strRange.append( stngVsAll->value("VsRmax", "100").toString() ).append(";");
    strRange.append( stngVsAll->value("VsRd", "0.5").toString() ).append(";");
    strRange.append( stngVsAll->value("VsHmin", "0").toString() ).append(";");
    strRange.append( stngVsAll->value("VsHmax", "40").toString() ).append(";");
    strRange.append( stngVsAll->value("VsHd", "0.5").toString() ).append(";");
    stngVsAll->endGroup();
    delete stngVsAll;

    for(int i=0; i<modelPrto->rowCount(); i++)
    {
        prtoTask.clear();
        strTaskVS.clear();
        prtoTask = prtoFromModel(i);

        strTaskVS.append(QString::number(prtoTask.X)).append(";");
        strTaskVS.append(QString::number(prtoTask.Y)).append(";");
        strTaskVS.append(strRange);
        strTaskVS.append(QString::number(prtoTask.Azimut)).append(";");
        strTaskVS.append("0");

        QSqlQuery qCount(QSqlDatabase::database("project"));
        qCount.exec("SELECT COUNT(*) FROM TaskCalc WHERE TASK = '"+strTaskVS+"'");
        qCount.next();

        if(qCount.value(0).toInt() == 0)
        {
            tskVs.Id = -1;
            tskVs.Type = 2;
            tskVs.Data = strTaskVS;

            createDb cbd;
            cbd.taskAdd(tskVs);
            modelTask->submitAll();
        }
    }
}


// ----------------------------------- ЗАДАНИЕ - Переместить вверх ----------------------------------- //
void MainWindow::taskMoveUp()
{
    int curRuw(ui->tableView_Task->currentIndex().row());
    task tskOld( taskFromModel(curRuw) );

    tskOld.Number = curRuw - 0.5;
    createDb().taskAdd(tskOld);
    modelTask->submitAll();

    for(int row = 0; row < modelTask->rowCount(); row++)
    {
        task tsk(taskFromModel(row));
        tsk.Number = row + 1;
        createDb().taskAdd(tsk);
    }
    modelTask->submitAll();

    ui->tableView_Task->selectRow(curRuw - 1);
}


// ----------------------------------- ЗАДАНИЕ - Переместить вниз ----------------------------------- //
void MainWindow::taskMoveDown()
{
    int curRuw(ui->tableView_Task->currentIndex().row());
    task tskOld( taskFromModel(curRuw) );

    tskOld.Number = curRuw + 2.5;
    createDb().taskAdd(tskOld);
    modelTask->submitAll();

    for(int row = 0; row < modelTask->rowCount(); row++)
    {
        task tsk(taskFromModel(row));
        tsk.Number = row + 1;
        createDb().taskAdd(tsk);
    }
    modelTask->submitAll();

    ui->tableView_Task->clearSelection();
    ui->tableView_Task->selectRow(curRuw + 1);
}


// ----------------------------------- ЗАДАНИЕ - Переместить ----------------------------------- //
void MainWindow::taskMoved(int LogicIndex,int OldVisualIndex, int NewVisualIndex)
{
    ui->tableView_Task->verticalHeader()->blockSignals(true);
    ui->tableView_Task->verticalHeader()->moveSection(NewVisualIndex, OldVisualIndex);
    ui->tableView_Task->verticalHeader()->blockSignals(false);

    if(NewVisualIndex > OldVisualIndex)
        for(int i = 0; i < (NewVisualIndex - OldVisualIndex); i++)
            taskMoveDown();
    else
        for(int i = 0; i < (OldVisualIndex - NewVisualIndex); i++)
            taskMoveUp();
}


void MainWindow::taskKeyPresed(int numKey, int numModifierKey)
{
    switch (numKey)
    {
    case Qt::Key_Delete:
        taskRemove();
        break;
    case Qt::Key_H:
        dlg_TaskZo->show();
        break;
    case Qt::Key_V:
        dlg_TaskVs->show();
        break;
    case Qt::Key_P:
        dlg_TaskPoint->show();
        break;
    case Qt::Key_Space:
        taskEnable();
        break;
    }

    if(numKey == 1056)      dlg_TaskZo->show();
    else if(numKey == 1052) dlg_TaskVs->show();
    else if(numKey == 1047) dlg_TaskPoint->show();
    else if(numKey == Qt::Key_A    and numModifierKey == Qt::ControlModifier) taskView();
    else if(numKey == Qt::Key_Up   and numModifierKey == Qt::ControlModifier) taskMoveUp();
    else if(numKey == Qt::Key_Down and numModifierKey == Qt::ControlModifier) taskMoveDown();
    else if(numKey == Qt::Key_Up)   ui->tableView_Task->selectRow(ui->tableView_Task->currentIndex().row() - 1);
    else if(numKey == Qt::Key_Down) ui->tableView_Task->selectRow(ui->tableView_Task->currentIndex().row() + 1);
}

/// ---------------------------------------------------------------------------------------------------------------- //
/// ---------------------------------------------------------- РАСЧЕТ ---------------------------------------------- //
/// ---------------------------------------------------------------------------------------------------------------- //

// ----------------------------------- РАСЧЕТ - Параметры расчета ----------------------------------- //
void MainWindow::showDlgParametrs()
{
    if(ui->menu_Prto->isEnabled())
    {
        dlg_Parametrs->loadParam( QSqlRecord(modelOptions->record(0)).value("Koef").toFloat() );
        dlg_Parametrs->show();
    }
}


void MainWindow::calcStart()
{
    QVector<task> vecTask;
    QVector<Prto> vecPrto;
    task taskCalc;
    Prto prtoCalc;
    CalcZozThread *thCalc = new CalcZozThread();

    for(int i=0; i < modelPrto->rowCount(); i++)
    {
        prtoCalc.clear();
        prtoCalc = prtoFromModel(i);

        if(prtoCalc.Type == 100 and prtoCalc.Enabled == true)
            vecPrto.append(prtoCalc);
    }

    for(int i=0; i < modelTask->rowCount(); i++)
    {
        taskCalc.clear();
        taskCalc = taskFromModel(i);

        if(taskCalc.Type == 1 and taskCalc.Enabled == true and taskCalc.Data.split(";").last() == "0")
            vecTask.append(taskCalc);
    }

    thCalc->loadTask(vecTask, vecPrto);
    connect( thCalc, SIGNAL(startCalc(int)), this, SLOT(progresBar(int)) );
    connect( thCalc, SIGNAL(done(int)), this, SLOT(progresBar(int)) );
    connect( thCalc, SIGNAL(progressChanged(int)), this, SLOT(progresBar(int)) );
    connect( thCalc, SIGNAL(doneResult(QString,int)), this, SLOT(calcResult(QString,int)) );
    connect( ui->action_CalcStop, SIGNAL(triggered()), thCalc, SLOT(stopCalc()) );
    thCalc->start();
}

/* ----------------------------------- РАСЧЕТ - Записать результат ----------------------------------- */
void MainWindow::calcResult(QString strResult, int idTask)
{
    QSqlQuery query(QSqlDatabase::database("project"));

    query.prepare ("UPDATE TaskCalc SET Path = :path WHERE id = :id");
    query.bindValue(":id",  idTask);
    query.bindValue(":path", strResult);
    query.exec();

    modelTask->submitAll();
}

/* ----------------------------------- РАСЧЕТ - Заморозка UI ----------------------------------- */
void MainWindow::UICalc(bool b)
{
    ui->tableView_Prto->setEnabled(b);
    ui->action_CalcStart->setEnabled(b);
    ui->dockWidget_antBd->setEnabled(b);
    ui->menu_Prto->setEnabled(b);
    ui->menu_Task->setEnabled(b);
    ui->toolBar_Edit->setEnabled(b);
}

/* ----------------------------------- РАСЧЕТ - Прогресс бар ----------------------------------- */
void MainWindow::progresBar(int value)
{
    if(value == -1)
    {
        pbar->setEnabled(true);
        UICalc(false);
    }
    else if(value == 101)
    {
        pbar->setValue(0);
        UICalc(true);
        pbar->setEnabled(false);
    }
    else
        pbar->setValue(value);
}

// ----------------------------------- РАСЧЕТ - Параметры расчета - Сохранить ----------------------------------- //
void MainWindow::signal_SendOptions(QString qsSendOptions)
{
    QSqlRecord recordPrmt = modelOptions->record(0);
    recordPrmt.setValue("Koef", qsSendOptions.replace(",",".").toFloat());
    modelOptions->setRecord(0,recordPrmt);
    recordPrmt.clear();
    modelOptions->submitAll();

    sbKoef->setText( "Коэф. " + QSqlRecord(modelOptions->record(0)).value("Koef").toString() );
}


/// ---------------------------------------------------------------------------------------------------------------- //
/// ----------------------------------------------------------- Графика  ------------------------------------------- //
/// ---------------------------------------------------------------------------------------------------------------- //
/* ------- Загрузить ЗОЗ ------- */
void MainWindow::graphcsOpenZo()
{
    QString strFileName = QFileDialog::getOpenFileName(this, tr("Открыть файл расчета"), "", tr("ЗОЗ (*.dat);;"));

    if(!strFileName.isEmpty())
    {
        ViewPlot *vpNew = new ViewPlot(this);
        vpNew->show();
        vpNew->plotLoadZo(strFileName);
        vpNew->plotSquare();
    }
}


/// ---------------------------------------------------------------------------------------------------------------- //
/// ----------------------------------------------------------- Справка  ------------------------------------------  //
/// ---------------------------------------------------------------------------------------------------------------- //
void MainWindow::helpPpcCsv()
{
    QString strName = QFileDialog::getSaveFileName(this,
             tr("Сохранить как"), "PPC",
             tr("CSV (*.csv);;All Files (*)"));
    if(strName.isEmpty())
        return;

    QFile filePPC(strName);
    if(filePPC.open(QIODevice::WriteOnly))
    {
        QTextStream txtstrem(&filePPC);
        txtstrem.setCodec("Windows-1251");
        txtstrem << QString::fromWCharArray(L"Сектор;Модель антенны;Частота;Размер;КУ;Азимут;"
                                            "X;Y;Высота;Угол наклона;Мощность;");
        filePPC.close();
    }
}

// --------------------------------------------------- О Программе --------------------------------------------------- //
void MainWindow::helpAbout()
{
    QString strAbout;
    strAbout = " <!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"\http://www.w3.org/TR/REC-html40/strict.dtd\"> \
                <html><head><meta name=qrichtext content=1 /> \
                <style type=text/css> \
                body { font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal; margin:0px;}\
                p, li { white-space: pre-wrap; margin:0px; -qt-block-indent:0; text-indent:0px; }\
                </style> \
                </head> \
                <body> \
                <p>SanPasport 0.1 (Тестовая версия)</p> \
                <p>Автор Владимир Kansept</p> \
                <p>email:<a href=\"mailto:kansept@yandex.ru\">kansept@yandex.ru</a></p> \
                <p><a href=\"http://www.soft.kansept.ru/\">www.soft.kansept.ru</a></p> \
                </body></html>";

    QMessageBox *about = new QMessageBox();
    about->setInformativeText(strAbout);
    about->exec();
    delete about;


//    int zPrd(24.5);
//    int zTask(10);

//    float pntX(10);
//    float pntZ(zPrd - zTask);
//    float AngleV(0);

//    if(pntZ != 0)
//    {
//        AngleV = (atan2(pntX, pntZ)*180) / 3.14;
//        AngleV = -(AngleV - 90);
//        if(AngleV < 0) { AngleV = AngleV + 360; }
//    }
//    else
//        AngleV = (pntX >= 0)? 0 : 180;


//    qDebug() << pntZ << AngleV;
}

/* ------- Открыть ситуационный план ------- */
void MainWindow::graphcsSitPlan()
{
    win_SitPlan->show();
}


// ------------------------------------ Кавычки ------------------------------------ /
QString MainWindow::quotedStr(const QString str)
{
    QString result;
    return result.append("'").append(str).append("'");
}
