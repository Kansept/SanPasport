#include "DnView.h"
#include "ui_DnView.h"
#include "tools/dialogconvert.h"
#include "tools/dialogpreferens.h"
#include "tools/window_projection_pkaemo4.h"
#include "tools/dialog_groupchange.h"
#include "qdata.h"

#include "qwt_polar_renderer.h"
#include "qwt_plot_renderer.h"
#include "qwt_scale_engine.h"
#include "qwt_plot_picker.h"
#include "qwt_scale_widget.h"

#include <QBuffer>
#include <QPen>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <qsettings.h>
#include <qimagewriter.h>
#include <math.h>
#include <QClipboard>
#include <QTextCodec>
#include <QProcess>
#include <QPrinter>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QTextDocument>
#include <QUrl>
#include <QColor>
#include <QColorDialog>
#include <QMimeData>
#include <QSvgGenerator>
#include <QAxBase>
#include <QAxObject>


using namespace std;

// ------- Глобальные переменные ------- >>
const int N1 = 361;       // Размер массива данных ДН
Prto g_dan;         // Переменная данных графика
// ------- Глобальные переменные ------- <<

// ------- Переменные в файле конфигуорации ------- >>
int g_GraphScale;               // Масштаб графика
QColor qcHorizDN;               // Цвет Горизонтальной ДН
QColor qcVertDN;                // Цвет Вертикальной ДН
QString g_qsOpenFile;           // Имя открытого файла
bool conf_bViewPanelData;       // Отображение Панели с данными
bool conf_bViewPanelTransform;  // Отображение Панели трансформации
bool conf_bViewToolBar;         // Отображение панели инструментов
QString conf_qsRecentFiles[10]; // Недавние файлы
// ------- Переменные в файле конфигуорации ------- <<


DnView::DnView(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Показывать заголовки TableWidget
    ui->tableWidget_Data1->horizontalHeader()->setVisible(true);
    ui->tableWidget_Data2->horizontalHeader()->setVisible(true);

    // Загрузка и применение файла конфигурации
    settings = new QSettings((QCoreApplication::applicationDirPath()) + "//configDnView.ini",QSettings::IniFormat);
    loadSettings();

    ui->action_HideHorizDN->setChecked(true);
    ui->action_HideVertDN->setChecked(true);

    // Создаем сетку полярного графика
    gridPolar = new QwtPolarGrid();
    for (int sc=0; sc < QwtPolar::ScaleCount; sc++) {
        gridPolar->showMinorGrid(sc);
        gridPolar->setMajorGridPen(sc,QPen(Qt::lightGray));                  // основных делений
        gridPolar->setMinorGridPen(sc,QPen(Qt::lightGray,0,Qt::DotLine));    // и вспомогательных
    }
    gridPolar->showAxis(QwtPolar::AxisRight,false);       // запрещаем отображать радиальную шкалу справа
    gridPolar->showAxis(QwtPolar::AxisTop,true);          // разрешаем отображать радиальную шкалу сверху
    gridPolar->attach(ui->PlotPolar);

    // Создаем сетку декартового графика
    gridDec = new QwtPlotGrid;      // создаем сетку
    gridDec->enableXMin(true);       // разрешаем отображение линий сетки, соответствующих вспомогательным делениям нижней шкалы
    gridDec->enableYMin(true);
    gridDec->setMajorPen(QPen(Qt::lightGray,1,Qt::SolidLine)); // черный для основных делений
    gridDec->setMinorPen(QPen(Qt::lightGray,1,Qt::DashLine));  // серый для вспомогательных
    gridDec->attach(ui->PlotDec);                       // связываем созданную сетку с графиком
    QwtPlotMarker *d_markerH = new QwtPlotMarker;
    d_markerH->setValue( 0, 0.0 );
    d_markerH->setLineStyle( QwtPlotMarker::VLine );
    d_markerH->setLinePen( QPen( Qt::black, 1.5, Qt::SolidLine ) );
    d_markerH->attach( ui->PlotDec );

    QwtPlotMarker *d_markerV = new QwtPlotMarker;
    d_markerV->setValue( 0, 0 );
    d_markerV->setLineStyle( QwtPlotMarker::HLine );
    d_markerV->setLinePen( QPen( Qt::black, 1.5, Qt::SolidLine ) );
    d_markerV->attach( ui->PlotDec );

    // Скрываем Шкалы в декартовом графике
    QwtScaleWidget *qwtswBottom = ui->PlotDec->axisWidget(QwtPlot::xBottom);
    QPalette paletteBottom = qwtswBottom->palette();
    paletteBottom.setColor( QPalette::WindowText, Qt::transparent); // for ticks
    qwtswBottom->setPalette( paletteBottom );

    QwtScaleWidget *qwtswlLeft = ui->PlotDec->axisWidget(QwtPlot::yLeft);
    QPalette palettelLeft = qwtswlLeft->palette();
    palettelLeft.setColor( QPalette::WindowText, Qt::transparent); // for ticks
    qwtswlLeft->setPalette( palettelLeft );

    // Шрифт графиков
    QFont fontAxis;
    fontAxis.setWeight(10);
    fontAxis.setFamily("Tahoma");
    ui->PlotDec->setAxisFont(0, fontAxis);
    ui->PlotDec->setAxisFont(1, fontAxis);
    ui->PlotDec->setAxisFont(2, fontAxis);
    ui->PlotDec->setAxisFont(3, fontAxis);
    ui->PlotPolar->setFont(fontAxis);

    // Объявление переменных диалогов
    dlgPreferens = new DialogPreferens(this);                     // Диалог "Настроек"
    dlgConvert = new DialogConvert(this);                         // Диалог "Конвертора"
    win_projection_PKAEMO4 = new Window_projection_PKAEMO4(this); // Диалог "Проекция (ПК АЭМО 4)"
    dlgGroupChange = new Dialog_GroupChange(this);                // Пакетная обработка

    // Слоты и сигналы
    // Файл
    connect( ui->action_Open, SIGNAL(triggered()), this, SLOT(action_Open()) );       // Файл -> Открыть
    connect( ui->action_Save, SIGNAL(triggered()), this, SLOT(action_Save()) );       // Файл -> Сохранить
    connect( ui->action_SaveAs, SIGNAL(triggered()), this, SLOT(action_SaveAs()) );   // Файл -> Сохранить как
    connect( ui->action_Export, SIGNAL(triggered()), this, SLOT(action_Export()) );   // Файл -> Экспорт
    connect( ui->action_Preferens, SIGNAL(triggered()), dlgPreferens, SLOT(show()) ); // Файл -> Настроек
    connect( dlgPreferens, SIGNAL(sendPrefOk()), this, SLOT(initGraph()) );
    connect( ui->action_Close, SIGNAL(triggered()), this, SLOT(closeDN()) );          // Файл -> Закрыть
    connect( ui->action_Exit, SIGNAL(triggered()), this, SLOT(close()) );             // Файл -> Выход
    // Вид
    connect( ui->action_Scale25dB, SIGNAL(triggered(bool)), this, SLOT(graphScale25db(bool)) );                 // Изменение масштаба графика 25dB
    connect( ui->action_Scale40dB, SIGNAL(triggered(bool)), this, SLOT(graphScale40db(bool)) );                 // Изменение масштаба графика 40dB
    connect( ui->action_ScaleNorm, SIGNAL(triggered(bool)), this, SLOT(graphScaleNorm(bool)) );                 // Нормализованная ДН
    connect( ui->action_HideHorizDN, SIGNAL(triggered(bool)), this, SLOT(on_actionViewHorizDn(bool)) );         // Показать/Скрыть Вертикальную ДН
    connect( ui->action_HideVertDN, SIGNAL(triggered(bool)), this, SLOT(on_actionViewVertDn(bool)) );           // Показать/Скрыть Горизонтальную ДН
    connect( ui->action_PanelData, SIGNAL(toggled(bool)), this, SLOT(on_actionViewPanelData(bool)) );           // Показать/Скрыть панель с данными
    connect( ui->action_PanelTransform, SIGNAL(toggled(bool)), this, SLOT(on_actionViewPanelTransform(bool)) ); // Показать/Скрыть панель трансформации
    // Редактирование ДН
    connect( ui->checkBox_PatternHoriz, SIGNAL(clicked()), this, SLOT(isEditPattern()) );
    connect( ui->checkBox_PatternVert, SIGNAL(clicked()), this, SLOT(isEditPattern()) );
    connect( ui->action_isEditPatternHoriz, SIGNAL(triggered()), this, SLOT(isEditPatternInMenu()) );
    connect( ui->action_isEditPatternVert, SIGNAL(triggered()), this, SLOT(isEditPatternInMenu()) );
    // Поворот
    connect( ui->toolButton_RotateCw, SIGNAL(clicked()), this, SLOT(setPatternRotateCw()) );
    connect( ui->toolButton_RotateCcw, SIGNAL(clicked()), this, SLOT(setPatternRotateCcw()) );
    connect( ui->action_EditPatternRotateCw, SIGNAL(triggered()), SLOT(setPatternRotateCw()) );
    connect( ui->action_EditPatternRotateCcw, SIGNAL(triggered()), SLOT(setPatternRotateCcw()) );
    // Отзеркаливание
    connect( ui->toolButton_Mirror, SIGNAL(clicked()), this, SLOT(action_PatternMirror()) );       // Отразить Горизонтальную ДН
    connect( ui->action_EditPatternMirror, SIGNAL(triggered()), this, SLOT(action_PatternMirror()) );  // Отразить Горизонтальную ДН
    // Симетрия
    connect( ui->action_EditPattermSymmetryTop, SIGNAL(triggered()), this, SLOT(setPatternSymmetryTop()) );
    connect( ui->toolButton_EditPattermSymmetryTop, SIGNAL(clicked()), this, SLOT(setPatternSymmetryTop()) );
    connect( ui->action_EditPattermSymmetryBottom, SIGNAL(triggered()), this, SLOT(setPatternSymmetryBottom()) );
    connect( ui->toolButton_EditPattermSymmetryBottom, SIGNAL(clicked()), this, SLOT(setPatternSymmetryBottom()) );
    // Поджать боковые лепестки
    connect( ui->toolButton_Preload, SIGNAL(clicked()), this, SLOT(setPreload()) );
    connect( ui->action_EditPreload, SIGNAL(triggered()), this, SLOT(setPreload()) );
    // Утилиты
    connect( ui->action_OpenInTextEdit, SIGNAL(triggered()), this, SLOT(openInTextEdit()) );                  // Утилиты -> Открыть в текстовом редакторе
    connect( ui->action_ConvertUnit, SIGNAL(triggered()), dlgConvert, SLOT(show()) );                         // Утилиты -> Конвертер величин
    connect( ui->action_projection_PKAEMO4, SIGNAL(triggered()), win_projection_PKAEMO4, SLOT(show()) );      // Утилиты -> Проекция (ПК АЭМО 4)
    connect( ui->action_projection_PKAEMO4, SIGNAL(triggered()), win_projection_PKAEMO4, SLOT(clearData()) ); // Утилиты -> Проекция (ПК АЭМО 4) - Очистить данные
    connect( ui->action_GroupChange, SIGNAL(triggered()), dlgGroupChange, SLOT(show()) );
    connect( ui->action_DataSheet, SIGNAL(triggered()), this, SLOT(setDataSheet()) );
    // О программе
    connect( ui->action_About, SIGNAL(triggered()), this, SLOT(action_AboutThis()) ); // Справка -> О Программе
    connect( ui->action_AboutQT, SIGNAL(triggered()), qApp, SLOT(aboutQt()) );        // Справка -> О QT
    // Контекстное меню
    connect( ui->PlotPolar, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuPolar_ContextMenu(QPoint)));
    connect( ui->PlotDec, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuPolar_ContextMenu(QPoint)));
    connect( ui->action_buff, SIGNAL(triggered()), this, SLOT(CopyToBuf()) );
    connect( ui->action_copyToBuf, SIGNAL(triggered()), this, SLOT(CopyToBuf()) );

    // Строка состояния
    sb1 = new QLabel(statusBar());
    sb2 = new QLabel(statusBar());
    sb3 = new QLabel(statusBar());
    sb4 = new QLabel(statusBar());
    ui->statusBar->setSizeGripEnabled(false);
    ui->statusBar->addWidget(sb1, 7);
    ui->statusBar->addWidget(sb2, 1);
    ui->statusBar->addWidget(sb3, 1);
    ui->statusBar->addWidget(sb4, 1);
    sb1->setText(" ");
    sb2->setText("ET: ");
    sb3->setText("Beamwidth H: ");
    sb4->setText("Beamwidth V: ");

    // Реализация Drag'n'Drop
    setAcceptDrops(true);

    // Разрешаем Контекстное меню
    ui->PlotPolar->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->PlotDec->setContextMenuPolicy(Qt::CustomContextMenu);

    // Кнопка Масштаб в toolBar
    toolButton_MenuScale = new QToolButton;
    toolButton_MenuScale->setText(QString::number(g_GraphScale)+" dB ");
    QMenu *MenuScale = new QMenu(toolButton_MenuScale);
    MenuScale->addAction(ui->action_Scale25dB);
    MenuScale->addAction(ui->action_Scale40dB);
    //       MenuScale->addAction(ui->action_ScaleNorm);
    toolButton_MenuScale->setPopupMode(QToolButton::InstantPopup);
    toolButton_MenuScale->setMenu(MenuScale);
    ui->toolBar_View->addWidget(toolButton_MenuScale);

    // Устанавливаем UI по умолчанию
    EnableUI(false);

    // Удаляем таб с паспортом
    ui->tabWidget->removeTab(2);

    for(int i=0; i<ui->tableWidget_Data1->rowCount(); i++)
        ui->tableWidget_Data1->item(i,0)->setBackground(QColor(245,245,245));
}


// --------------------------------------------------- ~MainWindow --------------------------------------------------- //
DnView::~DnView()
{
    saveSettings();
    if (!(CurvPolarHoriz)) {
        // Полярный график
        delete CurvPolarHoriz;
        delete CurvPolarVert;
        delete CurvPolar3db;
        delete gridPolar;
        delete d_marker1;
        // Декартовый график
        delete CurvDecHoriz;
        delete CurvDecVert;
        delete gridDec;
        delete ui;
    } else {
        // Координатную сетку
        delete gridPolar;
        delete ui;
    }
}


// --------------------------------------------------- Инициализация графиков --------------------------------------------------- //
void DnView::initGraph()
{
    settings->beginGroup("settings_DnView");
    qcHorizDN = settings->value("ColorHorizDN", "#00ff00").value<QColor>();
    qcVertDN = settings->value("ColorVertDN", "#ff0000").value<QColor>();
    settings->endGroup();

    QString styleLeg = "color: rgb(%1, %2, %3);";
    ui->labelPolar_ColorHorizDN_Box->setStyleSheet( styleLeg.arg(qcHorizDN.red()).arg(qcHorizDN.green()).arg( qcHorizDN.blue()) );
    ui->labelPolar_ColorVertzDN_Box->setStyleSheet( styleLeg.arg(qcVertDN.red()).arg(qcVertDN.green()).arg( qcVertDN.blue()) );
    ui->labelDec_ColorHorizDN_Box->setStyleSheet( styleLeg.arg(qcHorizDN.red()).arg(qcHorizDN.green()).arg( qcHorizDN.blue()) );
    ui->labelDec_ColorVertDN_Box->setStyleSheet( styleLeg.arg(qcVertDN.red()).arg(qcVertDN.green()).arg( qcVertDN.blue()) );

    if (g_dan.Name != "") {
        initDecartGraph(g_dan);
        initPolarGraph(g_dan);
    }
}


// --------------------------------------------------- Инициализация графиков --------------------------------------------------- //
void DnView::initAllGraph()
{
    if (g_dan.Name != "") {
        setDanGraph(g_dan);
        initDecartGraph(g_dan);
        initPolarGraph(g_dan);
    }
}

// --------------------------------------------------- Загрузка файла настроек --------------------------------------------------- //
void DnView::loadSettings()
{
    settings->beginGroup("settings_DnView");
    g_GraphScale = settings->value("g_GraphScale",25).toInt();
    qcHorizDN = settings->value("ColorHorizDN", "#00ff00").value<QColor>();
    qcVertDN = settings->value("ColorVertDN", "#ff0000").value<QColor>();
    conf_bViewPanelData =  settings->value("ViewPanelData",true).toBool();
    conf_bViewPanelTransform =  settings->value("ViewPanelTransform",true).toBool();
    conf_bViewToolBar =  settings->value("ViewToolBar",1).toBool();

    conf_qsRecentFiles[0] = settings->value("RecentFiles0", "-").toString();
    conf_qsRecentFiles[1] = settings->value("RecentFiles1", "-").toString();
    conf_qsRecentFiles[2] = settings->value("RecentFiles2", "-").toString();
    conf_qsRecentFiles[3] = settings->value("RecentFiles3", "-").toString();
    conf_qsRecentFiles[4] = settings->value("RecentFiles4", "-").toString();
    conf_qsRecentFiles[5] = settings->value("RecentFiles5", "-").toString();
    conf_qsRecentFiles[6] = settings->value("RecentFiles6", "-").toString();
    conf_qsRecentFiles[7] = settings->value("RecentFiles7", "-").toString();
    conf_qsRecentFiles[8] = settings->value("RecentFiles8", "-").toString();
    conf_qsRecentFiles[9] = settings->value("RecentFiles9", "-").toString();
    settings->endGroup();

    QString styleLeg = "color: rgb(%1, %2, %3);";
    ui->labelPolar_ColorHorizDN_Box->setStyleSheet( styleLeg.arg(qcHorizDN.red()).arg(qcHorizDN.green()).arg( qcHorizDN.blue()) );
    ui->labelPolar_ColorVertzDN_Box->setStyleSheet( styleLeg.arg(qcVertDN.red()).arg(qcVertDN.green()).arg( qcVertDN.blue()) );
    ui->labelDec_ColorHorizDN_Box->setStyleSheet( styleLeg.arg(qcHorizDN.red()).arg(qcHorizDN.green()).arg( qcHorizDN.blue()) );
    ui->labelDec_ColorVertDN_Box->setStyleSheet( styleLeg.arg(qcVertDN.red()).arg(qcVertDN.green()).arg( qcVertDN.blue()) );

    if (g_GraphScale == 25) {ui->action_Scale25dB->setChecked(true); ui->action_Scale40dB->setChecked(false);}
    if (g_GraphScale == 40) {ui->action_Scale40dB->setChecked(true); ui->action_Scale25dB->setChecked(false);}

    on_actionViewPanelData(conf_bViewPanelData);
    on_actionViewPanelTransform(conf_bViewPanelTransform);

    ui->action_PanelData->setChecked(conf_bViewPanelData);
    ui->action_PanelTransform->setChecked(conf_bViewPanelTransform);
    ui->actionView_ToolBar->setChecked(conf_bViewToolBar);

    ui->action_RecentFiles0->setText(conf_qsRecentFiles[0]);
    ui->action_RecentFiles1->setText(conf_qsRecentFiles[1]);
    ui->action_RecentFiles2->setText(conf_qsRecentFiles[2]);
    ui->action_RecentFiles3->setText(conf_qsRecentFiles[3]);
    ui->action_RecentFiles4->setText(conf_qsRecentFiles[4]);
    ui->action_RecentFiles5->setText(conf_qsRecentFiles[5]);
    ui->action_RecentFiles6->setText(conf_qsRecentFiles[6]);
    ui->action_RecentFiles7->setText(conf_qsRecentFiles[7]);
    ui->action_RecentFiles8->setText(conf_qsRecentFiles[8]);
    ui->action_RecentFiles9->setText(conf_qsRecentFiles[9]);

    if (g_dan.Name != "") {
        initDecartGraph(g_dan);
        initPolarGraph(g_dan);
    }
}


//--------------------------------------------------- Загрузка файла настроек --------------------------------------------------- //
void DnView::loadSettingsRecentFiles()
{
    settings->beginGroup("settings_DnView");
    conf_qsRecentFiles[0] = settings->value("RecentFiles0", "-").toString();
    conf_qsRecentFiles[1] = settings->value("RecentFiles1", "-").toString();
    conf_qsRecentFiles[2] = settings->value("RecentFiles2", "-").toString();
    conf_qsRecentFiles[3] = settings->value("RecentFiles3", "-").toString();
    conf_qsRecentFiles[4] = settings->value("RecentFiles4", "-").toString();
    conf_qsRecentFiles[5] = settings->value("RecentFiles5", "-").toString();
    conf_qsRecentFiles[6] = settings->value("RecentFiles6", "-").toString();
    conf_qsRecentFiles[7] = settings->value("RecentFiles7", "-").toString();
    conf_qsRecentFiles[8] = settings->value("RecentFiles8", "-").toString();
    conf_qsRecentFiles[9] = settings->value("RecentFiles9", "-").toString();
    settings->endGroup();

    ui->action_RecentFiles0->setText(conf_qsRecentFiles[0]);
    ui->action_RecentFiles1->setText(conf_qsRecentFiles[1]);
    ui->action_RecentFiles2->setText(conf_qsRecentFiles[2]);
    ui->action_RecentFiles3->setText(conf_qsRecentFiles[3]);
    ui->action_RecentFiles4->setText(conf_qsRecentFiles[4]);
    ui->action_RecentFiles5->setText(conf_qsRecentFiles[5]);
    ui->action_RecentFiles6->setText(conf_qsRecentFiles[6]);
    ui->action_RecentFiles7->setText(conf_qsRecentFiles[7]);
    ui->action_RecentFiles8->setText(conf_qsRecentFiles[8]);
    ui->action_RecentFiles9->setText(conf_qsRecentFiles[9]);
}


// --------------------------------------------------- Сохранение файла настроек ------------------------------------------------ //
void DnView::saveSettings()
{
    settings->beginGroup("settings_DnView");
    settings->setValue("g_GraphScale", g_GraphScale);
    settings->setValue( "ViewPanelData", conf_bViewPanelData );
    settings->setValue( "ViewPanelTransform", conf_bViewPanelTransform );
    settings->setValue( "ViewToolBar", conf_bViewToolBar);
    settings->endGroup();
    settings->sync();
}


// --------------------------------------------------- Реализация Drag'n'Drop --------------------------------------------------- //
void DnView::dragEnterEvent(QDragEnterEvent* event){
    if(event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}


// --------------------------------------------------- Реализация Drag'n'Drop --------------------------------------------------- //
void DnView:: dropEvent(QDropEvent* event){
    QList<QUrl> urls =event->mimeData()->urls();
    if(urls.isEmpty())
        return;
    QString fileName = urls.first().toLocalFile();

    if(fileName.isEmpty())
        return;
    else
        OpenFile(fileName);
}


// --------------------------------------------------- Инициализация полярного графика ------------------------------------------ //
void DnView::initPolarGraph(Prto g_dan)
{
    g_dan.AzHoriz[360]=360;
    g_dan.AzVert[360]=360;
    g_dan.RadHoriz[360]=g_dan.RadHoriz[0];
    g_dan.RadVert[360]=g_dan.RadVert[0];

    double marker3db[361];
    for (int i=0; i<=361; i++) {
        marker3db[i]=-3;
    }

    ui->PlotPolar->detachItems(0,1);
    gridPolar = new QwtPolarGrid();                            // создаем сетку

    for (int sc=0; sc < QwtPolar::ScaleCount; sc++)
    {
        gridPolar->showMinorGrid(sc);
        gridPolar->setMajorGridPen(sc,QPen(Qt::lightGray));                  // основных делений
        gridPolar->setMinorGridPen(sc,QPen(Qt::lightGray,0,Qt::DotLine));    // и вспомогательных
    }

    // markers
    QwtPolarMarker *marker = new QwtPolarMarker();
    if(g_GraphScale != 1) {
        marker->setPosition( QwtPointPolar( 269.5, -3 ) );
    } else {
        marker->setPosition( QwtPointPolar( 269.5, 0.747 ) );
    }

    marker->setSymbol( new QwtSymbol( QwtSymbol::HLine, QBrush( Qt::transparent ), QPen( Qt::green ), QSize( 0, 0 ) ) );
    marker->setLabelAlignment( Qt::AlignLeft | Qt::AlignTop );
    QwtText text( "-3" );
    text.setColor( Qt::black );
    QColor bg( Qt::white );
    bg.setAlpha( 200 );
    text.setBackgroundBrush( QBrush( bg ) );
    marker->setLabel( text );
    marker->attach( ui->PlotPolar );

    gridPolar->showAxis(QwtPolar::AxisRight,false);       // запрещаем отображать радиальную шкалу справа
    gridPolar->showAxis(QwtPolar::AxisTop,true);          // разрешаем отображать радиальную шкалу сверху

    gridPolar->attach(ui->PlotPolar);                        // связываем созданную сетку с графиком

    if(g_GraphScale != 1) {
        ui->PlotPolar->setScale(QwtPolar::Radius,-g_GraphScale, 0);    // радиальной шкалы
    } else {
        ui->PlotPolar->setScale(QwtPolar::Radius,0, g_GraphScale);    // радиальной шкалы
    }

    ui->PlotPolar->setScale(QwtPolar::Azimuth,359.9,-0.1,30);      // и азимутальной

    CurvPolarVert = new QwtPolarCurve(QString("Vertical"));        // создаем первую кривую с наименованием "DNA 1"
    CurvPolarVert->setPen(QPen(qcVertDN,2));                       // назначаем толщину 2 и цвет прорисовки - темно-зеленый

    CurvPolarHoriz = new QwtPolarCurve(QString("Horizontal"));     // создаем вторую кривую с наименованием "DNA 2"
    CurvPolarHoriz->setPen(QPen(qcHorizDN,2));                     // назначаем цвет прорисовки - красный

    CurvPolar3db = new QwtPolarCurve(QString("Marker3db"));        // создаем вторую кривую с наименованием "DNA 2"
    CurvPolar3db->setPen(QPen(Qt::black,1));                       // назначаем цвет прорисовки - красный
    CurvPolar3db->setPen(QPen(Qt::DashDotLine));
    CurvPolar3db->setStyle(QwtPolarCurve::Lines);
    CurvPolar3db->setData(new QData(g_dan.AzVert,marker3db,N1));   // передаем кривым подготовленные данные
    CurvPolar3db->attach(ui->PlotPolar);

    if(g_GraphScale != 1)
    {
        CurvPolarVert->setData(new QData(g_dan.AzVert,g_dan.RadVert,N1));     // передаем кривым подготовленные данные
        CurvPolarHoriz->setData(new QData(g_dan.AzHoriz,g_dan.RadHoriz,N1));  // передаем кривым подготовленные данные
    }
    else
    {
        double fRadHoriz[361], fRadVert[361];
        float fRadHorizMin, fRadVertMin;
        fRadHorizMin = g_dan.radHorizMin();
        fRadVertMin = g_dan.radVertMin();
        for(int i=0; i<361; i++)
        {
            fRadHoriz[i] = pow((float(fabs(fRadHorizMin)) - float(fabs(g_dan.RadHoriz[i]))) / -fRadHorizMin,2);
            fRadVert[i]  = pow((float(fabs(fRadVertMin)) - float(fabs(g_dan.RadVert[i]))) / -fRadVertMin,2);
        }
        CurvPolarVert->setData(new QData(g_dan.AzVert,fRadVert,N1));     // передаем кривым подготовленные данные
        CurvPolarHoriz->setData(new QData(g_dan.AzHoriz,fRadHoriz,N1));  // передаем кривым подготовленные данные
    }
    CurvPolarHoriz->attach(ui->PlotPolar);
    CurvPolarVert->attach(ui->PlotPolar);

    if( !(ui->action_HideHorizDN->isChecked()) ) { CurvPolarHoriz->hide(); }
    if( !(ui->action_HideVertDN->isChecked()) ) { CurvPolarVert->hide(); }

    ui->PlotPolar->replot();

    sb2->setText("ET: "+(QString::number(g_dan.etilt())) );
    sb3->setText("Beamwidth H: "+(QString::number(g_dan.beamwidthHoriz())) );
    sb4->setText("Beamwidth V: "+(QString::number(g_dan.beamwidthVert())) );
}


// --------------------------------------------------- Инициализация графика в декартовых координатах -------------------------- //
void DnView::initDecartGraph(Prto g_dan)
{
    ui->PlotDec->detachItems(0,1);
    gridDec = new QwtPlotGrid;       // создаем сетку
    gridDec->enableXMin(true);       // разрешаем отображение линий сетки, соответствующих вспомогательным делениям нижней шкалы
    gridDec->enableYMin(true);
    gridDec->setMajorPen(QPen(Qt::lightGray,1,Qt::SolidLine));    // черный для основных делений
    gridDec->setMinorPen(QPen(Qt::lightGray,1,Qt::DashLine));     // серый для вспомогательных
    gridDec->attach(ui->PlotDec);                                 // связываем созданную сетку с графиком

    // Маркер на 3 dB
    d_marker1 = new QwtPlotMarker();
    QString label;
    label.append("-3");
    QwtText text( label );
    text.setFont( QFont( "Helvetica", 10, QFont::Bold ) );
    // text.setColor( QColor( 200, 150, 0 ) );
    d_marker1->setLabel( text );
    d_marker1->setValue( 0.0, -3.0 );
    d_marker1->setLineStyle( QwtPlotMarker::HLine );
    d_marker1->setLabelAlignment( Qt::AlignLeft | Qt::AlignBottom );
    d_marker1->setLinePen( QPen( Qt::black, 1, Qt::DashDotLine ) );
    d_marker1->setItemAttribute( QwtPlotItem::Legend, true );
    d_marker1->attach( ui->PlotDec );

    QwtPlotMarker *d_markerH = new QwtPlotMarker;
    d_markerH->setValue( -180.0, 0.0 );
    d_markerH->setLineStyle( QwtPlotMarker::VLine );
    d_markerH->setLinePen( QPen( Qt::black, 1.5, Qt::SolidLine ) );
    d_markerH->attach( ui->PlotDec );

    QwtPlotMarker *d_markerV = new QwtPlotMarker;
    d_markerV->setValue( 0.0, -g_GraphScale );
    d_markerV->setLineStyle( QwtPlotMarker::HLine );
    d_markerV->setLinePen( QPen( Qt::black, 1.5, Qt::SolidLine ) );
    d_markerV->attach( ui->PlotDec );

    // устанавливаем
    QwtText qwttextAxisBottom("градусы");
    qwttextAxisBottom.setFont(QFont("Tahoma",11,1));
    ui->PlotDec->setAxisTitle(QwtPlot::xBottom,qwttextAxisBottom); // наименование нижней шкалы
    ui->PlotDec->setAxisScale(QwtPlot::xBottom,-180,180);      // минимальную и максимальную границы для нее

    QwtText qwttextAxisLeft("dB");
    qwttextAxisLeft.setFont(QFont("Tahoma",11,1));
    ui->PlotDec->setAxisTitle(QwtPlot::yLeft,qwttextAxisLeft);    // наименование левой шкалы

    if(g_GraphScale != 1) {
        ui->PlotDec->setAxisScale(QwtPlot::yLeft,-g_GraphScale,0);                   // минимальную и максимальную границы для нее
    } else {
        ui->PlotDec->setAxisScale(QwtPlot::yLeft,0,g_GraphScale);
    }

    CurvDecHoriz = new QwtPlotCurve(QString("Horizontal"));         // создаем первую кривую с наименованием "U1(t)"
    CurvDecHoriz->setRenderHint(QwtPlotItem::RenderAntialiased);    // разрешаем для нее сглаживание при прорисовке
    CurvDecHoriz->setPen(QPen(qcHorizDN,2));                        // назначаем цвет прорисовки - красный

    CurvDecVert = new QwtPlotCurve(QString("Vertical"));           // создаем вторую кривую с наименованием "U2(t)"
    CurvDecVert->setRenderHint(QwtPlotItem::RenderAntialiased);    // разрешаем для нее сглаживание при прорисовке
    CurvDecVert->setPen(QPen(qcVertDN,2));                         // назначаем цвет прорисовки - красный

    double fRadHoriz[361], fRadVert[361];
    if(g_GraphScale != 1) {
        for(int i=0; i<361; i++) {
            fRadHoriz[i] = g_dan.RadHoriz[i];
            fRadVert[i] = g_dan.RadVert[i];
        }
    } else {
        float fRadHorizMin, fRadVertMin;
        fRadHorizMin = g_dan.radHorizMin();
        fRadVertMin = g_dan.radVertMin();
        for(int i=0; i<361; i++) {
            fRadHoriz[i] = float(1 - (g_dan.RadHoriz[i]/fRadHorizMin));
            fRadVert[i] = float(1 - (g_dan.RadVert[i]/fRadVertMin));
        }
    }

    double TRadHoriz[361];           // передаем кривым подготовленные данные
    double TRadVert[361];
    double TAzVert[361];

    for (int k=0; k!=360; k++) {
        TAzVert[k]=-180+k;
        if (k<180) {
            TRadVert[k]=fRadVert[-(-k-180)];
            TRadHoriz[k]=fRadHoriz[-(-k-180)];
        } else {
            TRadVert[k]=fRadVert[k-180];
            TRadHoriz[k]=fRadHoriz[k-180];
        }
    }

    // Помещаем кривые на график
    CurvDecHoriz->setSamples(TAzVert,TRadHoriz,N1-1);
    CurvDecVert->setSamples(TAzVert,TRadVert,N1-1);
    CurvDecHoriz->attach(ui->PlotDec);
    CurvDecVert->attach(ui->PlotDec);

    if( !(ui->action_HideHorizDN->isChecked()) ) { CurvDecHoriz->hide(); }
    if( !(ui->action_HideVertDN->isChecked()) ) { CurvDecVert->hide(); }

    ui->PlotDec->replot();
}


// --------------------------------------------------- Файл - Открыть --------------------------------------------------- //
void DnView::action_Open()
{
    QString f = QFileDialog::getOpenFileName(this,
                                             tr("Открыть фаил ДН"), "",
                                             tr("All support files (*.msi *.pln *.txt *.bdn *.adf);;MSI/Nokia (*.msi *.pln);;NSMA (*.adf);;Inside (*.bdn);;Text file (*.txt);;All files (*)")
                                             );
    OpenFile(f);
}


// --------------------------------------------------- Открытие файла (Drag'n'Drop) --------------------------------------------------- //
bool DnView::OpenFile(const QString &f)
{
    if (!f.isEmpty())
    {
        if (f.split(".").last().toLower() == "msi" || f.split(".").last().toLower() == "pln" || f.split(".").last().toLower() == "txt") {
            g_dan.loadMsi(f);
            loadedPattern();
            addRecentFiles(f);
            return true;
        }
        if (f.split(".").last().toLower() == "adf") {
            g_dan.loadNsma(f);
            loadedPattern();
            addRecentFiles(f);
            return true;
        }
        if (f.split(".").last().toLower() == "bdn") {
            g_dan.loadedBdn(f);
            addRecentFiles(f);
            return true;
        }
        else {
            QMessageBox::warning(this, tr("Error"), tr("Ошибка открытия файла!"));
            return false;
        }
    }

    return true;
}

// --------------------------------------------------- Загрузка диаграммы --------------------------------------------------- //
bool DnView::loadedPattern()
{
    EnableUI(true);
    initPolarGraph(g_dan);
    initDecartGraph(g_dan);
    setDanGraph(g_dan);

    return true;
}


// --------------------------------------------------- Отправляем данные в табличку --------------------------------------------------- //
void DnView::setDanGraph(Prto g_dan)
{
    QTableWidgetItem* iName = new QTableWidgetItem(g_dan.Name);                     ui->tableWidget_Data1->setItem(0,1,iName);       // Название
    QTableWidgetItem* iFreq = new QTableWidgetItem(QString::number(g_dan.Freq));    ui->tableWidget_Data1->setItem(1,1,iFreq);       // Частота
    QTableWidgetItem* iGain = new QTableWidgetItem(QString::number(g_dan.Gain));    ui->tableWidget_Data1->setItem(2,1,iGain);       // КУ
    QTableWidgetItem* iTilt = new QTableWidgetItem(g_dan.ETilt);                     ui->tableWidget_Data1->setItem(3,1,iTilt);       // Угол
    QTableWidgetItem* iCom = new QTableWidgetItem(g_dan.Comment);                   ui->tableWidget_Data1->setItem(4,1,iCom);        // Коментарии
    for (int k=0; k<=360; k++) {
        QTableWidgetItem* iAzH = new QTableWidgetItem(QString::number(g_dan.AzHoriz[k]));       ui->tableWidget_Data2->setItem(k,0,iAzH);
        QTableWidgetItem* iRadV = new QTableWidgetItem(QString::number(fabs(g_dan.RadVert[k])));     ui->tableWidget_Data2->setItem(k,1,iRadV);
        QTableWidgetItem* iRadH = new QTableWidgetItem(QString::number(fabs(g_dan.RadHoriz[k])));     ui->tableWidget_Data2->setItem(k,2,iRadH);
    }
}


// --------------------------------------------------- Берём данные из таблички табличку --------------------------------------------------- //
Prto DnView::getDanGraph()
{
    Prto getDan;

    getDan.Name = ui->tableWidget_Data1->item(0,1)->text();
    getDan.Freq = ui->tableWidget_Data1->item(1,1)->text().toDouble();
    getDan.Gain = ui->tableWidget_Data1->item(2,1)->text().toDouble();
    getDan.ETilt = ui->tableWidget_Data1->item(3,1)->text();
    getDan.Comment = ui->tableWidget_Data1->item(4,1)->text();

    for (int k=0; k<=359; k++) {
        getDan.AzHoriz[k] = ui->tableWidget_Data2->item(k,0)->text().toDouble();
        getDan.AzVert[k] = ui->tableWidget_Data2->item(k,0)->text().toDouble();
        getDan.RadVert[k] = -(ui->tableWidget_Data2->item(k,1)->text().toDouble());
        getDan.RadHoriz[k] = -(ui->tableWidget_Data2->item(k,2)->text().toDouble());
    }

    return getDan;
}


// --------------------------------------------------- О Программе --------------------------------------------------- //
void DnView::action_AboutThis()
{
    QMessageBox::about(this, tr("О программе"), tr("DnView \n"
                                                   "Программа просмотра и редактирования диаграмм направленности антенн \n"
                                                   "Version: 1.1 \n \n"
                                                   "Автор: Владимир Kansept \n"
                                                   "e-mail: kansept@yandex.ru"));
}


// --------------------------------------------------- Файл -> Сохранить --------------------------------------------------- //
void DnView::action_Save()
{  
    if (ui->action_RecentFiles0->text().isEmpty()) {
        return;
    } else {
        g_dan.saveAsMsi(ui->action_RecentFiles0->text());
    }
}


// --------------------------------------------------- Файл -> Сохранить как --------------------------------------------------- //
void DnView::action_SaveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save As"), g_dan.Name,
                                                    tr("MSI/Nokia (*.msi);;All Files (*)"));

    g_dan.saveAsMsi(fileName);
}


// --------------------------------------------------- Файл -> Экспорт --------------------------------------------------- //
void DnView::action_Export()
{
    QString fileName = g_dan.Name;

#ifndef QT_NO_FILEDIALOG
    const QList<QByteArray> imageFormats =
            QImageWriter::supportedImageFormats();
    QStringList filter;
    filter += "PDF Documents (*.pdf)";

#ifndef QWT_NO_POLAR_SVG
    filter += "SVG Documents (*.svg)";
#endif

    filter += "Postscript Documents (*.ps)";
    if ( imageFormats.size() > 0 )
    {
        QString imageFilter( "Images (" );
        for ( int i = 0; i < imageFormats.size(); i++ )
        {
            if ( i > 0 )
                imageFilter += " ";
            imageFilter += "*.";
            imageFilter += imageFormats[i];
        }
        imageFilter += ")";
        filter += imageFilter;
    }
    fileName = QFileDialog::getSaveFileName(
                this, "Export File Name", fileName,
                filter.join( ";;" ), NULL, QFileDialog::DontConfirmOverwrite );
#endif
    if ( !fileName.isEmpty() )
    {

        if( ui->tabWidget->currentIndex()==0 ) {
            QBuffer b;
            QwtPolarRenderer renderer;
            renderer.renderDocument( ui->PlotPolar, fileName, QSizeF( 300, 200 ), 85 );

        }
        if( ui->tabWidget->currentIndex()==1 ) {
            QwtPlotRenderer renderer;
            renderer.renderDocument( ui->PlotDec, fileName, QSizeF( 300, 200 ), 85 );
        }
    }
}


// --------------------------------------------------- Файл -> Сформировать паспорт --------------------------------------------------- //
void DnView::setDataSheet()
{
    passportWord(getDanGraph());
}


/* --------------------------------------------------- Генерация паспорта --------------------------------------------------- */
void DnView::passportWord(Prto prtoPassp)
{
    settings->beginGroup("settings_DnView");
    int numTemplate = settings->value("DnViewTemplate", "0").toInt();
    settings->endGroup();

    QDir qdCache(QCoreApplication::applicationDirPath()+"/Cache");
    if( !qdCache.exists() )
    {
        qdCache.mkdir(qdCache.absolutePath());
    }
    // Делаем рисунки
    QFile fHorizPattern(qdCache.absolutePath() + "/HorizPattern.jpg");
    QFile fVertPattern(qdCache.absolutePath() + "/VertPattern.jpg");
    QwtPolarRenderer qwtPlrRndr;
    QwtPlotRenderer qwtDecRndr;
    // Горизонтальная
    on_actionViewHorizDn(true);
    on_actionViewVertDn(false);
    if(numTemplate == 0)
    {
        qwtPlrRndr.renderDocument( ui->PlotPolar, fHorizPattern.fileName(), QSizeF( 80, 80 ), 90 );
    }
    else if(numTemplate == 1) {
        qwtDecRndr.renderDocument( ui->PlotDec, fHorizPattern.fileName(), QSizeF( 180, 90 ), 90 );
    }
    // Вертикальная
    on_actionViewHorizDn(false);
    on_actionViewVertDn(true);

    if(numTemplate == 0)
    {
        qwtPlrRndr.renderDocument( ui->PlotPolar, fVertPattern.fileName(), QSizeF( 80, 80 ), 90 );
    }
    else if(numTemplate == 1)
    {
        qwtDecRndr.renderDocument( ui->PlotDec, fVertPattern.fileName(), QSizeF( 180, 90 ), 90 );
    }
    on_actionViewVertDn(true);
    on_actionViewHorizDn(true);

    QAxObject* WordApplication = new QAxObject("Word.Application");                  // Создем интерфейс к MSWord
    WordApplication->querySubObject( "Documents()" )->querySubObject("Add()");       // Создаю новый документ
    WordApplication->setProperty("Visible", false);                                  // Делаем Word видимым
    QAxObject* ActiveDocument = WordApplication->querySubObject("ActiveDocument()"); // Активный документ
    QAxObject* ActiveWindow = WordApplication->querySubObject("ActiveWindow()");     // Активное окно
    QAxObject* WordSelection = WordApplication->querySubObject("Selection");         // Выделение

    // Загловок
    QAxObject *Font = WordSelection->querySubObject("Font");
    Font->setProperty("Size", 22);
    Font->setProperty("Bold", true);
    QAxObject *ParagraphFormat = WordSelection->querySubObject("ParagraphFormat");
    ParagraphFormat->setProperty("Alignment", "wdAlignParagraphCenter");

    WordSelection->dynamicCall("TypeText(const QString&)", prtoPassp.Name);
    WordSelection->dynamicCall("TypeParagraph()");

    // Сбрасываем параметры шрифта
    Font->setProperty("Size", 12);
    Font->setProperty("Bold", false);
    ParagraphFormat->setProperty("Alignment", "wdAlignParagraphLeft");

    // Создаём таблицу
    QAxObject* Range = ActiveDocument->querySubObject("Range()");
    Range->dynamicCall( "Expand(wdTable)" );
    Range->dynamicCall( "Collapse(0)" );

    ActiveDocument->querySubObject("Tables()")->querySubObject("Add(QVariant&, QVariant&, QVariant&, QVariant&, QVariant&)", Range->asVariant(),5,2,1,2);;
    Range->dynamicCall( "Expand(wdTable)" );
    Range->dynamicCall( "Collapse(0)" );

    // Заполняем таблицу
    QAxObject* rangeTab = ActiveDocument->querySubObject("Tables(1)") ;
    rangeTab->querySubObject("Rows()") ;
    // 1-я строка
    rangeTab->querySubObject("Cell(Row, Column)" , 1,1)->querySubObject("Range()")->dynamicCall("InsertAfter(Text)" ,"Частотный диапозон, МГц:") ;
    rangeTab->querySubObject("Cell(Row, Column)" , 1,2)->querySubObject("Range()")->dynamicCall("InsertAfter(Text)" ,prtoPassp.Freq) ;
    // 2-я строка
    rangeTab->querySubObject("Cell(Row, Column)" , 2,1)->querySubObject("Range()")->dynamicCall("InsertAfter(Text)" ,"Коэффициент усиления антенны, дБi:");
    rangeTab->querySubObject("Cell(Row, Column)" , 2,2)->querySubObject("Range()")->dynamicCall("InsertAfter(Text)" ,prtoPassp.Gain) ;
    // 3-я строка
    rangeTab->querySubObject("Cell(Row, Column)" , 3,1)->querySubObject("Range()")->dynamicCall("InsertAfter(Text)" ,"Угол наклона:");
    rangeTab->querySubObject("Cell(Row, Column)" , 3,2)->querySubObject("Range()")->dynamicCall("InsertAfter(Text)" ,QString::number(prtoPassp.etilt()));
    // 4-я строка
    rangeTab->querySubObject("Cell(Row, Column)" , 4,1)->querySubObject("Range()")->dynamicCall("InsertAfter(Text)" ,"Ширина ГДН:") ;
    rangeTab->querySubObject("Cell(Row, Column)" , 4,2)->querySubObject("Range()")->dynamicCall("InsertAfter(Text)" ,QString::number(prtoPassp.beamwidthHoriz())) ;
    // 5-я строка
    rangeTab->querySubObject("Cell(Row, Column)" , 5,1)->querySubObject("Range()")->dynamicCall("InsertAfter(Text)" ,"Ширина BДН:");
    rangeTab->querySubObject("Cell(Row, Column)" , 5,2)->querySubObject("Range()")->dynamicCall("InsertAfter(Text)" ,QString::number(prtoPassp.beamwidthVert()));

    WordSelection->querySubObject("EndKey(QVariant&)", 6);  // Переходим в конец документа (Selection.EndKey Unit:=wdStory)
    ParagraphFormat->setProperty("Alignment", "wdAlignParagraphCenter");
    WordSelection->dynamicCall("TypeParagraph()");
    WordSelection->querySubObject("InlineShapes")->querySubObject("AddPicture(const QVariant&)", fHorizPattern.fileName());
    WordSelection->dynamicCall("TypeParagraph()");
    WordApplication->querySubObject("Selection")->querySubObject("TypeText(QVariant&)", "Диаграмма направленности антенны в горизонтальной плоскости");
    WordSelection->dynamicCall("TypeParagraph()");

    WordSelection->dynamicCall("TypeParagraph()");
    WordSelection->querySubObject("InlineShapes")->querySubObject("AddPicture(const QVariant&)", fVertPattern.fileName());
    WordSelection->dynamicCall("TypeParagraph()");
    WordApplication->querySubObject("Selection")->querySubObject("TypeText(QVariant&)", "Диаграмма направленности антенны в вертикальной плоскости");

    WordApplication->querySubObject("WordBasic")->querySubObject("ViewFooterOnly");
    WordApplication->querySubObject("Selection")->querySubObject("TypeText(QVariant&)", "Сформированно в программе DnView");

    // ActiveWindow.ActivePane.View.SeekView = wdSeekMainDocument
    ActiveWindow->querySubObject("ActivePane")->querySubObject("View")->setProperty("SeekView", "wdSeekMainDocument");
    WordApplication->setProperty("Visible", true);   // Делаем Word видимым

    // Удаляем файлы
    fHorizPattern.remove();
    fVertPattern.remove();
}


// --------------------------------------------------- Показать/скрыть горизонтальную диаграмму ---------------------------------- //
void DnView::on_actionViewHorizDn(bool b)
{
    if (b == false)   { CurvPolarHoriz->hide(); CurvDecHoriz->hide();}
    if (b == true)   { CurvPolarHoriz->show(); CurvDecHoriz->show();}
    ui->PlotPolar->replot();
    ui->PlotDec->replot();
}


// --------------------------------------------------- Показать/скрыть вертикальную диаграмму ------------------------------------ //
void DnView::on_actionViewVertDn(bool b)
{
    if (b == false)  { CurvPolarVert->hide(); CurvDecVert->hide(); }
    if (b == true)   { CurvPolarVert->show(); CurvDecVert->show(); }
    ui->PlotPolar->replot();
    ui->PlotDec->replot();
}
void DnView::action_ColorHorizChange(int iIndex)
{
    QString styleLeg = "color: rgb(%1, %2, %3);";
    if (iIndex == 1) {
        qcHorizDN = QColorDialog::getColor(Qt::white, this);
        ui->labelPolar_ColorHorizDN_Box->setStyleSheet( styleLeg.arg(qcHorizDN.red()).arg(qcHorizDN.green()).arg( qcHorizDN.blue()) );
        ui->labelDec_ColorHorizDN_Box->setStyleSheet( styleLeg.arg(qcHorizDN.red()).arg(qcHorizDN.green()).arg( qcHorizDN.blue()) );
        initPolarGraph(g_dan);
        initDecartGraph(g_dan);
    }
    if (iIndex == 2) {
        qcVertDN = QColorDialog::getColor(Qt::white, this);
        ui->labelPolar_ColorVertzDN_Box->setStyleSheet( styleLeg.arg(qcVertDN.red()).arg(qcVertDN.green()).arg( qcVertDN.blue()) );
        ui->labelDec_ColorVertDN_Box->setStyleSheet( styleLeg.arg(qcVertDN.red()).arg(qcVertDN.green()).arg( qcVertDN.blue()) );
        initPolarGraph(g_dan);
        initDecartGraph(g_dan);
    }
}


// --------------------------------------------------- Масштаб графика 25 dB --------------------------------------------------- //
void DnView::graphScale25db(bool b)
{
    if (b==true) {
        g_GraphScale = 25;
        ui->action_Scale25dB->setChecked(true);
        ui->action_Scale40dB->setChecked(false);
        ui->action_ScaleNorm->setChecked(false);
    }

    initPolarGraph(g_dan);
    initDecartGraph(g_dan);
    ui->PlotPolar->replot();
    toolButton_MenuScale->setText(QString::number(g_GraphScale)+" dB ");
}


// --------------------------------------------------- Масштаб графика 40 dB ---------------------------------------------------- //
void DnView::graphScale40db(bool b)
{
    if (b==true) {
        g_GraphScale = 40;
        ui->action_Scale25dB->setChecked(false);
        ui->action_Scale40dB->setChecked(true);
        ui->action_ScaleNorm->setChecked(false);
    }

    initPolarGraph(g_dan);
    initDecartGraph(g_dan);
    ui->PlotPolar->replot();
    toolButton_MenuScale->setText(QString::number(g_GraphScale)+" dB ");
}


// --------------------------------------------------- Масштаб графика 40 dB ---------------------------------------------------- //
void DnView::graphScaleNorm(bool b)
{
    if (b==true) {
        g_GraphScale = 1;
        ui->action_Scale25dB->setChecked(false);
        ui->action_Scale40dB->setChecked(false);
        ui->action_ScaleNorm->setChecked(true);
    }

    initPolarGraph(g_dan);
    initDecartGraph(g_dan);
    ui->PlotPolar->replot();
    toolButton_MenuScale->setText("Норм ");

}


// --------------------------------------------------- Поворот по часовой ---------------------------------------------------- //
void DnView::setPatternRotateCw()
{
    g_dan = getDanGraph();
    if(ui->checkBox_PatternHoriz->isChecked()) { g_dan.rotate(1, 0); }
    if(ui->checkBox_PatternVert->isChecked())  { g_dan.rotate(0, 1); }
    initAllGraph();
}


void DnView::setPatternRotateCcw()
{
    g_dan = getDanGraph();
    if(ui->checkBox_PatternHoriz->isChecked()) { g_dan.rotate(-1, 0); }
    if(ui->checkBox_PatternVert->isChecked())  { g_dan.rotate(0, -1); }
    initAllGraph();
}


void DnView::setPatternSymmetryBottom() {
    g_dan = getDanGraph();
    if(ui->checkBox_PatternHoriz->isChecked()) { g_dan.symmetryBottom(true, false); }
    if(ui->checkBox_PatternVert->isChecked())  { g_dan.symmetryBottom(false, true); }
    initAllGraph();
}


void DnView::setPreload()
{
    g_dan = getDanGraph();
    if(ui->checkBox_PatternVert->isChecked())  { g_dan.preloadPatternVert(); }
    initAllGraph();
}

void DnView::setPatternSymmetryTop() {
    g_dan = getDanGraph();
    if(ui->checkBox_PatternHoriz->isChecked()) { g_dan.symmetryTop(true, false); }
    if(ui->checkBox_PatternVert->isChecked())  { g_dan.symmetryTop(false, true); }
    initAllGraph();
}

void DnView::isEditPattern()
{
    ui->action_isEditPatternHoriz->setChecked(ui->checkBox_PatternHoriz->isChecked());
    ui->action_isEditPatternVert->setChecked(ui->checkBox_PatternVert->isChecked());

    if( !(ui->checkBox_PatternHoriz->isChecked()) && !(ui->checkBox_PatternVert->isChecked()) ) {
        enableEditPatterns(false);
    } else {
        enableEditPatterns(true);
    }

    if(ui->checkBox_PatternVert->isChecked()) {
        ui->action_EditPreload->setEnabled(true);
        ui->toolButton_Preload->setEnabled(true);
    }
    else {
        ui->action_EditPreload->setEnabled(false);
        ui->toolButton_Preload->setEnabled(false);
    }
}

void DnView::isEditPatternInMenu()
{
    ui->checkBox_PatternHoriz->setChecked(ui->action_isEditPatternHoriz->isChecked());
    ui->checkBox_PatternVert->setChecked(ui->action_isEditPatternVert->isChecked());

    if( !(ui->checkBox_PatternHoriz->isChecked()) && !(ui->checkBox_PatternVert->isChecked()) ) {
        enableEditPatterns(false);
    } else {
        enableEditPatterns(true);
    }

    if(ui->action_isEditPatternVert->isChecked()) {
        ui->action_EditPreload->setEnabled(true);
        ui->toolButton_Preload->setEnabled(true);
    }
    else {
        ui->action_EditPreload->setEnabled(false);
        ui->toolButton_Preload->setEnabled(false);
    }
}

// --------------------------------------------------- Отзеркаливание горизонтальной ДН ------------------------------------------ //
void DnView::action_PatternMirror()
{
    g_dan = getDanGraph();
    if(ui->checkBox_PatternHoriz->isChecked()) { g_dan.mirorrHoriz(); }
    if(ui->checkBox_PatternVert->isChecked()) { g_dan.mirorrsVert(); }
    initAllGraph();
}
void DnView::enableEditPatterns(bool b)
{
    ui->widget_TransformTools->setEnabled(b);
    ui->action_EditPatternMirror->setEnabled(b);
    ui->action_EditPatternRotateCw->setEnabled(b);
    ui->action_EditPatternRotateCcw->setEnabled(b);
    ui->action_EditPattermSymmetryTop->setEnabled(b);
    ui->action_EditPattermSymmetryBottom->setEnabled(b);
    ui->action_EditPreload->setEnabled(b);
}
// --------------------------------------------------- Узнать Электричечкий угол ДН ---------------------------------------------- //
void DnView::EnableUI(bool b)
{
    ui->action_Save->setEnabled(b);
    ui->action_SaveAs->setEnabled(b);
    ui->action_Export->setEnabled(b);
    ui->action_Close->setEnabled(b);

    ui->widget_Transform->setEnabled(b);
    ui->menu_Scale->setEnabled(b);
    toolButton_MenuScale->setEnabled(b);
    ui->menu_Hide->setEnabled(b);
    ui->action_HideHorizDN->setEnabled(b);
    ui->action_HideVertDN->setEnabled(b);
    ui->action_OpenInTextEdit->setEnabled(b);
    ui->action_DataSheet->setEnabled(b);
    ui->menu_Edit->setEnabled(b);

    if(b == true) {
        ui->tableWidget_Data1->setEnabled(b);
        ui->tableWidget_Data2->setEnabled(b);
        ui->tableWidget_Data1->setItemDelegateForColumn(0, new NonEditTableColumnDelegate());
    }
    if(b == false) {
        ui->tableWidget_Data1->setEnabled(b);
        ui->tableWidget_Data2->setEnabled(b);
    }
}


// --------------------------------------------------- Контекстное меню --------------------------------------------------- //
void DnView::menuPolar_ContextMenu(const QPoint &pos)
{
    QMenu menuPolar(this);

    QAction *actCopyToBuf = new QAction(tr("Копировать..."), this);
    connect(actCopyToBuf, SIGNAL(triggered()), this, SLOT(CopyToBuf()));
    menuPolar.addAction(actCopyToBuf);

    QAction *actCopyAsImg = new QAction(tr("Сохранить как изображение..."), this);
    connect( actCopyAsImg, SIGNAL(triggered()), this, SLOT(action_Export()) );
    menuPolar.addAction(actCopyAsImg);

    menuPolar.addSeparator();

    QAction *Pref = new QAction(tr("Настройки"), this);
    connect( Pref, SIGNAL(triggered()), dlgPreferens, SLOT(show()) );
    menuPolar.addAction(Pref);

    if (ui->tableWidget_Data1->isEnabled() == true) {
        actCopyToBuf->setEnabled(true);
        actCopyAsImg->setEnabled(true);
    } else {
        actCopyToBuf->setEnabled(false);
        actCopyAsImg->setEnabled(false);
    }

    menuPolar.exec(ui->tab_Decart->mapToGlobal(pos));
}


// --------------------------------------------------- Контекстное меню -> Копировать как изображение в буфер ------------------------ //
void DnView::CopyToBuf()
{
    QBuffer b;

    QSvgGenerator p;
    p.setOutputDevice(&b);
    p.setSize(QSize(800,600));
    p.setViewBox(QRect(0,0,800,600));

    QPainter painter;
    QwtPolarRenderer renderer;

    painter.begin(&p);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(ui->PlotPolar,&painter,QRectF(0,0,600,600));

    painter.end();

    QMimeData * d = new QMimeData();
    d->setData("image/svg+xml",b.buffer());
    d->setData("image/svg+xml-compressed",b.buffer());

    QApplication::clipboard()->setMimeData(d,QClipboard::Clipboard);
}


// --------------------------------------------------- Утилиты -> Открыть в текстовом редакторе --------------------------------------------------- //
void DnView::openInTextEdit() {

    QProcess* proc = new QProcess(this);
    proc->start( "notepad " + ui->action_RecentFiles0->text() );
}


// --------------------------------------------------- Показать/Скрыть панель с данными --------------------------------------------------- //
void  DnView::on_actionViewPanelData(bool b) {
    ui->dockWidget_Data->setVisible(b);
    conf_bViewPanelData = b;
}


// --------------------------------------------------- Показать/Скрыть панель трансформации --------------------------------------------------- //
void  DnView::on_actionViewPanelTransform(bool b) {
    ui->dockWidget_Transform->setVisible(b);
    conf_bViewPanelTransform = b;
}


// --------------------------------------------------- Файл - Закрыть --------------------------------------------------- //
void DnView::closeDN()
{
    if((CurvPolarHoriz)) {
        delete CurvPolarHoriz;
        delete CurvPolarVert;
        delete CurvDecHoriz;
        delete CurvDecVert;
        ui->PlotPolar->setTitle("");
        ui->PlotDec->setTitle("");
        ui->PlotPolar->replot();
        ui->PlotDec->replot();
    }

    g_dan.clear();
    setDanGraph(g_dan);

    ui->tableWidget_Data2->clearContents();
    EnableUI(false);
}


// --------------------------------------------------- Недавние файлы --------------------------------------------------- //
void DnView::addRecentFiles(const QString qfopen)
{
    QString fFileName;
    fFileName = qfopen;
    fFileName.replace("\\","/");
    this->setWindowTitle("DnView - " + fFileName.split("/").at(fFileName.split("/").size()-1));

    if (qfopen != conf_qsRecentFiles[0]) {
        for (int i=9; i>=1; i--) {
            conf_qsRecentFiles[i]=conf_qsRecentFiles[i-1];
        }
        conf_qsRecentFiles[0] = qfopen;

        settings->beginGroup("settings_DnView");
        settings->setValue( "RecentFiles0", conf_qsRecentFiles[0]);
        settings->setValue( "RecentFiles1", conf_qsRecentFiles[1]);
        settings->setValue( "RecentFiles2", conf_qsRecentFiles[2]);
        settings->setValue( "RecentFiles3", conf_qsRecentFiles[3]);
        settings->setValue( "RecentFiles4", conf_qsRecentFiles[4]);
        settings->setValue( "RecentFiles5", conf_qsRecentFiles[5]);
        settings->setValue( "RecentFiles6", conf_qsRecentFiles[6]);
        settings->setValue( "RecentFiles7", conf_qsRecentFiles[7]);
        settings->setValue( "RecentFiles8", conf_qsRecentFiles[8]);
        settings->setValue( "RecentFiles9", conf_qsRecentFiles[9]);
        settings->endGroup();

        settings->sync();

        ui->action_RecentFiles0->setText(conf_qsRecentFiles[0]);
        ui->action_RecentFiles1->setText(conf_qsRecentFiles[1]);
        ui->action_RecentFiles2->setText(conf_qsRecentFiles[2]);
        ui->action_RecentFiles3->setText(conf_qsRecentFiles[3]);
        ui->action_RecentFiles4->setText(conf_qsRecentFiles[4]);
        ui->action_RecentFiles5->setText(conf_qsRecentFiles[5]);
        ui->action_RecentFiles6->setText(conf_qsRecentFiles[6]);
        ui->action_RecentFiles7->setText(conf_qsRecentFiles[7]);
        ui->action_RecentFiles8->setText(conf_qsRecentFiles[8]);
        ui->action_RecentFiles9->setText(conf_qsRecentFiles[9]);
    }
}


// --------------------------------------------------- Недавние файлы --------------------------------------------------- //
void DnView::on_action_RecentFiles0_triggered() { if (ui->action_RecentFiles0->text() != "-") OpenFile(ui->action_RecentFiles0->text()); }
void DnView::on_action_RecentFiles1_triggered() { if (ui->action_RecentFiles1->text() != "-") OpenFile(ui->action_RecentFiles1->text()); }
void DnView::on_action_RecentFiles2_triggered() { if (ui->action_RecentFiles2->text() != "-") OpenFile(ui->action_RecentFiles2->text()); }
void DnView::on_action_RecentFiles3_triggered() { if (ui->action_RecentFiles3->text() != "-") OpenFile(ui->action_RecentFiles3->text()); }
void DnView::on_action_RecentFiles4_triggered() { if (ui->action_RecentFiles4->text() != "-") OpenFile(ui->action_RecentFiles4->text()); }
void DnView::on_action_RecentFiles5_triggered() { if (ui->action_RecentFiles5->text() != "-") OpenFile(ui->action_RecentFiles5->text()); }
void DnView::on_action_RecentFiles6_triggered() { if (ui->action_RecentFiles6->text() != "-") OpenFile(ui->action_RecentFiles6->text()); }
void DnView::on_action_RecentFiles7_triggered() { if (ui->action_RecentFiles7->text() != "-") OpenFile(ui->action_RecentFiles7->text()); }
void DnView::on_action_RecentFiles8_triggered() { if (ui->action_RecentFiles8->text() != "-") OpenFile(ui->action_RecentFiles8->text()); }
void DnView::on_action_RecentFiles9_triggered() { if (ui->action_RecentFiles9->text() != "-") OpenFile(ui->action_RecentFiles9->text()); }
