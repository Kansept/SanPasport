#include "SanPasport.h"
#include "ui_mainwindow.h"
#include "ViewPlot/viewplot.h"
#include "Delegates/DelegateTaskDescription.h"
#include "Delegates/DelegateTaskPath.h"
#include "Delegates/DelegateStatus.h"
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


using namespace std;

// ------- Глобальные переменные ------- //
QModelIndex G_qmiindex;
QFile G_fOpenProj;
QString G_strDbPath;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Thread = new ThreadCalcZoz();

    // Статус бар
    initStatusBar();

    enableUI(false);

    ui->toolBar_File->layout()->setSpacing(2);
    ui->toolBar_Edit->layout()->setSpacing(2);
    ui->toolBar_Graphics->layout()->setSpacing(2);

    // ------- Диалоги ------- //
    dialogTaskZoz = new Dialog_TaskZo(this);
    dlg_TaskVs    = new Dialog_TaskVs(this);
    dlg_TaskPoint = new Dialog_TaskPoint(this);
    dlg_EditAnt   = new Dialog_EditAnt(this);
    dlg_Parametrs = new Dialog_Parametrs(this);
    dlg_Preferens = new Dialog_Preferens(this);
    win_SitPlan   = new SitPlan(this);

    // ФАЙЛ
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
    // ПРТО
    connect( ui->action_PrtoSaveAs, SIGNAL(triggered()), this, SLOT(saveAsPattern()) );    // Сохранить ПРТО в файл
    connect( ui->action_PrtoLoadPPC, SIGNAL(triggered()), SLOT(loadAntennaPPC()) );        // Загружаем РРС из фала
    connect( ui->tableView_Antennas, SIGNAL(keyPressed(int,int)), SLOT(AntennaKeyPresed(int, int)) );  // Горячие клавиши
    connect( ui->tableView_Antennas, SIGNAL(customContextMenuRequested(QPoint)), SLOT(contextMenuAntenna(QPoint))); // Контекстное меню
    connect( ui->action_PrtoAddPPC, SIGNAL(triggered()), SLOT(addAntennaPPC()) );          // Добавить РРС
    connect( ui->action_prtoEnabled, SIGNAL(triggered()), this, SLOT(enableAntenna()) );   // Вкл./Выкл. антенну
    connect( ui->action_PrtoEnableAll, SIGNAL(triggered()), SLOT(enableAllAntennas()) );    // Включить все антенны
    connect( ui->action_PrtoDisableAll, SIGNAL(triggered()), SLOT(disableAllAntennas()) );  // Отключить все антенны
    connect( ui->action_PrtoAdd, SIGNAL(triggered()), this, SLOT(OpenFileAntenna()) );
    connect( ui->tableView_Antennas, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editAntenna()) );
    connect( ui->action_EditAnt, SIGNAL(triggered()), this, SLOT(editAntenna()) );
    connect( ui->tableView_Antennas, SIGNAL(clicked(QModelIndex)), this, SLOT(selected_ant()) );
    connect( this, SIGNAL(sendIdEditAnt(Antenna)), dlg_EditAnt, SLOT(insertDataForm(Antenna)) );
    connect( dlg_EditAnt, SIGNAL(sendEditAntFromDlgEdit(Antenna)), this, SLOT(saveEditantPattern(Antenna)) );
    connect( ui->action_EditRemove, SIGNAL(triggered()), this, SLOT(removeAntenna()) );
    connect( ui->tableView_Antennas->verticalHeader(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(moveAntenna(int,int,int)) );
    connect( ui->action_PrtoMoveUp, SIGNAL(triggered()), SLOT(moveUpAntenna()) );
    connect( ui->action_PrtoMoveDown, SIGNAL(triggered()), SLOT(moveDownAntenna()) );
    connect( ui->action_PrtoExportCSV, SIGNAL(triggered()), SLOT(exportAntennasToCsv()) );
    // ЗАДАНИЯ
    connect( ui->action_TaskView, SIGNAL(triggered()), this, SLOT(taskView()) );          // Показать задание
    connect( ui->action_TaskDelete, SIGNAL(triggered()), this, SLOT(taskRemove()) );      // Удалить задание
    connect( ui->action_TaskEnable, SIGNAL(triggered()), this, SLOT(taskEnable()) );      // Вкл/Выкл задание
    connect( ui->action_TaskEnableAll, SIGNAL(triggered()), SLOT(taskEnableAll()) );      // Включить все задания
    connect( ui->action_TaskDisableAll, SIGNAL(triggered()), SLOT(taskDisableAll()) );    // Отключить все задания
    connect( ui->action_TaskEdit, SIGNAL(triggered()), this, SLOT(taskEdit()) );          // Редактировать задание
    connect( ui->tableView_Task, SIGNAL(doubleClicked(QModelIndex)), SLOT(taskEdit()) );  // Редактировать задание     
    connect( ui->action_TaskCalcZo, SIGNAL(triggered()), dialogTaskZoz, SLOT(show()) );      // Добавить задание - Зона ограничения
    connect( dialogTaskZoz, SIGNAL(sendTaskZo()), SLOT(taskInsert()) );        // Добавить задание - Зона ограничения
    connect( ui->action_TaskCalcVS, SIGNAL(triggered()), dlg_TaskVs, SLOT(show())  );     // Добавить задание - Вертикальное сечение
    connect( dlg_TaskVs, SIGNAL(sendTaskVs()), SLOT(taskInsert()) );        // Добавить задание - Вертикальное сечение
    connect( ui->action_TaskCalcEmpPoint, SIGNAL(triggered()), dlg_TaskPoint, SLOT(show()) ); // Добавить задание - Расчет в точке
    connect( dlg_TaskPoint, SIGNAL(sendTaskPoint()), SLOT(taskInsert()) );      // Добавить задание - Расчет в точке
    connect( ui->action_TaskZoFromPrto, SIGNAL(triggered()), SLOT(taskZozFromAntennas()) );             // ЗО сечения на основе ПРТО
    connect( ui->action_TaskVertFromPrto, SIGNAL(triggered()), this, SLOT(taskVertFromPrto()) );  // Верткальные сечения на основе ПРТО
    connect( ui->tableView_Task, SIGNAL(keyPressed(int,int)), SLOT(taskKeyPresed(int, int)) );
    connect( ui->tableView_Task, SIGNAL(customContextMenuRequested(QPoint)), SLOT(contextMenuTask(QPoint)) ); // Контекстное меню
    connect( ui->tableView_Task->verticalHeader(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(taskMoved(int,int,int)) );
    connect( ui->action_TaskMoveUp, SIGNAL(triggered()), SLOT(taskMoveUp()) );
    connect( ui->action_TaskMoveDown, SIGNAL(triggered()), SLOT(taskMoveDown()) );
    connect( ui->actionTasksVertSort, SIGNAL(triggered()), SLOT(sortTasks()) );
    // Параметры проекта
    connect( ui->action_Param, SIGNAL(triggered()), this, SLOT(showDialogParametrs()) );
    connect( dlg_Parametrs, SIGNAL(SendOptionsFromDlgOption(QString)), SLOT(signal_SendOptions(QString)) );
    connect( ui->action_EditDublAnt, SIGNAL(triggered()), this, SLOT(dublicateAntenna()) );
    connect( this, SIGNAL(destroyed()), this, SLOT(fileClose()) );
    connect( sbKoef, SIGNAL(clicked()), this, SLOT(showDialogParametrs()) );
    // База данных
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
    // Графики
    connect( ui->action_GraphcsLoadZo, SIGNAL(triggered()), this, SLOT(graphcsOpenZo()) );   // Открыт файл ЗО
    connect( ui->action_SitPlan, SIGNAL(triggered()), this, SLOT(graphcsSitPlan()) );        // Открыть ситуационный план
    // Расчет
    connect( ui->action_CalcStart, SIGNAL(triggered()), SLOT(calcStart()) );
    connect( ui->action_CalcStop, SIGNAL(triggered()), SLOT(calcStop()) );
    // Справка
    connect( ui->action_HelpPpcCsv, SIGNAL(triggered()), SLOT(helpPpcCsv()) );
    connect( ui->action_HelpAbout, SIGNAL(triggered()), SLOT(helpAbout()) );
    // Разрешаем Контекстное меню
    ui->tableView_Antennas->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView_Task->setContextMenuPolicy(Qt::CustomContextMenu);

    QSettings *settingSanPasport = new QSettings( (QCoreApplication::applicationDirPath())
                                              + "//configSanPasport.ini",QSettings::IniFormat );
    settingSanPasport->beginGroup("SanPasport");
    G_strDbPath = settingSanPasport->value("DbPath", "").toString();
    settingSanPasport->endGroup();

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


void MainWindow::initStatusBar()
{
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
}


// ----------------------------------- Деструктор ----------------------------------- //
MainWindow::~MainWindow()
{
    delete ui;
}


/* ----------------------------------- Контекстное меню ----------------------------------- */
void MainWindow::contextMenuAntenna(const QPoint &pos)
{
    if (ui->tableView_Antennas->selectionModel()->selectedIndexes().size() > 0)
        ui->menu_Prto->exec( ui->tableView_Antennas->mapToGlobal(pos) );
}


void MainWindow::contextMenuTask(const QPoint &pos)
{
    if (ui->tableView_Task->selectionModel()->selectedIndexes().size() > 0)
        ui->menu_Task->exec( ui->tableView_Task->mapToGlobal(pos) );
}


/* ----------------------------------- Закрытие окна ----------------------------------- */
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (ui->action_FileSave->isEnabled() == false)
        return;

    QMessageBox msgBox(QMessageBox::Question, tr("Закрыть проект"), tr("Сохранить изменения в проекте?"),
                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setButtonText(QMessageBox::Yes, trUtf8("Да"));
    msgBox.setButtonText(QMessageBox::No, trUtf8("Нет"));
    msgBox.setButtonText(QMessageBox::Cancel, trUtf8("Отмена"));

    int iMessage(msgBox.exec());

    if (iMessage == QMessageBox::No) {
        event->accept();
    } else if (iMessage == QMessageBox::Cancel) {
        event->ignore();
    } else {
        fileSave();
        event->accept();
    }
}


// ----------------------------------- Реализация Drag'n'Drop ----------------------------------- //
void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}


// ----------------------------------- Реализация Drag'n'Drop ----------------------------------- //
void MainWindow:: dropEvent(QDropEvent* event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString fileName = urls.first().toLocalFile();

    if (fileName.isEmpty()) {
        return;
    } else {
        if (fileName.split(".").last().toLower() == "rto" and ui->menu_Prto->isEnabled() == false)
            fileImportPkaemo4(fileName);
        else if (fileName.split(".").last().toLower() == "msi" and ui->menu_Prto->isEnabled() == true)
           AntennaFromFile(fileName);
    }
}


// ----------------------------------- Включить/Выключить UI ----------------------------------- //
void MainWindow::enableUI(bool b)
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
    Project::init();
    fileLoadModel(QSqlDatabase::database("project"));
}


/* ------- Файл -> Открыть ------- */
void MainWindow::fileOpen()
{
    if (ui->menu_Prto->isEnabled())
        if (!fileClose())
            return;

    QString fOpen = QFileDialog::getOpenFileName( this, tr("Открыть проект ПКАЭМО"), "",
                                                  tr("All support files (*.spp);;") );
    if (fOpen.isEmpty())
        return;

    fileOpen(fOpen);
}


void MainWindow::fileOpen(const QString pathFile)
{
    Project::init();
    Project::saveAs(pathFile, false);
    fileLoadModel(Project::getDatabase());

    G_fOpenProj.setFileName(pathFile);
    setWindowTitle("SanPasport - " + pathFile.split("/").last());
}



/* ------- Загрузка модели ------- */
void MainWindow::fileLoadModel(const QSqlDatabase db)
{
    modelOptions = new QSqlTableModel(0,db.database("project"));
    modelOptions->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelOptions->setTable("options");
    modelOptions->select();

    // modelPrto
    modelAntenna = new ModelAntennas(0,db.database("project"));
    modelAntenna->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelAntenna->setTable("antennas");
    modelAntenna->select();

    // Устанавливаем названия столбцов
    modelAntenna->setHeaderData(AntennaHeader::Id, Qt::Horizontal, "ИД");
    modelAntenna->setHeaderData(AntennaHeader::Enabled, Qt::Horizontal, "Вкл");
    modelAntenna->setHeaderData(AntennaHeader::Name, Qt::Horizontal, "Название");
    modelAntenna->setHeaderData(AntennaHeader::Owner, Qt::Horizontal, "Сектор");
    modelAntenna->setHeaderData(AntennaHeader::Frequency, Qt::Horizontal, "Частота");
    modelAntenna->setHeaderData(AntennaHeader::Gain, Qt::Horizontal, "КУ");
    modelAntenna->setHeaderData(AntennaHeader::Height, Qt::Horizontal, "Размер");
    modelAntenna->setHeaderData(AntennaHeader::Polarization, Qt::Horizontal, "Поляр.");
    modelAntenna->setHeaderData(AntennaHeader::PowerTotal, Qt::Horizontal, "Мощность \n на входе");
    modelAntenna->setHeaderData(AntennaHeader::PowerTrx, Qt::Horizontal, "Мощность \n передатчика");
    modelAntenna->setHeaderData(AntennaHeader::CountTrx, Qt::Horizontal, "Кол-во \n передатчков");
    modelAntenna->setHeaderData(AntennaHeader::FeederLeght, Qt::Horizontal, "Длина \n фидера");
    modelAntenna->setHeaderData(AntennaHeader::FeederLoss, Qt::Horizontal, "Потери");
    modelAntenna->setHeaderData(AntennaHeader::Ksvn, Qt::Horizontal, "КСВН");
    modelAntenna->setHeaderData(AntennaHeader::LossOther, Qt::Horizontal, "Другие \n потери");
    modelAntenna->setHeaderData(AntennaHeader::X, Qt::Horizontal, "X");
    modelAntenna->setHeaderData(AntennaHeader::Y, Qt::Horizontal, "Y");
    modelAntenna->setHeaderData(AntennaHeader::Z, Qt::Horizontal, "Высота");
    modelAntenna->setHeaderData(AntennaHeader::Azimut, Qt::Horizontal, "Азимут");
    modelAntenna->setHeaderData(AntennaHeader::Tilt, Qt::Horizontal, "Угол \n наклона");
    modelAntenna->setHeaderData(AntennaHeader::Sort, Qt::Horizontal, "Порядок");
    modelAntenna->setHeaderData(AntennaHeader::RadHorizontal, Qt::Horizontal, "ДН Гориз");
    modelAntenna->setHeaderData(AntennaHeader::RadVertical, Qt::Horizontal, "ДН Верт");

    ui->tableView_Antennas->setModel(modelAntenna);

    ui->tableView_Antennas->hideColumn(AntennaHeader::Id);
    ui->tableView_Antennas->hideColumn(AntennaHeader::Polarization);
    ui->tableView_Antennas->hideColumn(AntennaHeader::PowerTrx);
    ui->tableView_Antennas->hideColumn(AntennaHeader::CountTrx);
    ui->tableView_Antennas->hideColumn(AntennaHeader::FeederLeght);
    ui->tableView_Antennas->hideColumn(AntennaHeader::FeederLoss);
    ui->tableView_Antennas->hideColumn(AntennaHeader::Ksvn);
    ui->tableView_Antennas->hideColumn(AntennaHeader::LossOther);
    ui->tableView_Antennas->hideColumn(AntennaHeader::Sort);
    ui->tableView_Antennas->hideColumn(AntennaHeader::RadHorizontal);
    ui->tableView_Antennas->hideColumn(AntennaHeader::RadVertical);
    ui->tableView_Antennas->hideColumn(AntennaHeader::Type);
    ui->tableView_Antennas->hideColumn(AntennaHeader::Pdu);

    ui->tableView_Antennas->setItemDelegateForColumn(AntennaHeader::Enabled, new DelegateStatus);
    ui->tableView_Antennas->hideColumn(AntennaHeader::Id);

    // Ширина названия столбцов
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Id, 30);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Enabled, 50);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Name, 200);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Owner, 100);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Frequency, 50);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Gain, 50);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Height, 50);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Polarization, 50);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::PowerTotal, 75);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::PowerTrx, 40);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::CountTrx, 40);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::FeederLeght, 40);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::FeederLoss, 50);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Ksvn, 60);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::LossOther, 150);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::X, 40);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Y, 40);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Z, 50);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Azimut, 50);
    ui->tableView_Antennas->horizontalHeader()->resizeSection(AntennaHeader::Tilt, 60);
    // Сортировка
    ui->tableView_Antennas->verticalHeader()->setSectionsMovable(true);
    ui->tableView_Antennas->sortByColumn(AntennaHeader::Sort, Qt::AscendingOrder);
    ui->tableView_Antennas->setSortingEnabled(false);

    // modelTask
    modelTask = new ModelTasks(0,db.database("project"));
    modelTask->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelTask->setTable("tasks");
    modelTask->select();
    ui->tableView_Task->setModel(modelTask);
    ui->tableView_Task->setItemDelegateForColumn(TaskHeader::Enabled, new DelegateStatus);
    ui->tableView_Task->setItemDelegateForColumn(TaskHeader::Params, new DelegateTaskDescription);
    ui->tableView_Task->setItemDelegateForColumn(TaskHeader::Path, new DelegateTaskPath);

    modelTask->setHeaderData(TaskHeader::Id, Qt::Horizontal, "ИД");
    modelTask->setHeaderData(TaskHeader::Enabled, Qt::Horizontal, "Вкл");
    modelTask->setHeaderData(TaskHeader::Type, Qt::Horizontal, "Тип");
    modelTask->setHeaderData(TaskHeader::Params, Qt::Horizontal, "Параметры");
    modelTask->setHeaderData(TaskHeader::Path, Qt::Horizontal, "Статус");
    modelTask->setHeaderData(TaskHeader::Sort, Qt::Horizontal, "Порядок");

    ui->tableView_Task->horizontalHeader()->resizeSection(TaskHeader::Enabled, 40);
    ui->tableView_Task->horizontalHeader()->resizeSection(TaskHeader::Params, 1000);
    ui->tableView_Task->hideColumn(TaskHeader::Id);
    ui->tableView_Task->hideColumn(TaskHeader::Type);
    ui->tableView_Task->hideColumn(TaskHeader::Sort);
    // Сортировка
    ui->tableView_Task->verticalHeader()->setSectionsMovable(true);
    ui->tableView_Task->sortByColumn(TaskHeader::Sort, Qt::AscendingOrder);
    ui->tableView_Task->setSortingEnabled(false);

    sbKoef->setText( "Коэф. " + QSqlRecord(modelOptions->record(0)).value("Koef").toString() );

    enableUI(true);
}

QString MainWindow::currentPath()
{
    return G_fOpenProj.fileName();
}

/* ------- Файл -> Сохранить ------- */
void MainWindow::fileSave()
{
    if (G_fOpenProj.fileName() == "") {
        fileSaveAs();
        return;
    } else {
        Project::saveAs(G_fOpenProj.fileName(), true);
    }
}


/* ------- Файл -> Сохранить как -------*/
void MainWindow::fileSaveAs()
{
    QString strSaveProject = QFileDialog::getSaveFileName(this,
             tr("Сохранить как..."), "untitled",
             tr("SanPasport (*.spp);;All Files (*)"));

    Project::saveAs(strSaveProject, true);

    G_fOpenProj.setFileName(strSaveProject);
    setWindowTitle("SanPasport - " + strSaveProject.split("/").last());
}


/* -------  Сохранить результаты расчётов -------*/
void MainWindow::fileSaveCalc()
{
    if (G_fOpenProj.exists()) {
        QFile flTask;
        QStringList strlPath (G_fOpenProj.fileName().replace("\\","/").split("/"));
        strlPath.removeLast();
        QString strPathNew (strlPath.join("/")+"/Result/");

        if (!(QDir(strPathNew).exists()))
            QDir().mkdir(strPathNew);

        QVector<Task> tasks (Project::getTasks());

        for (int i=0;  i < tasks.count(); i++)
        {
            QString strFileName (tasks.at(i).Path);
            strFileName.replace("\\","/");
            flTask.setFileName(strFileName);

            if (flTask.exists())
                if ( !(flTask.copy(strPathNew + strFileName.split("/").last())) ) {
                    QMessageBox msgBox;
                    msgBox.setText("Ошибка сохранения!");
                    msgBox.exec();
                    return;
                }
        }
    } else {
        QMessageBox msgBox;
        msgBox.setText("Проект должен быть сохранён!");
        msgBox.exec();
    }
}


/* ------- Импорт в ПКАЭМО -------*/
void MainWindow::fileImportPkaemo4_DialogOpen()
{
    QString f = QFileDialog::getOpenFileName(
                this,
                tr("Открыть проект ПКАЭМО"), "",
                tr("All support files (*.rto);;")
    );
    if (!f.isEmpty())
        fileImportPkaemo4(f);
}


// ------------------------------ Импорт данных из ПК АЭМО 4 ------------------------------ //
void MainWindow::fileImportPkaemo4(const QString pathFile)
{
    Project::importFromPkaemo(pathFile);
    fileLoadModel(Project::getDatabase());
    modelOptions->submitAll();
}


/* ------- Экспорт данных в ПК АЭМО 4 ------- */
void MainWindow::fileExportPkaemo4()
{
    QString fileCopy = QFileDialog::getSaveFileName(this, tr("Save As"), "united", tr("(*.rto);; Files PKAEMO(*)"));
    if (!fileCopy.isEmpty())
        Project::exportToPkaemo(fileCopy);
}


// ----------------------------------- Закрыть проект ----------------------------------- //
bool MainWindow::fileClose()
{
    QMessageBox msgBox(QMessageBox::Question, tr("Сообщение"), tr("Вы действительно хотите закрыть проект?"),
                       QMessageBox::Yes | QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, trUtf8("Да"));
    msgBox.setButtonText(QMessageBox::No, trUtf8("Нет"));
    int iMessage (msgBox.exec());

    if (iMessage == QMessageBox::No)
        return false;

    QSqlDatabase db;
    db.database("qt_sql_default_connection").close();
    delete modelAntenna;
    delete modelOptions;
    delete modelTask;
    enableUI(false);

    return true;
}



// ----------------------------------------------------------------------------------------------------------------- //
/// --------------------------------------------------------- ПРТО ------------------------------------------------- //
// ----------------------------------------------------------------------------------------------------------------- //

// ----------------------------------- ПРТО - Вкл.Выкл ----------------------------------- //
void MainWindow::enableAntenna()
{ 
    foreach (QModelIndex index, ui->tableView_Antennas->selectionModel()->selectedRows()) {
        QSqlRecord record = modelAntenna->record( index.row() );
        record.setValue("Enabled", !record.value("Enabled").toBool());
        modelAntenna->setRecord(index.row(), record);
    }
    modelAntenna->submitAll();
}


/* ------- ПРТО - Включить все антенны ------- */
void MainWindow::enableAllAntennas()
{
    for (int row = 0; row < modelAntenna->rowCount(); row++) {
        QSqlRecord record = modelAntenna->record(row);
        record.setValue("Enabled", true);
        modelAntenna->setRecord(row, record);
    }
    modelAntenna->submitAll();
}


/* ------- ПРТО - Отключить все антенны ------- */
void MainWindow::disableAllAntennas()
{
    for (int row = 0; row < modelAntenna->rowCount(); row++) {
        QSqlRecord record = modelAntenna->record(row);
        record.setValue("Enabled", false);
        modelAntenna->setRecord(row, record);
    }
    modelAntenna->submitAll();
}


// ----------------------------------- ПРТО - Диалог открытия диаграммы ----------------------------------- //
void MainWindow::OpenFileAntenna()
{
    QString filePath = QFileDialog::getOpenFileName(
            this,
            tr("Открыть фаил ДН"), "",
            tr("All support files (*.msi *.pln *.txt *.bdn);;"
                "MSI/Nokia (*.msi *.pln);;"
                "Text file (*.txt);;"
                "Inside (*.bdn);;"
                "All files (*)")
    );
    AntennaFromFile(filePath);
}



// ----------------------------------- ПРТО - Открыть файл ----------------------------------- //
bool MainWindow::AntennaFromFile(const QString &f)
{
    if (ui->menu_Prto->isEnabled() == false)
        return false;
    Antenna antenna;

    if (!f.isEmpty()) {
        if ( f.split(".").last().toLower() == "msi"
          || f.split(".").last().toLower() == "pln"
          || f.split(".").last().toLower() == "txt"
        ) {
            antenna.clear();
            antenna.loadMsi(f);
            addAntenna(antenna);
            return true;
        }
        if (f.split(".").last().toLower() == "bdn") {
            // loadedBdn(f);
            return true;
        } else {
            QMessageBox::warning(this, tr("Ошибка"), tr("Не правильный формат файла."));
            return false;
        }
    }

    return true;
}


// ----------------------------------- ПРТО - Сохранить в файл ----------------------------------- //
void MainWindow::saveAsPattern()
{
    if (ui->tableView_Antennas->selectionModel()->selectedIndexes().size() > 0) {
        Antenna saveAntenna;
        saveAntenna = getAntennaFromModel(ui->tableView_Antennas->selectionModel()->selectedRows().first().row());
        QString fileName = QFileDialog::getSaveFileName(this,
                 tr("Save As"), saveAntenna.Name,
                 tr("MSI/Nokia (*.msi);;All Files (*)"));
        if (!fileName.isEmpty())
            saveAntenna.saveAsMsi(fileName);
    }
}


// =================================== ПРТО - Дудлировать =================================== //
void MainWindow::dublicateAntenna()
{
    if (ui->tableView_Antennas->selectionModel()->selectedIndexes().size() < 0)
        return;

    int numLastRow (ui->tableView_Antennas->selectionModel()->selectedRows().last().row());

    foreach (QModelIndex index, ui->tableView_Antennas->selectionModel()->selectedRows()) {
        Antenna antenna = getAntennaFromModel(index.row());
        int lastNumber = antenna.Owner.right(1).toInt();
        if (lastNumber != 0) {
           antenna.Owner = antenna.Owner.left(antenna.Owner.count() - 1).append(QString::number(lastNumber + 1));
        }
        addAntenna( antenna );
    }

    ui->tableView_Antennas->selectRow(numLastRow + 1);
}



// =================================== ПРТО - Удаляем антенну =================================== //
bool MainWindow::removeAntenna()
{  
    if (ui->tableView_Antennas->selectionModel()->currentIndex().row() == -1)
        return false;

    QMessageBox msgBox (QMessageBox::Question, tr("Удалить"), tr("Вы действительно хотите удалить антенну?"),
                       QMessageBox::Yes | QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, trUtf8("Да"));
    msgBox.setButtonText(QMessageBox::No, trUtf8("Нет"));

    int iMessage(msgBox.exec());

    if (iMessage == QMessageBox::No)\
        return false;

    foreach (QModelIndex index, ui->tableView_Antennas->selectionModel()->selectedRows())
        modelAntenna->removeRow(index.row());

    modelAntenna->submitAll();
    return true;
}



// ----------------------------------- Вставить новую антенну из файла ----------------------------------- //
void MainWindow::addAntenna(const Antenna antenna)
{
    Project::addAntenna(antenna);
    modelAntenna->submitAll();
}


// ----------------------------------- ПРТО - добавить РРС ----------------------------------- //
void MainWindow::addAntennaPPC()
{
    Project::addAntennaPPC();
    modelAntenna->submitAll();
}


// ------------------------------------ Чтение ДН из строки таблицы ------------------------------------ //
Antenna MainWindow::getAntennaFromModel(const int row)
{
    QSqlRecord record = modelAntenna->record(row);

    Antenna antenna;
    antenna.clear();

    antenna.Id = record.value("id").toInt();
    antenna.Enabled = record.value("enabled").toBool();
    antenna.Sort = record.value("sort").toInt();
    antenna.Name = record.value("name").toString();
    antenna.Owner = record.value("owner").toString();
    antenna.Frequency = record.value("frequency").toFloat();
    antenna.Gain = record.value("gain").toFloat();
    antenna.Height = record.value("height").toFloat();
    antenna.Type = record.value("type").toInt();

    antenna.PowerTotal = record.value("power_total").toFloat();
    antenna.PowerTrx = record.value("power_trx").toFloat();
    antenna.FeederLeght = record.value("feeder_leght").toFloat();
    antenna.FeederLoss = record.value("feeder_loss").toFloat();
    antenna.KSVN = record.value("ksvn").toFloat();
    antenna.LossOther = record.value("loss_other").toFloat();
    antenna.CountTrx = record.value("count_trx").toInt();

    antenna.X = record.value("x").toFloat();
    antenna.Y = record.value("y").toFloat();
    antenna.Z = record.value("z").toFloat();
    antenna.Azimut = record.value("azimut").toFloat();
    antenna.Tilt = record.value("tilt").toFloat();

    QStringList qslRadHoriz(record.value("rad_horizontal").toString().split(";"));
    QStringList qslRadVert(record.value("rad_vertical").toString().split(";"));

    for (int i = 0; i < 360; i++) {
        antenna.AzimutHorizontal[i] = i;
        antenna.AzimutVertical[i] = i;
        antenna.RadHorizontal[i] = qslRadHoriz.at(i).toFloat();
        antenna.RadVertical[i] = qslRadVert.at(i).toFloat();
    }
    return antenna;
}


// ----------------------------------- Открываем форму для редактирования антенны ---------------------------------- //
void MainWindow::editAntenna()
{
    if (ui->tableView_Antennas->selectionModel()->currentIndex().row() == -1)
        return;

    emit sendIdEditAnt(getAntennaFromModel(ui->tableView_Antennas->currentIndex().row()));
    dlg_EditAnt->show();
}



/* ------ Получаем данныем из формы ------ */
void MainWindow::saveEditantPattern(Antenna antenna)
{
    Project::editAntenna(antenna);
    modelAntenna->submitAll();
    ui->tableView_Antennas->selectRow(G_qmiindex.row());
}


/* ------ Загружаем РРС из фала ------ */
void MainWindow::loadAntennaPPC()
{
    QString fOpen = QFileDialog::getOpenFileName(this,
                                             tr("Открыть файл с РРС"), "",
                                             tr("All support files (*.csv);;")
                                             );
    QFile fPPC(fOpen);
    if (fOpen.isEmpty())
        return;

    QString strAll;

    QStringList srtlAll;
    QStringList srtlLine;
    Antenna antenna;

    if (fPPC.open(QIODevice::ReadOnly)) {
        QTextStream txtstrem (&fPPC);
        txtstrem.setCodec("Windows-1251");
        strAll = txtstrem.readAll();
        strAll.replace(",",".");
        srtlAll = strAll.split(QRegExp("\\n"), QString::SkipEmptyParts);

        for(int i=1; i < srtlAll.size(); i++) {
            srtlLine = srtlAll.at(i).split(QRegExp(";"));
            antenna.clear();

            antenna.Owner = srtlLine.at(0);               // Сектор
            antenna.Name = srtlLine.at(1);                // Модель антенны
            antenna.Frequency = srtlLine.at(2).toFloat();      // Частота
            antenna.Height = srtlLine.at(3).toFloat();    // Размер
            antenna.Gain = srtlLine.at(4).toFloat();      // КУ
            antenna.Azimut = srtlLine.at(5).toFloat();    // Азимут
            antenna.X = srtlLine.at(6).toFloat();         // X
            antenna.Y = srtlLine.at(7).toFloat();         // Y
            antenna.Z = srtlLine.at(8).toFloat();         // Высота
            antenna.Tilt = srtlLine.at(9).toFloat();      // Угол наклона
            antenna.PowerTrx = srtlLine.at(10).toFloat(); // Мощность
            antenna.PowerTotal = antenna.calcPower();
            antenna.Type = 15;
            addAntenna(antenna);
        }
        fPPC.close();
    }
}

void MainWindow::selected_ant()
{
    G_qmiindex = ui->tableView_Antennas->selectionModel()->selectedRows().at(0);
}


/* ----------------------------------- ПРТО - Переместить вверх ----------------------------------- */
void MainWindow::moveUpAntenna()
{
    int currentRow (ui->tableView_Antennas->currentIndex().row());
    if (currentRow != 0) {
        Antenna antennaOld (getAntennaFromModel(currentRow));
        antennaOld.Sort = currentRow - 0.5;
        Project::editAntenna(antennaOld);
        modelAntenna->submitAll();

        for (int row = 0; row < modelAntenna->rowCount(); row++) {
            Antenna antenna(getAntennaFromModel(row));
            antenna.Sort = row + 1;
            Project::editAntenna(antenna);
        }
        modelAntenna->submitAll();

        ui->tableView_Antennas->clearSelection();
        ui->tableView_Antennas->selectRow(currentRow - 1);
    }
}


/* ----------------------------------- ПРТО - Переместить вниз ----------------------------------- */
void MainWindow::moveDownAntenna()
{
    int currentRow (ui->tableView_Antennas->currentIndex().row());

    if (modelAntenna->rowCount() > currentRow + 1) {
        Antenna antennaOld (getAntennaFromModel(currentRow));

        antennaOld.Sort = currentRow + 2.5;
        Project::editAntenna(antennaOld);
        modelAntenna->submitAll();

        for (int row = 0; row < modelAntenna->rowCount(); row++) {
            Antenna antenna(getAntennaFromModel(row));
            antenna.Sort = row + 1;
            Project::editAntenna(antenna);
        }
        modelAntenna->submitAll();

        ui->tableView_Antennas->clearSelection();
        ui->tableView_Antennas->selectRow(currentRow + 1);
    }
}


// ----------------------------------- ПРТО - Переместить ----------------------------------- //
void MainWindow::moveAntenna(int LogicIndex,int OldVisualIndex, int NewVisualIndex)
{
    ui->tableView_Antennas->verticalHeader()->blockSignals(true);
    ui->tableView_Antennas->verticalHeader()->moveSection(NewVisualIndex, OldVisualIndex);
    ui->tableView_Antennas->verticalHeader()->blockSignals(false);

    if (NewVisualIndex > OldVisualIndex)
        for (int i = 0; i < (NewVisualIndex - OldVisualIndex); i++)
            moveDownAntenna();
    else
        for (int i = 0; i < (OldVisualIndex - NewVisualIndex); i++)
            moveUpAntenna();
}


/* ----------------------------------- ПРТО - Горячие клавиши ----------------------------------- */
void MainWindow::AntennaKeyPresed(int numKey, int numModifierKey)
{
    switch (numKey) {
    case Qt::Key_Delete:
        removeAntenna();
        break;
    case Qt::Key_Insert:
        OpenFileAntenna();
        break;
    case Qt::Key_Space:
        enableAntenna();
        break;
    case Qt::Key_Enter:
        editAntenna();
        break;
    default:
        break;
    }  
    if (numKey == Qt::Key_D and numModifierKey == Qt::ControlModifier) dublicateAntenna();
    else if (numKey == Qt::Key_Up and numModifierKey == Qt::ControlModifier)    moveUpAntenna();
    else if (numKey == Qt::Key_Down and numModifierKey == Qt::ControlModifier)  moveDownAntenna();
    else if (numKey == Qt::Key_Up)   ui->tableView_Antennas->selectRow(ui->tableView_Antennas->currentIndex().row() - 1);
    else if (numKey == Qt::Key_Down) ui->tableView_Antennas->selectRow(ui->tableView_Antennas->currentIndex().row() + 1);
}


void MainWindow::exportAntennasToCsv()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), "project", tr("CSV (*.csv);;All Files (*)"));
    if (fileName.isEmpty())
        return;

    Project::saveAntennasToCsv(fileName);
}


/// ---------------------------------------------------------------------------------------------------------------- //
/// -------------------------------------------------------- БД ---------------------------------------------------- //
/// ---------------------------------------------------------------------------------------------------------------- //
/* ----------------------------------- БД - Просмотр ДН ----------------------------------- */
void MainWindow::dbPrtoView()
{
    QProcess *proc = new QProcess(this);

    if (ui->treeView_Db->isVisible())
        proc->start( "DnView.exe \"" + (modelDb->filePath(ui->treeView_Db->selectionModel()->currentIndex())) + "\"" );
    else if (ui->tableWidgetDbSearch->isVisible() and ui->tableWidgetDbSearch->currentRow() != -1)
       proc->start( "DnView.exe \"" + ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 1)->text()
                    + "/" + ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 0)->text() + "\"" );
}


/* ------- ПРТО - Открыть из БД ------- */
void MainWindow::dbPrtoInsert()
{
    QString strFile;

    if (ui->treeView_Db->isVisible())
        strFile = modelDb->filePath(ui->treeView_Db->selectionModel()->currentIndex()) ;
    else if (ui->tableWidgetDbSearch->isVisible() and ui->tableWidgetDbSearch->currentRow() != -1)
        strFile = ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 1)->text() + "/" +
                  ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 0)->text();

    if (!QFileInfo(strFile).isDir()) {
        if ( QFileInfo(strFile).fileName().split(".").last().toLower() == "pdf" ) {
            dbOpenOutside();
        } else {
            AntennaFromFile(strFile );
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

        if (ui->treeView_Db->isVisible())
            finfBrowse.setFile(modelDb->filePath(ui->treeView_Db->selectionModel()->currentIndex()));
        else if (ui->tableWidgetDbSearch->isVisible() and ui->tableWidgetDbSearch->currentRow() != -1)
            finfBrowse.setFile(ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 1)->text() + "/" +
                               ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 0)->text());

        if (finfBrowse.isDir())
            procBrowse->start( "explorer.exe /e, \"" + finfBrowse.absoluteFilePath().replace("/","\\") + "\"" );
        else if (finfBrowse.isFile())
            procBrowse->start( "explorer.exe /select, \"" + finfBrowse.absoluteFilePath().replace("/","\\") + "\"" );
}


/* ----------------------------------- БД - Открыть в проводнике ----------------------------------- */
void MainWindow::dbOpenOutside()
{
    QProcess *procOpen = new QProcess(this);
    QFileInfo finfBrowse;

    if (ui->treeView_Db->isVisible())
        finfBrowse.setFile(modelDb->filePath(ui->treeView_Db->selectionModel()->currentIndex()));
    else if (ui->tableWidgetDbSearch->isVisible() and ui->tableWidgetDbSearch->currentRow() != -1)
        finfBrowse.setFile(ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 1)->text() + "/" +
                           ui->tableWidgetDbSearch->item(ui->tableWidgetDbSearch->currentRow(), 0)->text());
    if (finfBrowse.isFile())
        procOpen->start( "explorer.exe /e, \"" + finfBrowse.absoluteFilePath().replace("/","\\") + "\"" );
}


/* ----------------------------------- БД - Открыть папку ----------------------------------- */
void MainWindow::dbSetRoot(QModelIndex indexRoot)
{
    if (modelDb->isDir(indexRoot)) {
        ui->treeView_Db->setRootIndex(indexRoot);
        ui->toolButtonDb_Up->setEnabled(true);
    }
}


/* ----------------------------------- БД - Перейти на уровень вверх ----------------------------------- */
void MainWindow::dbUp()
{
    if (ui->treeView_Db->isVisible()) {
        if (modelDb->filePath(ui->treeView_Db->rootIndex()) != G_strDbPath)
            ui->treeView_Db->setRootIndex(modelDb->parent(ui->treeView_Db->rootIndex()));
    } else if (ui->tableWidgetDbSearch->isVisible()) {
        dbSearchClear();
    }
    if (modelDb->filePath(ui->treeView_Db->rootIndex()) == G_strDbPath)
        ui->toolButtonDb_Up->setEnabled(false);
}


/* ----------------------------------- БД - Фильтр ----------------------------------- */
void MainWindow::dbFilter(bool b)
{
    if (b)
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
    if (ui->lineEditDb_Search->text().isEmpty()) {
        dbSearchClear();
        return;
    }

    ui->toolButtonDb_Up->setEnabled(true);
    ui->tableWidgetDbSearch->setVisible(true);
    ui->treeView_Db->setVisible(false);

    QString dir (modelDb->filePath(ui->treeView_Db->rootIndex()));
    QString name (ui->lineEditDb_Search->text());
    QStringList strlResult;

    QDirIterator it(dir, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        it.next();
        if (it.fileName().contains(name))
            if (QFileInfo(it.filePath()).isFile())
                strlResult.append(it.filePath());
    }

    ui->tableWidgetDbSearch->setColumnCount(2);
    ui->tableWidgetDbSearch->setRowCount(strlResult.size());
    ui->tableWidgetDbSearch->setHorizontalHeaderLabels(QStringList() << "Файл" << "Путь");

    for (int i=0; i<strlResult.size(); i++) {
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
Task MainWindow::taskFromModel(int row)
{
    QSqlRecord record = modelTask->record(row);
    Task task;

    task.Id = record.value("id").toInt();
    task.Enabled = record.value("enabled").toBool();
    task.Type = record.value("type").toInt();
    task.Params = record.value("params").toString();
    task.Path = record.value("path").toString();
    task.Sort = record.value("sort").toInt();

    return task;
}


void MainWindow::taskView()
{
    QSqlRecord record = modelTask->record( ui->tableView_Task->selectionModel()->selectedRows().first().row() );
    QString strPath;

    if (QFile(record.value("path").toString()).exists())
        strPath = record.value("path").toString();
    else
        strPath = QFileInfo(G_fOpenProj).absolutePath() + "/Result/"
                + QFileInfo(record.value("path").toString()).fileName();

    if (QFile(strPath).exists()) {
        ViewPlot *vpNew = new ViewPlot(this);
        vpNew->plotLoadZo(strPath);
        vpNew->show();
        vpNew->plotSquare();
    }
}


// ----------------------------------- ЗАДАНИЕ - Вкл/Выкл ----------------------------------- //
void MainWindow::taskEnableAll()
{
    for (int row = 0; row < modelTask->rowCount(); row++) {
        QSqlRecord record = modelTask->record(row);
        record.setValue("enabled", true);
        modelTask->setRecord(row, record);
    }
    modelTask->submitAll();
}


// ----------------------------------- ЗАДАНИЕ - Включить все задания ----------------------------------- //
void MainWindow::taskEnable()
{
    int currentRow (ui->tableView_Task->selectionModel()->selectedRows().last().row());
    foreach (QModelIndex index, ui->tableView_Task->selectionModel()->selectedRows()) {
         QSqlRecord record = modelTask->record( index.row() );
         record.setValue("enabled", !record.value("enabled").toBool());
         modelTask->setRecord(index.row(), record);
    }
    modelTask->submitAll();
    ui->tableView_Task->selectRow(currentRow);
}


// ----------------------------------- ЗАДАНИЕ - Отключить все задания ----------------------------------- //
void MainWindow::taskDisableAll()
{
    for (int row = 0; row < modelTask->rowCount(); row++) {
        QSqlRecord record = modelTask->record(row);
        record.setValue("enabled", false);
        modelTask->setRecord(row, record);
    }
    modelTask->submitAll();
}


// ----------------------------------- ЗАДАНИЕ - Редактировать ----------------------------------- //
void MainWindow::taskEdit()
{ 
    Task task (taskFromModel(ui->tableView_Task->currentIndex().row()));

    if (task.Type == TaskType::Zoz) {
        dialogTaskZoz->show();
        dialogTaskZoz->insertData(task);
    } else if (task.Type == TaskType::Vs) {
        dlg_TaskVs->show();
        dlg_TaskVs->insertData(task);
    } else if (task.Type == TaskType::Point) {
        dlg_TaskPoint->show();
        dlg_TaskPoint->insertData(task);
    }
}

// ----------------------------------- ЗАДАНИЕ - Удаляем задание ----------------------------------- //
bool MainWindow::taskRemove()
{   
    QMessageBox msgBox (QMessageBox::Question, tr("Удалить"), tr("Вы действительно хотите удалить задание?"),
                        QMessageBox::Yes | QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, trUtf8("Да"));
    msgBox.setButtonText(QMessageBox::No, trUtf8("Нет"));

    int iMessage(msgBox.exec());

    if (iMessage == QMessageBox::No)
        return false;

    foreach (QModelIndex index, ui->tableView_Task->selectionModel()->selectedRows())
        modelTask->removeRow(index.row());

    modelTask->submitAll();
    return true;
}


/* ------- Горизонтальные сечения на основе ПРТО ------- */
void MainWindow::taskZozFromAntennas()
{
    Antenna antenna;
    QList<float> lstTaskZo;

    for (int i=0; i<modelAntenna->rowCount(); i++) {
        antenna.clear();
        antenna = getAntennaFromModel(i);
        if ( !lstTaskZo.contains(antenna.Z) )
            lstTaskZo.append(antenna.Z);
    }
    qStableSort(lstTaskZo.begin(), lstTaskZo.end(), qGreater<float>());

    QStringList strlHeght;
    for (int j=0; j<lstTaskZo.count(); j++)
       strlHeght.append( QString::number(lstTaskZo.at(j)) );

    dialogTaskZoz->show();
    dialogTaskZoz->insertHeight( strlHeght.join(" ") );
}

/* ------- Верткальные сечения на основе ПРТО ------- */
void MainWindow::taskVertFromPrto()
{
    Antenna antenna;
    QString taskParams;
    QString strRange;
    Task task;

    QSettings *settingVs = new QSettings( (QCoreApplication::applicationDirPath())
                                          + "//configSanPasport.ini",QSettings::IniFormat );
    settingVs->beginGroup("SanPasport");
    strRange.append( settingVs->value("VsRmin", "0").toString() ).append(";");
    strRange.append( settingVs->value("VsRmax", "100").toString() ).append(";");
    strRange.append( settingVs->value("VsRd", "0.5").toString() ).append(";");
    strRange.append( settingVs->value("VsHmin", "0").toString() ).append(";");
    strRange.append( settingVs->value("VsHmax", "40").toString() ).append(";");
    strRange.append( settingVs->value("VsHd", "0.5").toString() ).append(";");
    settingVs->endGroup();
    delete settingVs;

    for (int i=0; i<modelAntenna->rowCount(); i++) {
        antenna.clear();
        taskParams.clear();
        antenna = getAntennaFromModel(i);

        taskParams.append(QString::number(antenna.X)).append(";");
        taskParams.append(QString::number(antenna.Y)).append(";");
        taskParams.append(strRange);
        taskParams.append(QString::number(antenna.Azimut)).append(";");
        taskParams.append("0");

        QSqlQuery qCount (Project::getDatabase());
        qCount.exec("SELECT COUNT(*) FROM tasks WHERE params = '" + taskParams + "'");
        qCount.next();

        if (qCount.value(0).toInt() == 0) {
            task.Id = -1;
            task.Type = 2;
            task.Params = taskParams;

            Project::addTask(task);
            modelTask->submitAll();
        }
    }
}

void MainWindow::sortTasks()
{
    QList<float> listAzimut;
    QVector<Task> tasksVs (Project::getTasks(TaskType::Vs));

    foreach (Task task, tasksVs) {
        float azimut (task.Params.split(";").at(8).toFloat());
        listAzimut.append(azimut);
    }
    qStableSort(listAzimut.begin(), listAzimut.end(), qLess<float>());

    foreach (Task task, tasksVs) {
        task.Sort = listAzimut.indexOf(task.Params.split(";").at(8).toFloat());
        Project::addTask(task);
    }

    int countVs (listAzimut.count());

    QList<float> listHeight;
    QVector<Task> tasksZoz (Project::getTasks(TaskType::Zoz));

    foreach (Task task, tasksZoz) {
        float height (task.Params.split(";").at(6).toFloat());
        listHeight.append(height);
    }
    qStableSort(listHeight.begin(), listHeight.end(), qGreater<float>());

    foreach (Task task, tasksZoz) {
        task.Sort = countVs + listHeight.indexOf(task.Params.split(";").at(6).toFloat());
        Project::addTask(task);
    }

    modelTask->submitAll();
}

// ----------------------------------- ЗАДАНИЕ - Переместить вверх ----------------------------------- //
void MainWindow::taskMoveUp()
{
    int currentRow (ui->tableView_Task->currentIndex().row());
    if (currentRow != 0) {
        Task taskOld (taskFromModel(currentRow));

        taskOld.Sort = currentRow - 0.5;
        Project::addTask(taskOld);
        modelTask->submitAll();

        for (int row = 0; row < modelTask->rowCount(); row++) {
            Task task (taskFromModel(row));
            task.Sort = row + 1;
            Project::addTask(task);
        }
        modelTask->submitAll();

        ui->tableView_Task->clearSelection();
        ui->tableView_Task->selectRow(currentRow - 1);
    }
}


// ----------------------------------- ЗАДАНИЕ - Переместить вниз ----------------------------------- //
void MainWindow::taskMoveDown()
{
    int currentRow (ui->tableView_Task->currentIndex().row());

    if (modelTask->rowCount() > currentRow + 1) {
        Task taskOld (taskFromModel(currentRow));

        taskOld.Sort = currentRow + 2.5;
        Project::addTask(taskOld);
        modelTask->submitAll();

        for (int row = 0; row < modelTask->rowCount(); row++) {
            Task task(taskFromModel(row));
            task.Sort = row + 1;
            Project::addTask(task);
        }
        modelTask->submitAll();

        ui->tableView_Task->clearSelection();
        ui->tableView_Task->selectRow(currentRow + 1);
    }
}


// ----------------------------------- ЗАДАНИЕ - Переместить ----------------------------------- //
void MainWindow::taskMoved(int LogicIndex, int OldVisualIndex, int NewVisualIndex)
{
    ui->tableView_Task->verticalHeader()->blockSignals(true);
    ui->tableView_Task->verticalHeader()->moveSection(NewVisualIndex, OldVisualIndex);
    ui->tableView_Task->verticalHeader()->blockSignals(false);

    if (NewVisualIndex > OldVisualIndex)
        for (int i = 0; i < (NewVisualIndex - OldVisualIndex); i++)
            taskMoveDown();
    else
        for (int i = 0; i < (OldVisualIndex - NewVisualIndex); i++)
            taskMoveUp();
}


void MainWindow::taskKeyPresed(int numKey, int numModifierKey)
{
    switch (numKey) {
    case Qt::Key_Delete:
        taskRemove();
        break;
    case Qt::Key_H:
        dialogTaskZoz->show();
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

    if (numKey == 1056)      dialogTaskZoz->show();
    else if (numKey == 1052) dlg_TaskVs->show();
    else if (numKey == 1047) dlg_TaskPoint->show();
    else if (numKey == Qt::Key_A    and numModifierKey == Qt::ControlModifier) taskView();
    else if (numKey == Qt::Key_Up   and numModifierKey == Qt::ControlModifier) taskMoveUp();
    else if (numKey == Qt::Key_Down and numModifierKey == Qt::ControlModifier) taskMoveDown();
    else if (numKey == Qt::Key_Up)   ui->tableView_Task->selectRow(ui->tableView_Task->currentIndex().row() - 1);
    else if (numKey == Qt::Key_Down) ui->tableView_Task->selectRow(ui->tableView_Task->currentIndex().row() + 1);
}

/// ---------------------------------------------------------------------------------------------------------------- //
/// ---------------------------------------------------------- РАСЧЕТ ---------------------------------------------- //
/// ---------------------------------------------------------------------------------------------------------------- //

// ----------------------------------- РАСЧЕТ - Параметры расчета ----------------------------------- //
void MainWindow::showDialogParametrs()
{
    if (ui->menu_Prto->isEnabled()) {
        dlg_Parametrs->loadParam( QSqlRecord(modelOptions->record(0)).value("Koef").toFloat() );
        dlg_Parametrs->show();
    }
}


void MainWindow::calcStart()
{
    QVector<Task> tasks;
    QVector<Antenna> antennas;
    Task task;
    Antenna antenna;
    ThreadCalcZoz *thCalc = new ThreadCalcZoz();

    for (int i=0; i < modelAntenna->rowCount(); i++) {
        antenna.clear();
        antenna = getAntennaFromModel(i);

        if(antenna.Type == 100 and antenna.Enabled == true)
            antennas.append(antenna);
    }

    for (int i=0; i < modelTask->rowCount(); i++) {
        task.clear();
        task = taskFromModel(i);

        if (task.Type == 1 and task.Enabled == true and task.Params.split(";").last() == "0")
            tasks.append(task);
    }

    thCalc->loadTask(tasks, antennas);
    connect( thCalc, SIGNAL(startCalc(int)), this, SLOT(progresBar(int)) );
    connect( thCalc, SIGNAL(done(int)), this, SLOT(progresBar(int)) );
    connect( thCalc, SIGNAL(progressChanged(int)), this, SLOT(progresBar(int)) );
    connect( thCalc, SIGNAL(doneResult(QString,int)), this, SLOT(calcResult(QString,int)) );
    connect( ui->action_CalcStop, SIGNAL(triggered()), thCalc, SLOT(stopCalc()) );
    thCalc->start();
}

//! TODO пустая функция
void MainWindow::calcStop()
{
}

/* ----------------------------------- РАСЧЕТ - Записать результат ----------------------------------- */
void MainWindow::calcResult(QString strResult, int idTask)
{
    QSqlQuery query (Project::getDatabase());

    query.prepare ("UPDATE tasks SET path = :path WHERE id = :id");
    query.bindValue(":id",  idTask);
    query.bindValue(":path", strResult);
    query.exec();

    modelTask->submitAll();
}

/* ----------------------------------- РАСЧЕТ - Заморозка UI ----------------------------------- */
void MainWindow::UICalc(bool b)
{
    ui->tableView_Antennas->setEnabled(b);
    ui->action_CalcStart->setEnabled(b);
    ui->dockWidget_antBd->setEnabled(b);
    ui->menu_Prto->setEnabled(b);
    ui->menu_Task->setEnabled(b);
    ui->toolBar_Edit->setEnabled(b);
}

/* ----------------------------------- РАСЧЕТ - Прогресс бар ----------------------------------- */
void MainWindow::progresBar(int value)
{
    if (value == -1) {
        pbar->setEnabled(true);
        UICalc(false);
    } else if (value == 101) {
        pbar->setValue(0);
        UICalc(true);
        pbar->setEnabled(false);
    } else {
        pbar->setValue(value);
    }
}

// ----------------------------------- РАСЧЕТ - Параметры расчета - Сохранить ----------------------------------- //
void MainWindow::signal_SendOptions(QString qsSendOptions)
{
    QSqlRecord recordPrmt = modelOptions->record(0);
    recordPrmt.setValue("Koef", qsSendOptions.replace(",", ".").toFloat());
    modelOptions->setRecord(0,recordPrmt);
    recordPrmt.clear();
    modelOptions->submitAll();

    sbKoef->setText("Коэф. " + QSqlRecord(modelOptions->record(0)).value("Koef").toString());
}


/// ---------------------------------------------------------------------------------------------------------------- //
/// ----------------------------------------------------------- Графика  ------------------------------------------- //
/// ---------------------------------------------------------------------------------------------------------------- //
/* ------- Загрузить ЗОЗ ------- */
void MainWindow::graphcsOpenZo()
{
    QString strFileName = QFileDialog::getOpenFileName(this, tr("Открыть файл расчета"), "", tr("ЗОЗ (*.dat);;"));

    if (!strFileName.isEmpty()) {
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
    if (strName.isEmpty())
        return;

    QFile filePPC(strName);
    if (filePPC.open(QIODevice::WriteOnly)) {
        QTextStream txtstrem(&filePPC);
        txtstrem.setCodec("UTF-8");
        txtstrem << QString("Sector;Name;Freq;Size;Gain;Azimut;X;Y;Height;Titl;Power;").toUtf8();
        txtstrem.setGenerateByteOrderMark(false);
        filePPC.close();
    }
}

// --------------------------------------------------- О Программе --------------------------------------------------- //
void MainWindow::helpAbout()
{
    QString textAbout;

    textAbout = " <!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"> \
                <html><head><meta name=qrichtext content=1 /> \
                <style type=text/css> \
                body { font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal; margin:0px;}\
                p, li { white-space: pre-wrap; margin:0px; -qt-block-indent:0; text-indent:0px; }\
                </style> \
                </head> \
                <body> \
                <p>SanPasport 0.2b (Тестовая версия)</p> \
                <p>Автор Владимир Kansept</p> \
                <p>email:<a href=\"mailto:kansept@yandex.ru\">kansept@yandex.ru</a></p> \
                <p><a href=\"http://www.soft.kansept.ru/\">www.soft.kansept.ru</a></p> \
                </body></html>";

    QMessageBox *about = new QMessageBox();
    about->setInformativeText(textAbout);
    about->exec();
    delete about;
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
