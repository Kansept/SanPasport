#include "sitplan.h"
#include "ui_sitplan.h"
#include "../mainwindow.h"

#include <QFileDialog>
#include <QFontDialog>
#include <QDebug>
#include <QPixmap>
#include <qmath.h>
#include <math.h>
#include <QPicture>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QToolBar>
#include <QTextCursor>
#include <QVariant>
#include <QSvgGenerator>
#include <QBuffer>
#include <QGraphicsGridLayout>
#include <QScreen>
#include <QGraphicsEffect>
#include <QMargins>
#include <QMimeData>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QList>
#include <QtConcurrent>
#include <QProcess>
#include <QProgressDialog>

//  Переменные в файле конфигуорации ------- >>
ConfigSitPlan g_confConfig;

SitPlan::SitPlan(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SitPlan)
{
    ui->setupUi(this);

    if(!(QDir(QCoreApplication::applicationDirPath() + "/Cache").exists())) {
        QDir().mkdir(QCoreApplication::applicationDirPath() + "/Cache"); }

    // Создаём SVG файл шаблона если его нет
    if(!(QFile(QCoreApplication::applicationDirPath() + "/Cache/PixmapSvg.svg").exists()))
    {
        QFile filePixmapSvg("://resourse/default/PixmapSvg.svg");
        QByteArray qbaPixmapSvg;
        if(filePixmapSvg.open(QIODevice::ReadOnly))
            qbaPixmapSvg = filePixmapSvg.readAll();

        filePixmapSvg.close();
        filePixmapSvg.setFileName(QCoreApplication::applicationDirPath() + "/Cache/PixmapSvg.svg");

        if(filePixmapSvg.open(QIODevice::WriteOnly))
            filePixmapSvg.write(qbaPixmapSvg);
        filePixmapSvg.close();
    }

    // Строка состояния
    sb1 = new QLabel(statusBar());
    sb2 = new QLabel(statusBar());
    sb3 = new QLabel(statusBar());
    sb4 = new QLabel(statusBar());
    sb5 = new QLabel(statusBar());
    pbar = new QProgressBar(statusBar());

    ui->statusBar->setSizeGripEnabled(false);
    ui->statusBar->addWidget(sb1, 1);
    ui->statusBar->addWidget(sb2, 1);
    ui->statusBar->addWidget(sb3, 1);
    ui->statusBar->addWidget(sb4, 1);
    ui->statusBar->addWidget(sb5, 4);
    ui->statusBar->addWidget(pbar, 1);

    pbar->setAlignment(Qt::AlignRight);
    pbar->setMaximumHeight(15);
    pbar->setVisible(false);

    sb1->setText("X = ");
    sb2->setText("Y = ");
    sb3->setText("R = ");
    sb4->setText("Угол ");
    sb5->setText(" ");

    // ------- Загрузка и применение файла конфигурации -------  //
    settings = new QSettings((QCoreApplication::applicationDirPath()) + "//configSitPlan.ini",QSettings::IniFormat);
    settingRead();

    scene = new QGraphicsScene(this);
    scene->setSceneRect(QRectF(0, 0, cmToPixel(42), cmToPixel(29.7)));
    scene->setBackgroundBrush(QPixmap("://resourse/default/background.png"));   // Изображение для фона
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setAlignment(Qt::AlignCenter| Qt::AlignHCenter);

    QPen blackPen(Qt::white);
    blackPen.setWidth(1);
    QRectF CanvasS(0, 0, cmToPixel(42), cmToPixel(29.7));
    rectangle = scene->addRect(CanvasS,blackPen, Qt::white);
    rectangle->setZValue(-1);

    svgSitPlan = new QGraphicsSvgItem("");
    svgSitPlan->setCachingEnabled(true);

    text = scene->addText("");
    text->setFlag(QGraphicsItem::ItemIsMovable);
    text->setFlag(QGraphicsItem::ItemIsSelectable);
    text->setPos(scene->width()/3,scene->height()/20);

    plotZoz = new PlotZoz();
    //  ------------------------------------------------------------- Скрываем шкалы деления и текст <<

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(plotZoz);
    widget = new QWidget;
    // Прозрачность
    widget->setAttribute(Qt::WA_NoSystemBackground);
    widget->setAttribute(Qt::WA_TranslucentBackground);
    widget->setAttribute(Qt::WA_OpaquePaintEvent);
    widget->setStyleSheet("background-color: green"); // transparent
    widget->setStyleSheet("opacity: 0");
    widget->setLayout(layout);
    widget->layout()->setContentsMargins(0,0,0,0);

    proxyPlot = scene->addWidget(widget);
    proxyPlot->setFlag(QGraphicsProxyWidget::ItemIsMovable);
    proxyPlot->setFlag(QGraphicsProxyWidget::ItemIsSelectable);

    pixCenter = new QGraphicsSvgItem("://resourse/default/center.svg");
    pixCenter->setFlag(QGraphicsItem::ItemIsSelectable);
    pixCenter->setFlag(QGraphicsItem::ItemIsMovable);
    scene->addItem(pixCenter);
    pixCenter->setPos(scene->sceneRect().width()/2,scene->sceneRect().height()/2);

    scene->update();
    ui->graphicsView->update();
    plotZoz->replot();

    const float xScale = ( ((physicalDpiX()+logicalDpiX())/2)/300.0 );
    const float yScale = ( ((physicalDpiY()+logicalDpiY())/2)/300.0 );
    ui->graphicsView->scale(xScale, yScale);

    // QAction
    PageLandscape = new QAction(QIcon("://resourse/default/page_landscape.png"), tr("Альбом"), this);
    PageLandscape->setCheckable(true);
    PageLandscape->setChecked(true);
    PagePortrait = new QAction(QIcon("://resourse/default/page_portrait.png"), tr("Портрет"), this);
    PagePortrait->setCheckable(true);
    TitleInsertHeight = new QAction(QIcon("://resourse/default/page_portrait.png"), tr("Высота среза"), this);
    TitleInsertScale = new QAction(QIcon("://resourse/default/page_portrait.png"), tr("Масштаб"), this);

    // Виджеты для Панели инструментов
    comboBox_CanvasSize = new QComboBox;
    comboBox_CanvasSize->addItems(QStringList() << "A4" << "A3");
    comboBox_CanvasSize->setCurrentIndex(1);

    comboBox_SetZOScale = new QComboBox;
    comboBox_SetZOScale->setEditable(true);
    comboBox_SetZOScale->addItems(QStringList() << "1:1000" << "1:800" << "1:500");
    g_fZOScale = 1;

    comboBox_sceneScale = new QComboBox;
    comboBox_sceneScale->setEditable(true);
    for (int i = 25; i <= 200; i = i + 25)
        comboBox_sceneScale->addItem(QString().setNum(i).append("%"));
    QIntValidator *validatorScale = new QIntValidator(1, 200, this);
    comboBox_sceneScale->setValidator(validatorScale);
    comboBox_sceneScale->setMaximumWidth(100);
    comboBox_sceneScale->setCurrentIndex(3);

    zoomSlider = new QSlider;
    zoomSlider->setRange(0, 200);
    zoomSlider->setValue(100);
    zoomSlider->setOrientation(Qt::Horizontal);
    zoomSlider->setTickPosition(QSlider::TicksRight);
    zoomSlider->setMaximumWidth(100);
    zoomSlider->setSingleStep(20);

    lineEdit_CurrentPage = new QLineEdit("0");
    lineEdit_CurrentPage->setMaximumWidth(50);

    label_AllPage = new QLabel(" /0 ");
    QFont fontlabel_AllPage;

  //  label_AllPage->setPointSizeF(10);
    label_AllPage->setFont(fontlabel_AllPage);

    QMenu *menuTitleInsert = new QMenu(ui->toolButton_TitleInsertVar);
    menuTitleInsert->addAction(TitleInsertHeight);
    menuTitleInsert->addAction(TitleInsertScale);
    ui->toolButton_TitleInsertVar->setMenu(menuTitleInsert);

    // Панель инструментов
    ToolBarDocument = addToolBar(tr("Документ"));
    ToolBarDocument->addWidget(comboBox_CanvasSize);
    ToolBarDocument->addAction(PageLandscape);
    ToolBarDocument->addAction(PagePortrait);
    ToolBarDocument->addWidget(comboBox_SetZOScale);
    ToolBarDocument->addAction(ui->action_Print);
    ToolBarDocument->layout()->setSpacing(2);

    ToolBarImage = addToolBar(tr("Изображение"));
    ToolBarImage->addAction(ui->action_SitPlanLoad);
    ToolBarImage->addAction(ui->action_SitPlanLook);
    ToolBarImage->addAction(ui->action_SitPlanForCanvas);
    ToolBarImage->layout()->setSpacing(2);

    ToolBarNavigator = addToolBar(tr("Навигация"));
    ToolBarNavigator->addAction(ui->action_PageFirst);
    ToolBarNavigator->addAction(ui->action_PageBack);
    ToolBarNavigator->addWidget(lineEdit_CurrentPage);
    ToolBarNavigator->addWidget(label_AllPage);
    ToolBarNavigator->addAction(ui->action_PageNext);
    ToolBarNavigator->addAction(ui->action_PageLast);
    ToolBarNavigator->layout()->setSpacing(2);

    ToolBarView = addToolBar(tr("Вид"));
    ToolBarView->addAction(ui->action_ModeDrag);
    ToolBarView->addAction(ui->action_ModeSelect);
    ToolBarView->addSeparator();
    ToolBarView->addAction(ui->action_ViewZoomOut);
    ToolBarView->addWidget(zoomSlider);
    ToolBarView->addAction(ui->action_ViewZoomIn);
    ToolBarView->addAction(ui->action_ViewOriginal);
    ToolBarView->addAction(ui->action_ViewBest);
    ToolBarView->layout()->setSpacing(2);

    // Модель - Представление
    modelFilesZO = new QSqlTableModel(0, QSqlDatabase::database("project"));
    modelFilesZO->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelFilesZO->setTable("filesZO");
    modelFilesZO->select();
    ui->tableView_Pages->setModel(modelFilesZO);
    // Название колонок
    modelFilesZO->setHeaderData(0, Qt::Horizontal, "ИД");
    modelFilesZO->setHeaderData(1, Qt::Horizontal, "Вкл");
    modelFilesZO->setHeaderData(2, Qt::Horizontal, "Высота");
    modelFilesZO->setHeaderData(3, Qt::Horizontal, "Файл");
    modelFilesZO->setHeaderData(4, Qt::Horizontal, "Данные");
    // Размер колонок
    ui->tableView_Pages->horizontalHeader()->resizeSection(0,20);
    ui->tableView_Pages->horizontalHeader()->resizeSection(1,30);
    ui->tableView_Pages->horizontalHeader()->resizeSection(2,60);
    ui->tableView_Pages->horizontalHeader()->resizeSection(3,120);
    ui->tableView_Pages->horizontalHeader()->resizeSection(4,50);
    // Прячем колонки
    ui->tableView_Pages->hideColumn(0);
    ui->tableView_Pages->hideColumn(1);
    ui->tableView_Pages->hideColumn(4);
    ui->tableView_Pages->hideColumn(5);

    modelOptionsSitPlan = new QSqlTableModel(0, QSqlDatabase::database("project"));
    modelOptionsSitPlan->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelOptionsSitPlan->setTable("OptionsSitPlan");
    modelOptionsSitPlan->select();

    // Диалоги
    dlg_ConfigDocument = new Dialog_ConfigDocument(this);
    dlg_ConfigGrid = new Dialog_ConfigGrid(this);
    dlg_ConfigZo = new Dialog_ConfigZo(this);

    // Файл
    connect( ui->action_Exit, SIGNAL(triggered()), this, SLOT(close()) );   // Закрыть окно
    // Зона ограниченной застройки
    connect( ui->action_ZoClear, SIGNAL(triggered()), this, SLOT(zozClear()) );
    connect( ui->toolButton_ZoAdd, SIGNAL(clicked()), this, SLOT(zozOpen()) );
    connect( ui->action_ZO_Load, SIGNAL(triggered()), this, SLOT(zozOpen()) );                        // Загрузить ЗО
    connect( ui->action_ZO_Delete, SIGNAL(triggered()), this, SLOT(zozRemove()) );                    // Удалить ЗО
    connect( ui->toolButton_ZozRemove, SIGNAL(clicked()), SLOT(zozRemove()) );
    connect( ui->action_GridFont, SIGNAL(triggered()), this, SLOT(showConfigGridFont()) );
    connect( ui->action_VisibleGrid, SIGNAL(triggered(bool)), this, SLOT(zozGridVisible(bool)) );     // Показать/Скрыть - Сетка
    connect( scene, SIGNAL(selectionChanged()), this, SLOT(zozMove()) );                              // Если выбран объект
    connect( ui->tableView_Pages, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(zozActivate(QModelIndex)) );
    connect( ui->tableView_Pages, SIGNAL(keyPressed(int,int)), SLOT(zozKeyPresed(int, int)) );     // Горячие клавиши
    connect( comboBox_SetZOScale, SIGNAL(currentIndexChanged(QString)), this, SLOT(zozPlotScale(QString)) );
    connect(this, SIGNAL(pbarVal(int)), this, SLOT(zozProgressBar(int)));
    connect( ui->graphicsView, SIGNAL(sendMousePoint(QPointF)), this, SLOT(zozPositionMouse(QPointF)) );
    connect( ui->action_ZozLoadFromProject, SIGNAL(triggered()), SLOT(zozLoadFromProject()) );

    // Ситуационный план
    connect( ui->action_PixRotate90Po, SIGNAL(triggered()), this, SLOT(planRotate90CW()) );
    connect( ui->action_PixRotate90Protiv, SIGNAL(triggered()), this, SLOT(planRotate90CWW()) );
    connect( ui->action_PixRotate180, SIGNAL(triggered()), this, SLOT(planRotate180()) );
    connect( ui->action_SitPlanLoad, SIGNAL(triggered()), this, SLOT(planOpen()) );              // Загрузить СитПлан
    connect( ui->action_SitPlanForHeight, SIGNAL(triggered()), this, SLOT(planForHeight()) );    // Подогнать по высоте СитПлан
    connect( ui->action_SitPlanForWidth, SIGNAL(triggered()), this, SLOT(planForWidth()) );      // Подогнать по ширине СитПлан
    connect( ui->action_SitPlanForCanvas, SIGNAL(triggered()), this, SLOT(planForCanvas()) );    // Подогнать по ширине СитПлан
    connect( ui->action_SitPlanLook, SIGNAL(triggered(bool)), this, SLOT(planLook(bool)) );      // Закрепить СитПлан
    connect( ui->action_SitPlanAuto, SIGNAL(triggered(bool)), this, SLOT(planAutoSize(bool)) );  // Автоматически подгонят СитПлан
    connect( ui->action_VisbleSitPlan, SIGNAL(triggered(bool)), this, SLOT(planVisible(bool)) );    // Показать/Скрыть - СитПлан
    //Заголовок
    connect( ui->toolButton_TitleFont,SIGNAL(clicked()), SLOT(titleFontChange()) );                               // Изменить шрифт
    connect( ui->action_TitleFont, SIGNAL(triggered()), SLOT(titleFontChange()) );                                // Изменить шрифт
    connect( ui->action_VisibleTitle, SIGNAL(triggered(bool)), this, SLOT(titleVisible(bool)) );                  // Показать/Скрыть - Заголовок
    connect( ui->action_FontMask, SIGNAL(toggled(bool)), this, SLOT(titleCursorPositionChanged(bool)) );          // Маскировка текста
    connect( ui->toolButton_TitleFontMask, SIGNAL(clicked(bool)), this, SLOT(titleCursorPositionChanged(bool)) ); // Маскировка текста
    connect( TitleInsertHeight, SIGNAL(triggered()), this, SLOT(titleInsertHeight()) );
    connect( TitleInsertScale, SIGNAL(triggered()), this, SLOT(titleInsertScale()) );
    connect( ui->textEdit_Title, SIGNAL(cursorPositionChanged()), SLOT(titleCursorPositionChanged()) );
    // Страницы
    connect( ui->toolButton_ZoMoveUp, SIGNAL(clicked()), this, SLOT(pageMoveTop()) );
    connect( ui->toolButton_ZoMoveDown, SIGNAL(clicked()), this, SLOT(pageMoveBottom()) );
    connect( ui->toolButton_ZoMovePageDown, SIGNAL(clicked()), this, SLOT(pageMoveDown()) );
    connect( ui->toolButton_ZoMovePageUp, SIGNAL(clicked()), this, SLOT(pageMoveUp()) );
    connect( ui->action_PageFirst, SIGNAL(triggered()), this, SLOT(pageViewFirst()) );             // Первая страница
    connect( ui->action_PageBack, SIGNAL(triggered()), this, SLOT(pageViewBack()) );               // Предыдущяя страница
    connect( ui->action_PageNext, SIGNAL(triggered()), this, SLOT(pageViewNext()) );               // Следующяя страница
    connect( ui->action_PageLast, SIGNAL(triggered()), this, SLOT(pageViewLast()) );               // Последняя страница
    connect( lineEdit_CurrentPage, SIGNAL(textChanged(QString)), this, SLOT(pageView(QString)) );  // Установить страницу
    connect( comboBox_CanvasSize, SIGNAL(currentIndexChanged(QString)), this, SLOT(pageSize(QString)) );   // Выбор размера
    connect( PagePortrait, SIGNAL(triggered()), this, SLOT(pagePortrait()) );
    connect( PageLandscape, SIGNAL(triggered()), this, SLOT(pageLandscape()) );
    // Печать
    connect( ui->action_Print,SIGNAL(triggered()), this, SLOT(printPreview()) );
    connect( ui->action_PrintQuick, SIGNAL(triggered()), this, SLOT(printQuick()) );
    connect( ui->action_ExportPDF, SIGNAL(triggered()), this, SLOT(printPDF()) );    // Экспорт в PDF
    // Диалоги
    connect( ui->action_DocumentConfig, SIGNAL(triggered()), this, SLOT(showConfigPage()) );
    connect( dlg_ConfigDocument, SIGNAL(sendOkDialog_ConfigPage(QVariant)), this, SLOT(settingApplyPage(QVariant)) );
    connect( ui->action_ZO_Param, SIGNAL(triggered()),  this, SLOT(showConfigZo()) );
    connect( ui->pushButton_ParamZO, SIGNAL(clicked()), this, SLOT(showConfigZo()) );
    connect( dlg_ConfigZo, SIGNAL(sendOkDialog_ConfigZo(QVariant)), this, SLOT(settingApplyZoz(QVariant)) );
    connect( ui->action_GridConfig, SIGNAL(triggered()), this, SLOT(showConfigGrid()) );
    connect( dlg_ConfigGrid, SIGNAL(sendOkDialog_ConfigGrid(QVariant)), this, SLOT(settingApplyGrid(QVariant)) );
    // Вид
    connect( zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(viewZoomCustom()) );   // Сменить размер ползунком
    connect( ui->action_ModeSelect, SIGNAL(triggered()), this, SLOT(viewModeSelect()) );   // Режим выбора
    connect( ui->action_ModeDrag, SIGNAL(triggered()), this, SLOT(viewModeDrag()) );       // Режим перемещения по полотну
    connect( ui->action_ViewOriginal, SIGNAL(triggered()), this, SLOT(viewZoomOriginal()) );   // Оригинальный размер
    connect( ui->action_ViewZoomIn, SIGNAL(triggered()), this, SLOT(viewZoomIn()) );           // Увеличить
    connect( ui->action_ViewZoomOut, SIGNAL(triggered()), this, SLOT(viewZoomOut()) );         // Уменьшить
    connect( ui->action_ViewBest, SIGNAL(triggered()), this, SLOT(viewZoomBest()) );               // Вписать в окно
    // Настройки
    connect( ui->action_SettingsSave, SIGNAL(triggered()), this, SLOT(settingWrite()) );  // Сохранить текущие настройки как по умолчанию

    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setInteractive(false);
    ui->graphicsView->setMouseTracking(true);

    QSqlRecord recordOpt = modelOptionsSitPlan->record(0);
    ui->textEdit_Title->setText( recordOpt.value("TITLE").toString() );
    ui->action_ViewBest->setVisible(false);
    comboBox_sceneScale->setVisible(false);

    settingApply();
    viewZoomIn();
    viewZoomIn();

    ui->menu_Rotate->setEnabled(false);
    ui->menu_SitPlanScale->setEnabled(false);
    ui->action_SitPlanLook->setEnabled(false);
    ui->action_SitPlanClear->setEnabled(false);
    ui->action_SitPlanForCanvas->setEnabled(false);

    // Включаем Drag'n'Drop
    setAcceptDrops(true);
    printEnabled(false);

    ui->graphicsView->setMouseTracking(true);

    proxyPlot->setVisible(false);

    ui->graphicsView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( ui->graphicsView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(contextMenuGrapicsZoz(QPoint)) );
}


/* ------ Деструктор ------ */
SitPlan::~SitPlan()
{
    delete ui;
}


/* ------ Загрузка модели при открытии ------ */
void SitPlan::showEvent(QShowEvent *event)
{
    if(! event->isAccepted())
        return;

    modelFilesZO = new QSqlTableModel(0, QSqlDatabase::database("project"));
    modelFilesZO->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelFilesZO->setTable("files_zoz");
    modelFilesZO->select();
    ui->tableView_Pages->setModel(modelFilesZO);
    ui->tableView_Pages->setItemDelegateForColumn(2, new mDelegateHeight);
    // Название колонок
    modelFilesZO->setHeaderData(0, Qt::Horizontal, "ИД");
    modelFilesZO->setHeaderData(1, Qt::Horizontal, "Вкл");
    modelFilesZO->setHeaderData(2, Qt::Horizontal, "Высота");
    modelFilesZO->setHeaderData(3, Qt::Horizontal, "Файл");
    modelFilesZO->setHeaderData(4, Qt::Horizontal, "Данные");
    // Размер колонок
    ui->tableView_Pages->horizontalHeader()->resizeSection(0,20);
    ui->tableView_Pages->horizontalHeader()->resizeSection(1,30);
    ui->tableView_Pages->horizontalHeader()->resizeSection(2,60);
    ui->tableView_Pages->horizontalHeader()->resizeSection(3,120);
    ui->tableView_Pages->horizontalHeader()->resizeSection(4,50);
    // Прячем колонки
    ui->tableView_Pages->hideColumn(0);
    ui->tableView_Pages->hideColumn(1);
    ui->tableView_Pages->hideColumn(4);
    ui->tableView_Pages->hideColumn(3);
    ui->tableView_Pages->hideColumn(5);

    modelOptionsSitPlan = new QSqlTableModel(0, QSqlDatabase::database("project"));
    modelOptionsSitPlan->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelOptionsSitPlan->setTable("options_sitplan");
    modelOptionsSitPlan->select();
}


void SitPlan::contextMenuGrapicsZoz(QPoint pos)
{
    QMenu menuContext;
    menuContext.addAction(ui->action_ModeDrag);
    menuContext.addAction(ui->action_ModeSelect);
    menuContext.addSeparator();
        QMenu *menuContextLoad = new QMenu("Загрузить");
        menuContextLoad->addAction(ui->action_SitPlanLoad);
        menuContextLoad->addSeparator();
        menuContextLoad->addAction(ui->action_ZO_Load);
        menuContextLoad->addAction(ui->action_ZozLoadFromProject);
    menuContext.addMenu(menuContextLoad);
    menuContext.addMenu(ui->menu_ViewVisible);
    menuContext.exec( ui->graphicsView->mapToGlobal(pos) );
}


/* ------- Реализация Drag'n'Drop ------- */
void SitPlan::dragEnterEvent(QDragEnterEvent* event)
{
    if(event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}


/* ------- Реализация Drag'n'Drop ------- */
void SitPlan::dropEvent(QDropEvent* event)
{
    QList<QUrl> urls =event->mimeData()->urls();
    if(urls.isEmpty())
        return;
    QString fileName = urls.first().toLocalFile();

    if(fileName.isEmpty())
        return;
    else
        if(urls.last().toString().split(".").last().toLower() == "jpg")
            planLoad(fileName);

    if(urls.last().toString().split(".").last().toLower() == "dat")
        for(int i = 0; i < urls.size(); i++)
        {
            fileName = urls.at(i).toString();
            zozReading(fileName.replace(0,8,""));
        }
}



/// ------------------------------------------ Ситуационный план ------------------------------------------ ///

/* -------  Открыть файл изображения(Ситуационный план) ------- */
void SitPlan::planOpen()
{
    const QList<QByteArray> imageFormats = QImageWriter::supportedImageFormats();

    QString imageFilter;
    for ( int i = 0; i < imageFormats.size(); i++ )
        imageFilter.append(" ").append("*.").append(imageFormats[i]);

    QString f = QFileDialog::getOpenFileName(this,
                                             tr("Открыть фаил ДН"), "",
                                             "Все поддерживаемые (" + imageFilter + " *.svg" + ");;" + "Images (" + imageFilter + ")" + ";;SVG (*.svg);;PDF(testing) (*.pdf);;Все файлы (*.*)"
                                             );
    if(f.size())
        planLoad(f);
}


/* -------  Загрузить файл изображения(Ситуационный план) ------- */
void SitPlan::planLoad(const QString f)
{
    delete svgSitPlan;
    QFile(QCoreApplication::applicationDirPath() + "/Cache/tempsp.svg").remove();

    const QList<QByteArray> imageFormats = QImageWriter::supportedImageFormats();
    QByteArray qbaExtension;
    qbaExtension.append(f.split(".").last().toLower());

    // ("bmp", "ico", "jpeg", "jpg", "pbm", "pgm", "png", "ppm", "tif", "tiff", "wbmp", "xbm", "xpm")
    if( imageFormats.contains(qbaExtension) )
    {
        QPixmap *pixImg = new QPixmap(f);

        QFile fSvg(QCoreApplication::applicationDirPath() + "/Cache/PixmapSvg.svg");
        fSvg.open(QIODevice::ReadOnly);
        QString strSVG(fSvg.readAll());
        fSvg.close();

        strSVG.replace("%width", QString::number(pixImg->width()));
        strSVG.replace("%height", QString::number(pixImg->height()));
        strSVG.replace("%filepath", f);

        QFile fSvgSave(QCoreApplication::applicationDirPath() + "/Cache/tempsp.svg");
        fSvgSave.open(QIODevice::WriteOnly);

        QTextStream out(&fSvgSave);
        out.setCodec("Windows-1251");
        out << strSVG;
        fSvgSave.close();

        delete pixImg;
    }
    else if(qbaExtension == "svg")
    {
        QFile(f).copy(QCoreApplication::applicationDirPath() + "/Cache/tempsp.svg");
    }
    else if(qbaExtension == "pdf")
    {
        QProcess *proc = new QProcess(this);
        proc->setReadChannel(QProcess::StandardError);
        proc->start( "\"" + QCoreApplication::applicationDirPath() + "/pdf2svg/pdf2svg.exe\" " + "\"" + f + "\"" +
                     " \"" + QCoreApplication::applicationDirPath() + "/Cache/tempsp.svg\" 1" );
        proc->waitForFinished();
        QByteArray qbaError(proc->readAllStandardError());
        if( !(qbaError.isEmpty()) )
            QMessageBox::information(0, "Ошибка!", qbaError);
    }
    svgSitPlan = new QGraphicsSvgItem( QCoreApplication::applicationDirPath() + "/Cache/tempsp.svg" );
    scene->addItem(svgSitPlan);

    svgSitPlan->setZValue(0);
    proxyPlot->setZValue(2);

    svgSitPlan->setFlag(QGraphicsProxyWidget::ItemIsMovable);
    svgSitPlan->setFlag(QGraphicsProxyWidget::ItemIsSelectable);

    if(ui->action_SitPlanAuto->isChecked() == true)
    {
        ui->menu_Rotate->setEnabled(true);
        ui->menu_SitPlanScale->setEnabled(true);
        ui->action_SitPlanLook->setEnabled(true);
        ui->action_SitPlanClear->setEnabled(true);
        ui->action_SitPlanForCanvas->setEnabled(true);

        planForCanvas();
        planLook(true);
        ui->action_SitPlanLook->setChecked(true);
    }
}


/* ------- Подогнаяет СитПлан по высоте ------- */
void SitPlan::planForHeight()
{
    svgSitPlan->setScale( scene->sceneRect().height()/svgSitPlan->boundingRect().height() );
    svgSitPlan->setPos(0,0);
}


/* ------- Подогнаяет СитПлан по ширине ------- */
void SitPlan::planForWidth()
{
    svgSitPlan->setScale(scene->sceneRect().width()/svgSitPlan->boundingRect().width());
    svgSitPlan->setPos(0,0);
}


/* -------  Подогнаяет СитПлан ------- */
void SitPlan::planForCanvas()
{
    if ( scene->sceneRect().height()/svgSitPlan->boundingRect().height() >
         scene->sceneRect().width()/svgSitPlan->boundingRect().width() )
        planForWidth();
    else
        planForHeight();
}


/* -------  Закрепить СитПлан ------- */
void SitPlan::planLook(bool b)
{
    svgSitPlan->setZValue(-0.1);

    if(b == true)
        svgSitPlan->setEnabled(false);
    else
        svgSitPlan->setEnabled(true);
}


/* -------  ------- */
void SitPlan::planAutoSize(bool b)
{
    g_confConfig.SitPlanAuto = b;
}


/* -------  Повернуть по часовой стрелки на 90 гр. ------- */
void SitPlan::planRotate90CW()
{
    svgSitPlan->setRotation(svgSitPlan->rotation() + 90);
}


/* -------  Повернуть против часовой стрелки на 90 гр. ------- */
void SitPlan::planRotate90CWW()
{
    svgSitPlan->setRotation(svgSitPlan->rotation() - 90);
}


/* -------  Повернуть на 180 гр. ------- */
void SitPlan::planRotate180()
{
    svgSitPlan->setRotation(svgSitPlan->rotation() + 180);
}


/* ------- Видимость Ситуационного плана ------- */
void SitPlan::planVisible(bool checked)
{
    svgSitPlan->setVisible(checked);
}



/// ------------------------------------------ Зона ограниченной застройки ------------------------------------------ ///

/* -------  Загрузить файл излучения  ------- */
void SitPlan::zozOpen()
{
    QString f;
    QStringList qslf = QFileDialog::getOpenFileNames(this,
                                                     tr("Открыть фаил ДН"), "",
                                                     tr("ZO support files (*.dat);;Text file (*.txt);;All files (*)")
                                                     );
    // Если нажали отмену прерываем фун
    if(qslf.isEmpty())
        return;

    zozProgressBarMax(qslf.size());
    QtConcurrent::run(this,&SitPlan::zozLoad,qslf);
}


/* -------  загрузка файла ЗОЗ ------- */
void SitPlan::zozLoad(QStringList srtlFiles)
{
    for(int i=0; i<srtlFiles.size(); i++)
        if(QFile(srtlFiles.at(i)).exists())
        {
            zozReading(srtlFiles.at(i));
            printEnabled(true);
            emit pbarVal(i);
        }
    pageViewLast();
}


/* -------  загрузка файлов ЗОЗ из проекта ------- */
void SitPlan::zozLoadFromProject()
{
    QString strProjPath(QFileInfo(MainWindow().currentPath()).absolutePath());
    Project prjLoadTask;
    QVector<Task> vecTask(prjLoadTask.taskFromDb());
    QStringList qslf;

    for(int i=0; i < vecTask.count(); i++)
       if(QFile(vecTask.at(i).Path).exists() and vecTask.at(i).Type == 1)
          qslf.append(vecTask.at(i).Path);
       else if(QFile(strProjPath +"/Result/"+QFileInfo(vecTask.at(i).Path).fileName()).exists() and vecTask.at(i).Type == 1)
          qslf.append(strProjPath +"/Result/"+QFileInfo(vecTask.at(i).Path).fileName());

    if(!qslf.isEmpty())
    {
        zozProgressBarMax(qslf.count());
        QtConcurrent::run(this,&SitPlan::zozLoad,qslf);
    }
}

/* -------  Чтение файла ЗО ------- */
void SitPlan::zozReading(const QString f )
{
    ZO zoZona;
    zoZona.readZoz(f);
    QByteArray qbaValues;

    for(int i=0; i < zoZona.Values.size(); i++)
        qbaValues.append(";").append( QString::number(zoZona.Values.at(i)) );

    qbaValues.remove(0,1);

    QSqlRecord recordAnt = modelFilesZO->record();
    recordAnt.setValue("height", zoZona.Height );
    recordAnt.setValue("path", f );
    recordAnt.setValue("data", qbaValues);
    recordAnt.setValue("size", zoZona.MaxX );
    modelFilesZO->insertRecord(-1,recordAnt);
    recordAnt.clear();
}

/* -------  Чтение файла ЗО по клику ------- */
void SitPlan::zozActivate(const QModelIndex &index)
{
    proxyPlot->setVisible(true);

    QString qsHight;
    QString qsTitle;
    float fZoSide;

    // читаем данные из таблицы
    QSqlRecord record = modelFilesZO->record(index.row());
    qsHight = record.value("hight").toString();
    fZoSide = record.value("size").toFloat();

    QStringList strlValues = record.value("data").toString().split(";", QString::SkipEmptyParts);
    QVector<double> vecValues;

    for(int i=0; i<strlValues.size(); i++) {
        vecValues.append(strlValues.at(i).toDouble());}

    QSqlRecord recordOpt = modelOptionsSitPlan->record(0);
    qsTitle = recordOpt.value("title").toString();
    qsTitle.replace("[H]",qsHight);
    qsTitle.replace("[S]",comboBox_SetZOScale->currentText());
    qsTitle.replace(QRegExp("\\n"), "<br>");

    text->setHtml( qsTitle );
    lineEdit_CurrentPage->blockSignals(true);
    lineEdit_CurrentPage->setText( QString::number(ui->tableView_Pages->currentIndex().row()+1) );
    lineEdit_CurrentPage->blockSignals(false);

    zozPlotDraw(vecValues, fZoSide);
}


/* ------- Нарисовать график ЗОЗ ------- */
void SitPlan::zozPlotDraw(const QVector<double> values, const float fZoSide)
{
    int fZoSideUser(fZoSide);

    if(g_confConfig.GridSize != 0)
        fZoSideUser = g_confConfig.GridSize;

    plotZoz->setAxisFontForAll(g_confConfig.GridAxisFont);
    plotZoz->setAxisScaleForAll(fZoSideUser, g_confConfig.GridStep);
    plotZoz->setData(values);

    if(plotZoz->gridIsVisible())
        proxyPlot->setGeometry( QRect( 0, 0,
                                       cmToPixel(((fZoSideUser*2)/10)) + (plotZoz->size().width()  - plotZoz->canvas()->width()),
                                       cmToPixel(((fZoSideUser*2)/10)) + (plotZoz->size().height() - plotZoz->canvas()->height()) ) );
    else
        proxyPlot->setGeometry( QRect( 0, 0,
                                       cmToPixel((fZoSideUser*2)/10),
                                       cmToPixel((fZoSideUser*2)/10)) );

    proxyPlot->setPos( pixCenter->pos().x() + pixCenter->boundingRect().center().x() - (plotZoz->canvas()->geometry().x() + (plotZoz->canvas()->geometry().width()/2)) * g_fZOScale,
                       pixCenter->pos().y() + pixCenter->boundingRect().center().y() - (plotZoz->canvas()->geometry().y() + (plotZoz->canvas()->geometry().height()/2)) * g_fZOScale );

    zozPlotScale(comboBox_SetZOScale->currentText());
    label_AllPage->setText(" /" + QString::number(modelFilesZO->rowCount()) + " ");
    zozGridVisible(ui->action_VisibleGrid->isChecked());
    plotZoz->replot();
}


/* -------  Удаление файла ЗОЗ  ------- */
void SitPlan::zozRemove()
{
    int intRowSelect = ui->tableView_Pages->currentIndex().row();
    modelFilesZO->removeRow( intRowSelect );
    modelFilesZO->submitAll();
    ui->tableView_Pages->selectRow(intRowSelect);

    if(modelFilesZO->rowCount() == 0)
        zozClear();
}


/* ------- Очистить ------- */
void SitPlan::zozClear()
{
    modelFilesZO->removeRows(0, modelFilesZO->rowCount());
    modelFilesZO->submitAll();
    printEnabled(false);
    proxyPlot->setVisible(false);
}


/* ------- Масштаб ------- */
void SitPlan::zozPlotScale(const QString arg1)
{
    QStringList qslSetScale;
    qslSetScale = arg1.split(":", QString::SkipEmptyParts);

    if(qslSetScale.size() == 2)
        g_fZOScale =( 1000/qslSetScale.at(1).toFloat() );

    proxyPlot->setScale(g_fZOScale);
    proxyPlot->setPos( pixCenter->pos().x() + pixCenter->boundingRect().center().x() - (plotZoz->canvas()->geometry().x() + (plotZoz->canvas()->geometry().width()/2)) * g_fZOScale,
                       pixCenter->pos().y() + pixCenter->boundingRect().center().y() - (plotZoz->canvas()->geometry().y() + (plotZoz->canvas()->geometry().height()/2)) * g_fZOScale );

    g_confConfig.ZoScale = arg1;
}


/* -------  Срабатывает при выбелении объекта ------- */
void SitPlan::zozMove()
{
    if(pixCenter->isSelected())
        proxyPlot->setSelected(true);
}


/* ------- Перемещение мыши ------- */
void SitPlan::zozPositionMouse(QPointF point)
{
    QPointF pxy = ui->graphicsView->mapToScene(point.toPoint());

    double Xpos,Ypos,R,Angle;
    Xpos = pixelToM( -((pixCenter->boundingRect().center().x() + pixCenter->pos().x()) - pxy.toPoint().x()) ) / g_fZOScale;
    Ypos = pixelToM( ((pixCenter->boundingRect().center().y() + pixCenter->pos().y()) - pxy.toPoint().y()) ) / g_fZOScale;
    R =  sqrt(pow(Xpos,2) + pow(Ypos,2));

    sb1->setText( "X = " + QString::number(int(Xpos)) );
    sb2->setText( "Y = " + QString::number(int(Ypos)) );
    sb3->setText( "R = " + QString::number(int(R)) );

    if(Xpos != 0)
    {
        Angle = (atan2(Ypos,Xpos)*180)/3.14;
        Angle = -(Angle - 90);
        if(Angle < 0)
            Angle = Angle + 360;
    }
    else
        Angle = 0;

    sb4->setText( "Угол = " + QString::number( int(Angle)) );
}


/* ------- Видимость Сетки ------- */
void SitPlan::zozGridVisible(bool checked)
{
    if(checked == false)
        plotZoz->setAxisVisible(0,0,0,0);
    else
        plotZoz->setAxisVisible(g_confConfig.GridAxisVisibleLeft,g_confConfig.GridAxisVisibleRight,
                               g_confConfig.GridAxisVisibleBottom,g_confConfig.GridAxisVisibleTop);

    if(modelFilesZO->rowCount() != 0)
        plotZoz->setGridVisible(checked);

    plotZoz->replot();
}


void SitPlan::zozProgressBar(int num)
{
    pbar->setValue(num+1);

    if((pbar->maximum()-1) == num)
    {
        modelFilesZO->submitAll();
        pbar->setVisible(false);
    }
}


void SitPlan::zozProgressBarMax(const int numMax)
{
    pbar->setVisible(true);
    pbar->setRange(0, numMax);
}


/* ----------------------------------- Горячие клавиши ----------------------------------- */
void SitPlan::zozKeyPresed(int numKey, int numModifierKey)
{
    if(numKey == Qt::Key_Delete)  zozRemove();
    else if(numKey == Qt::Key_Up and numModifierKey == Qt::ControlModifier)    pageMoveTop();
    else if(numKey == Qt::Key_Down and numModifierKey == Qt::ControlModifier)  pageMoveBottom();
    else if(numKey == Qt::Key_Up)   ui->tableView_Pages->selectRow(ui->tableView_Pages->currentIndex().row() - 1);
    else if(numKey == Qt::Key_Down) ui->tableView_Pages->selectRow(ui->tableView_Pages->currentIndex().row() + 1);
}



/// --------------------------------------------- Заголовок --------------------------------------------- ///
/* ------- Видимость заголовка ------- */
void SitPlan::titleVisible(bool checked)
{
    text->setVisible(checked);
    ui->action_VisibleTitle->setChecked(checked);
}


/* ------- Вставить высоту ------- */
void SitPlan::titleInsertHeight()
{
    ui->textEdit_Title->textCursor().insertText("[H]");
}


/* ------- Вставить масштаб ------- */
void SitPlan::titleInsertScale()
{
    ui->textEdit_Title->textCursor().insertText("[S]");
}


/* ------- Вставить перенос строки ------- */
void SitPlan::titleInsertBr()
{
    ui->textEdit_Title->textCursor().insertText("<br/>");
}


/* -------  Изменение параметра шрифта  ------- */
void SitPlan::titleFontChange()
{
    bool bOk;
    QFont font = QFontDialog::getFont( &bOk, text->font() );

    if(bOk)
        text->setFont(font);
}


/* -------  Изменение заголовка  ------- */
void SitPlan::titleCursorPositionChanged(bool bMask)
{
    QString qsTitle;

    if(bMask)
        qsTitle.append("<style> p { background: white; } </style> <p> ");

    qsTitle.append( ui->textEdit_Title->toPlainText() );
    qsTitle.replace(QRegExp("\\n"), "<br>");

    text->setHtml(qsTitle);
    modelOptionsSitPlan->setData(modelOptionsSitPlan->index(0,0),qsTitle);
    modelOptionsSitPlan->submitAll();

    ui->toolButton_TitleFontMask->blockSignals(true);
    ui->action_FontMask->blockSignals(true);
        ui->toolButton_TitleFontMask->setChecked(bMask);
        ui->action_FontMask->setChecked(bMask);
    ui->toolButton_TitleFontMask->blockSignals(false);
    ui->action_FontMask->blockSignals(false);
}


/// ------------------------------------------ Страницы ------------------------------------------ ///
/* -------  Размер листа ------- */
void SitPlan::pageSize(const QString &arg1)
{
    QPen blackPen(Qt::black);
    blackPen.setWidth(2);

    if(arg1 == "A4" and PageLandscape->isChecked())
        scene->setSceneRect(0, 0, cmToPixel(29.7), cmToPixel(21));

    if(arg1 == "A4" and PagePortrait->isChecked())
        scene->setSceneRect(0, 0, cmToPixel(21), cmToPixel(29.7));

    if(arg1 == "A3" and PageLandscape->isChecked())
        scene->setSceneRect(0, 0, cmToPixel(42), cmToPixel(29.7));

    if(arg1 == "A3" and PagePortrait->isChecked())
        scene->setSceneRect(0, 0, cmToPixel(29.7), cmToPixel(42));

    rectangle->setRect(scene->sceneRect());

    if(modelFilesZO->rowCount() > 0)
        zozActivate(ui->tableView_Pages->currentIndex());
}


/* -------  Портретная ориентация листа ------- */
void SitPlan::pagePortrait()
{
    if( PagePortrait->isChecked() )
    {
        PageLandscape->setChecked(false);
        pageSize(comboBox_CanvasSize->currentText());
    }
    PagePortrait->setChecked(true);
}


/* -------  Альбомная ориентация листа ------- */
void SitPlan::pageLandscape()
{
    if( PageLandscape->isChecked() )
    {
        PagePortrait->setChecked(false);
        pageSize(comboBox_CanvasSize->currentText());
    }
    PageLandscape->setChecked(true);
}


/* ------- Поднять страницу ------- */
void SitPlan::pageMoveUp()
{
    int intPos;
    intPos = ui->tableView_Pages->currentIndex().row();

    for(int i = 0; i < intPos; i++)
        pageMoveTop();
}


/* ------- Опустить страницу ------- */
void SitPlan::pageMoveDown()
{
    int intPos;
    intPos = modelFilesZO->rowCount() - ui->tableView_Pages->currentIndex().row()-1;

    for(int i = 0; i < intPos; i++)
        pageMoveBottom();
}


/* ------- Поднять страницу в самый вверх ------- */
void SitPlan::pageMoveTop()
{
    int intRow = ui->tableView_Pages->currentIndex().row();
    int intMovedRow;
    if(intRow > 0)
    {
        intMovedRow = intRow-1;

        QSqlRecord recMove = modelFilesZO->record(intRow);
        QSqlRecord recOrig = modelFilesZO->record(intMovedRow);

        modelFilesZO->setData(modelFilesZO->index(intMovedRow, 1), recMove.value(1));
        modelFilesZO->setData(modelFilesZO->index(intMovedRow, 2), recMove.value(2));
        modelFilesZO->setData(modelFilesZO->index(intMovedRow, 3), recMove.value(3));
        modelFilesZO->setData(modelFilesZO->index(intMovedRow, 4), recMove.value(4));
        modelFilesZO->setData(modelFilesZO->index(intMovedRow, 5), recMove.value(5));

        modelFilesZO->setData(modelFilesZO->index(intRow, 1), recOrig.value(1));
        modelFilesZO->setData(modelFilesZO->index(intRow, 2), recOrig.value(2));
        modelFilesZO->setData(modelFilesZO->index(intRow, 3), recOrig.value(3));
        modelFilesZO->setData(modelFilesZO->index(intRow, 4), recOrig.value(4));
        modelFilesZO->setData(modelFilesZO->index(intRow, 5), recOrig.value(5));

        ui->tableView_Pages->selectRow(intMovedRow);
    }
}


/* ------- Опустить страницу в самый низ ------- */
void SitPlan::pageMoveBottom()
{
    int intRow = ui->tableView_Pages->currentIndex().row();
    int intMovedRow;

    if(intRow < modelFilesZO->rowCount()-1)
    {
        intMovedRow = intRow + 1;

        QSqlRecord recMove = modelFilesZO->record(intRow);
        QSqlRecord recOrig = modelFilesZO->record(intMovedRow);

        for(int i=1; i<=5; i++)
            modelFilesZO->setData(modelFilesZO->index(intMovedRow,i),  recMove.value(i));

        for(int i=1; i<=5; i++)
            modelFilesZO->setData(modelFilesZO->index(intRow,i),  recOrig.value(i));

        ui->tableView_Pages->selectRow(intMovedRow);
    }
}


/* -------  Первая страница  ------- */
void SitPlan::pageViewFirst()
{
    ui->tableView_Pages->selectRow(0);
}


/* -------  Предыдущяя страница  ------- */
void SitPlan::pageViewBack()
{
    ui->tableView_Pages->selectRow(ui->tableView_Pages->currentIndex().row()-1);
}


/* -------  Следующяя страница  ------- */
void SitPlan::pageViewNext()
{
    ui->tableView_Pages->selectRow(ui->tableView_Pages->currentIndex().row()+1);
}


/* -------  Последняя страница  ------- */
void SitPlan::pageViewLast()
{
    ui->tableView_Pages->selectRow(modelFilesZO->rowCount()-1);
}


/* -------  Установить страницу  ------- */
void SitPlan::pageView(const QString arg)
{
    int intPageNumber;
    intPageNumber = arg.toInt()-1;
    ui->tableView_Pages->selectRow(intPageNumber);

    if( 0 < intPageNumber and intPageNumber < modelFilesZO->rowCount())
        ui->tableView_Pages->selectRow(intPageNumber);
    else
        pageViewLast();
}



/// --------------------------------------------- Печать --------------------------------------------- ///

/* ------- Предварительный просмотр печати ------- */
void SitPlan::printPreview()
{
    QPrinter printer(QPrinter::HighResolution);

    if(comboBox_CanvasSize->currentText() == "A3") { printer.setPaperSize(QPrinter::A3); }
    if(comboBox_CanvasSize->currentText() == "A4") { printer.setPaperSize(QPrinter::A4); }

    if(PageLandscape->isChecked()) { printer.setOrientation(QPrinter::Landscape); }
    if(PagePortrait->isChecked()) { printer.setOrientation(QPrinter::Portrait); }

    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)), SLOT(printBegin(QPrinter*)));
    preview.exec();
}


/* ------- Печать ------- */
void SitPlan::printBegin(QPrinter *printer)
{
    if(comboBox_CanvasSize->currentText() == "A3") { printer->setPageSize(QPrinter::A3); }
    else { printer->setPageSize(QPrinter::A4); }
    if(PageLandscape->isChecked()) { printer->setOrientation(QPrinter::Landscape); }
    if(PagePortrait->isChecked()) { printer->setOrientation(QPrinter::Portrait); }

    QPainter painter;
    printer->setResolution(g_confConfig.PaperDpi);
    printer->setPageMargins(0,0,0,0,QPrinter::Millimeter);
    painter.begin(printer);

    pixCenter->setVisible(false);
    text->setFlag(QGraphicsItem::ItemIsSelectable, false);

    bool firstPage = true;

    for( int row = 0; row < modelFilesZO->rowCount(); ++row )
    {
        if (!firstPage) { printer->newPage(); }
        QModelIndex topLevelIndex = modelFilesZO->index( row, 0 );
        zozActivate(topLevelIndex);

        QPainter picPainter;
        QPicture pic;

        float fMargin = cmToPixel(1)*0.1*g_confConfig.PaperMargin;

        // Чтобы QWT отображался как вектор
        picPainter.begin(&pic);
        scene->render(&picPainter, QRectF( fMargin, fMargin, scene->sceneRect().width()-(fMargin*2), scene->sceneRect().height()-(fMargin*2) ),
                      scene->sceneRect(), Qt::KeepAspectRatio);
        picPainter.end();

        painter.drawPicture(QPointF(0, 0), pic);
        text->setHtml(text->toPlainText().arg(row));
        firstPage = false;

        // DEMO режим
        painter.setFont(QFont("Arial", 100));
        painter.drawText(scene->sceneRect().width()/2, scene->sceneRect().height()/2 , "Demo");

    }

    painter.setBackground(QBrush(Qt::white));
    painter.end();

    pixCenter->setVisible(true);
    text->setFlag(QGraphicsItem::ItemIsSelectable, true);
}


/* ----- Быстрая Печать ----- */
void SitPlan::printQuick()
{
    QPrinter printer;
    printBegin(&printer);
}


/* -----  Экспорт в PDF ----- */
void SitPlan::printPDF()
{
    QString sSaveFileName;
    sSaveFileName = QFileDialog::getSaveFileName(this, tr("Экспорт в PDF"), sSaveFileName,
                                                 tr("PDF files (*.pdf)"), NULL, QFileDialog::DontConfirmOverwrite );

    if ( !sSaveFileName.isEmpty() )
    {
        QPrinter printer;
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(sSaveFileName);
        printBegin(&printer);
    }
}


/* -----  Разрешить/Запретить печать ----- */
void SitPlan::printEnabled(const bool b)
{
    ui->action_ExportPDF->setEnabled(b);
    ui->action_Print->setEnabled(b);
    ui->action_PrintQuick->setEnabled(b);
}



/// --------------------------------------------- Настройки --------------------------------------------- ///

/* -------  Загрузка настроек  ------- */
void SitPlan::settingRead()
{
    settings->beginGroup("settingsSitPlan");
    g_confConfig.PaperSize = settings->value("GlobalPaperSize", "A3").toString();
    g_confConfig.PaperOrientation = settings->value("GlobalPaperOrientation", "Landscape").toString();
    g_confConfig.PaperDpi = settings->value("GlobalPaperDpi", 150).toFloat();
    g_confConfig.PaperMargin =  settings->value("GlobalPaperMargin", 5).toInt();
    g_confConfig.VisibleTitle = settings->value("GlobalVisibleTitle", true).toBool();
    g_confConfig.VisibleZo = settings->value("GlobalVisibleZo", true).toBool();
    g_confConfig.VisibleSitPlan = settings->value("GlobalVisibleSitPlan", true).toBool();
    g_confConfig.VisibleGrid = settings->value("GlobalVisibleGrid", true).toBool();
    g_confConfig.SitPlanAuto = settings->value("GlobalSitPlanAuto", true).toBool();
    g_confConfig.TitleFont = settings->value( "TitleFont", QFont("Arial")).value<QFont>();
    g_confConfig.TitleText = settings->value("TitleText", "Ситуационный план (М:[S]). Высота среза [H].").toString();
    g_confConfig.ZoPenWidth = settings->value("ZoPenWidth",1).toInt();
    g_confConfig.ZoColor = settings->value("ZoColor", "#ff0000").value<QColor>();
    g_confConfig.ZoScale = settings->value("ZoScale", "1:1000").toString();
    g_confConfig.ZoSmooth = settings->value("ZoSmooth",true).toBool();
    g_confConfig.GridAxisVisibleLeft = settings->value("GridAxisLeft", true).toBool();
    g_confConfig.GridAxisVisibleRight = settings->value("GridAxisRight", false).toBool();
    g_confConfig.GridAxisVisibleBottom = settings->value("GridAxisBottom", true).toBool();
    g_confConfig.GridAxisVisibleTop = settings->value("GridAxisTop", false).toBool();
    g_confConfig.GridSize = settings->value("GridSize", 0).toInt();
    g_confConfig.GridStep = settings->value("GridStep", 10).toInt();
    g_confConfig.GridPen = settings->value("GridPen", QPen(Qt::black,1,Qt::SolidLine)).value<QPen>();
    g_confConfig.GridAxisFont = settings->value("GridAxisFont", QFont("Arial")).value<QFont>();
    settings->endGroup();
}


/* -------  Применение настроек - ЗО  ------- */
void SitPlan::settingApplyZoz(QVariant qvarZoConfig)
{
    ConfigSitPlan confZoConfig;
    confZoConfig = qvarZoConfig.value<ConfigSitPlan>();

    g_confConfig.ZoPenWidth = confZoConfig.ZoPenWidth;
    g_confConfig.ZoColor = confZoConfig.ZoColor;
    g_confConfig.ZoScale = confZoConfig.ZoScale;
    g_confConfig.ZoSmooth = confZoConfig.ZoSmooth;

    comboBox_SetZOScale->setCurrentText(g_confConfig.ZoScale);

    if(modelFilesZO->rowCount() > 0) {
        zozActivate(ui->tableView_Pages->currentIndex());
    }
}


/* -------  Применение настроек - Сетки  ------- */
void SitPlan::settingApplyGrid(QVariant qvarConfigGrid)
{
    ConfigSitPlan confGridConfig;
    confGridConfig = qvarConfigGrid.value<ConfigSitPlan>();

    g_confConfig.GridAxisVisibleLeft = confGridConfig.GridAxisVisibleLeft;
    g_confConfig.GridAxisVisibleRight = confGridConfig.GridAxisVisibleRight;
    g_confConfig.GridAxisVisibleBottom = confGridConfig.GridAxisVisibleBottom;
    g_confConfig.GridAxisVisibleTop = confGridConfig.GridAxisVisibleTop;
    g_confConfig.GridSize = confGridConfig.GridSize;
    g_confConfig.GridPen = confGridConfig.GridPen;
    g_confConfig.GridStep = confGridConfig.GridStep;

    plotZoz->setGridPen(g_confConfig.GridPen);

    plotZoz->setAxisVisible(g_confConfig.GridAxisVisibleLeft,g_confConfig.GridAxisVisibleRight,
                           g_confConfig.GridAxisVisibleBottom,g_confConfig.GridAxisVisibleTop);

    zozActivate(ui->tableView_Pages->currentIndex());
}


/* -------  Применение настроек - Страницы  ------- */
void SitPlan::settingApplyPage(QVariant qvarConfigPage)
{
    ConfigSitPlan confPageConfig;
    confPageConfig = qvarConfigPage.value<ConfigSitPlan>();

    g_confConfig.PaperSize = confPageConfig.PaperSize;
    g_confConfig.PaperDpi = confPageConfig.PaperDpi;
    g_confConfig.PaperOrientation = confPageConfig.PaperOrientation;
    g_confConfig.PaperMargin = confPageConfig.PaperMargin;

    comboBox_CanvasSize->setCurrentText(g_confConfig.PaperSize);
    pageSize(comboBox_CanvasSize->currentText());

    if(confPageConfig.PaperOrientation == "Landscape")
    {
        PageLandscape->setChecked(true);
        pageLandscape();
    }
    else
    {
        PagePortrait->setChecked(true);
        pagePortrait();
    }
}


/* -------  Применение настроек - Глобальная ------- */
void SitPlan::settingApply()
{
    comboBox_CanvasSize->setCurrentText(g_confConfig.PaperSize);
    comboBox_SetZOScale->setCurrentText(g_confConfig.ZoScale);

    titleVisible(g_confConfig.VisibleTitle);
    if(modelFilesZO->rowCount() > 0)
        zozGridVisible(g_confConfig.VisibleGrid);

    planVisible(g_confConfig.VisibleSitPlan);

    ui->textEdit_Title->setText(g_confConfig.TitleText);

    if(modelFilesZO->rowCount() > 0)
        zozActivate(ui->tableView_Pages->currentIndex());

    plotZoz->setGridPen(g_confConfig.GridPen);

    if (g_confConfig.PaperOrientation == "Landscape")
    {
        PageLandscape->setChecked(false);
        PagePortrait->setChecked(false);
        pageLandscape();
    }
    else
    {
        PageLandscape->setChecked(false);
        PagePortrait->setChecked(false);
        pagePortrait();
    }
    pageSize(comboBox_CanvasSize->currentText());

    plotZoz->setAxisVisible(g_confConfig.GridAxisVisibleLeft,g_confConfig.GridAxisVisibleRight,
                           g_confConfig.GridAxisVisibleBottom,g_confConfig.GridAxisVisibleTop);

    text->setFont(g_confConfig.TitleFont);
    ui->action_SitPlanAuto->setChecked(g_confConfig.SitPlanAuto);
}


/* ------- Сохранение настроек ------- */
void SitPlan::settingWrite()
{
    settings->beginGroup("settingsSitPlan");

    settings->setValue("GlobalPaperSize", comboBox_CanvasSize->currentText());
    if(PageLandscape->isChecked())
        settings->setValue("GlobalPaperOrientation", "Landscape");
    else
        settings->setValue("GlobalPaperOrientation", "Portrait");

    settings->setValue("GlobalVisibleTitle",ui->action_VisibleTitle->isChecked());
    settings->setValue("GlobalVisibleZo",ui->action_VisibleZo->isChecked());
    settings->setValue("GlobalVisibleSitPlan",ui->action_VisbleSitPlan->isChecked());
    settings->setValue("GlobalVisibleGrid",ui->action_VisibleGrid->isChecked());
    settings->setValue("GlobalSitPlanAuto", ui->action_SitPlanAuto->isChecked());
    settings->setValue("GlobalPaperDpi", g_confConfig.PaperDpi);
    settings->setValue("GlobalPaperMargin", g_confConfig.PaperMargin);

    settings->setValue("ZoPenWidth", g_confConfig.ZoPenWidth);
    settings->setValue("ZoColor", g_confConfig.ZoColor.name());
    settings->setValue("ZoScale", g_confConfig.ZoScale);
    settings->setValue("ZoSmooth", g_confConfig.ZoSmooth);

    settings->setValue("TitleFont", text->font());
    settings->setValue("TitleText", ui->textEdit_Title->toPlainText());

    settings->setValue("GridAxisLeft", g_confConfig.GridAxisVisibleLeft );
    settings->setValue("GridAxisRight", g_confConfig.GridAxisVisibleRight );
    settings->setValue("GridAxisBottom", g_confConfig.GridAxisVisibleBottom );
    settings->setValue("GridAxisTop", g_confConfig.GridAxisVisibleTop );
    settings->setValue("GridStep", g_confConfig.GridStep );

    settings->setValue("GridAxisFont", g_confConfig.GridAxisFont);
    settings->setValue("GridPen", g_confConfig.GridPen);
    settings->setValue("SitPlanAuto", ui->action_SitPlanAuto->isChecked());

    settings->endGroup();
}



/// --------------------------------------------- Диалоги --------------------------------------------- ///
/* -------  Показать диалог настроек ЗО  ------- */
void SitPlan::showConfigZo()
{
    dlg_ConfigZo->loadSettings(g_confConfig);
    dlg_ConfigZo->show();
}


/* -------  Показать диалог настроек страницы  ------- */
void SitPlan::showConfigPage()
{
    g_confConfig.PaperSize = comboBox_CanvasSize->currentText();
    if(PageLandscape->isChecked())
        g_confConfig.PaperOrientation = "Landscape";
    else
        g_confConfig.PaperOrientation = "Portrait";

    dlg_ConfigDocument->loadSettings(g_confConfig);
    dlg_ConfigDocument->show();
}


/* -------  Показать диалог настроек сетки  ------- */
void SitPlan::showConfigGrid()
{
    dlg_ConfigGrid->loadSettings(g_confConfig);
    dlg_ConfigGrid->show();
}


/* ------- Показать диалог настроек шрифта сетки  ------- */
void SitPlan::showConfigGridFont()
{
    bool bOk;
    g_confConfig.GridAxisFont = QFontDialog::getFont(&bOk,g_confConfig.GridAxisFont,0,"Шрифт сетки");

    if(bOk)
        plotZoz->setAxisFontForAll(g_confConfig.GridAxisFont);

    plotZoz->replot();
    zozActivate(ui->tableView_Pages->currentIndex());
}



/// --------------------------------------------- Меню -> Вид --------------------------------------------- ///
/* -------  Режим выбора - Выбор  ------- */
void SitPlan::viewModeSelect()
{
    if(ui->action_ModeSelect->isChecked() ) {
        ui->action_ModeDrag->setChecked(false);
        ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
        ui->graphicsView->setInteractive(true);
    }
    ui->action_ModeSelect->setChecked(true);
}


/* -------  Режим выбора - Рука  ------- */
void SitPlan::viewModeDrag()
{
    if(ui->action_ModeDrag->isChecked())
    {
        ui->action_ModeSelect->setChecked(false);
        ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
        ui->graphicsView->setInteractive(false);
    }
    ui->action_ModeDrag->setChecked(true);
}


/* -------  Оригинальный вид  ------- */
void SitPlan::viewZoomOriginal()
{
    zoomSlider->setValue(100);
}


/* -------  Увеличить ------- */
void SitPlan::viewZoomIn()
{
    zoomSlider->setValue(zoomSlider->value()+10);
}


/* ------- Уменьшить ------- */
void SitPlan::viewZoomOut()
{
    zoomSlider->setValue(zoomSlider->value()-10);
}


/* ------- Вид по размеру экрана ------- */
void SitPlan::viewZoomBest()
{
    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}


/* ------- Слайдер ------- */
void SitPlan::viewZoomCustom()
{
    qreal scale = qPow(qreal(2), (zoomSlider->value() - 100) / qreal(25));

    const float xScale = ( ((physicalDpiX()+logicalDpiX())/2)/300.0 );
    const float yScale = ( ((physicalDpiY()+logicalDpiY())/2)/300.0 );

    QMatrix matrix;
    matrix.scale(xScale*scale, yScale*scale);

    ui->graphicsView->setMatrix(matrix);

    if(zoomSlider->value() == 100)
        ui->action_ViewOriginal->setChecked(true);
    else
        ui->action_ViewOriginal->setChecked(false);
}



/// --------------------------------------------- Вспомогательные --------------------------------------------- ///
/* -------  Пиксели в сантиметры  ------- */
float SitPlan::cmToPixel(const float x)
{
    return ( (logicalDpiX()/2.54)*x );
}
float SitPlan::pixelToM(const float x)
{
    return ( x/((logicalDpiX())/2.54) )*10;
}
