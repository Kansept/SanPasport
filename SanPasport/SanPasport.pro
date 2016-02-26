#-------------------------------------------------
#
# Project created by QtCreator 2013-10-22T11:38:26
#
#-------------------------------------------------

QT += core gui
QT += sql widgets
QT += printsupport
QT += svg xml
QT += concurrent widgets

CONFIG += qwt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SanPasport
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    Dialog/dialog_editant.cpp \
    Dialog/dialog_parametrs.cpp \
    qdata.cpp \
    Dialog/dialog_preferens.cpp \
    Task/dialog_taskzo.cpp \
    Task/dialog_taskvs.cpp \
    Task/dialog_taskpoint.cpp \
    SitPlan/sitplan.cpp \
    SitPlan/Dialog/dialog_configdocument.cpp \
    SitPlan/Dialog/dialog_configgrid.cpp \
    SitPlan/Dialog/dialog_configzo.cpp \
    createdb.cpp \
    ViewPlot/viewplot.cpp \
    MyWidget/qsingleitemsquarelayout.cpp \
    ViewPlot/dialog/dialog_confizgraph.cpp \
    MyWidget/plotzoz.cpp

HEADERS  += mainwindow.h \
    Dialog/dialog_editant.h \
    Dialog/dialog_parametrs.h \
    qdata.h \
    Dialog/dialog_preferens.h \
    Task/dialog_taskzo.h \
    Task/dialog_taskvs.h \
    Task/dialog_taskpoint.h \
    SitPlan/sitplan.h \
    SitPlan/Dialog/dialog_configdocument.h \
    SitPlan/Dialog/dialog_configgrid.h \
    SitPlan/Dialog/dialog_configzo.h \
    SitPlan/config.h \
    SitPlan/MyTableView.h \
    createdb.h \
    ../MyWidget/MySitPlanView.h \
    MyWidget/DotLineEdit.h \
    ../MyWidget/DotLineEdit.h \
    MyWidget/tableviewsp.h \
    ViewPlot/viewplot.h \
    MyWidget/qsingleitemsquarelayout.h \
    MyWidget/mymodeltask.h \
    MyWidget/sitplanview.h \
    ViewPlot/dialog/dialog_confizgraph.h \
    MyWidget/toolbuttoncolor.h \
    MyWidget/qsquarewidget.h \
    MyWidget/plotzoz.h

FORMS    += mainwindow.ui \
    Dialog/dialog_editant.ui \
    Dialog/dialog_parametrs.ui \
    Dialog/dialog_preferens.ui \
    Task/dialog_taskzo.ui \
    Task/dialog_taskvs.ui \
    Task/dialog_taskpoint.ui \
    SitPlan/sitplan.ui \
    SitPlan/Dialog/dialog_configdocument.ui \
    SitPlan/Dialog/dialog_configgrid.ui \
    SitPlan/Dialog/dialog_configzo.ui \
    ViewPlot/viewplot.ui \
    ViewPlot/dialog/dialog_confizgraph.ui

include(../qwt.pri)
include(../qwtpolar.pri)


INCLUDEPATH += D:/Qt/5.5/Src/qtbase/src/3rdparty/sqlite
SOURCES += D:/Qt/5.5/Src/qtbase/src/3rdparty/sqlite/sqlite3.c

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../prto/release/ -lprto
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../prto/debug/ -lprto
else:unix:!macx: LIBS += -L$$OUT_PWD/../prto/ -lprto

INCLUDEPATH += $$PWD/../prto
DEPENDPATH += $$PWD/../prto

RC_FILE += sanpasport.rc

RESOURCES += \
    ../resourse.qrc

OTHER_FILES += \
    SitPlan/myicon.rc