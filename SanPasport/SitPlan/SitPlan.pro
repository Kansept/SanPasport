#-------------------------------------------------
#
# Project created by QtCreator 2014-05-07T15:15:18
#
#-------------------------------------------------

QT += core gui
QT += sql widgets
QT += printsupport
QT += svg xml
QT += webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SitPlan
TEMPLATE = app


SOURCES += main.cpp\
        sitplan.cpp \
    Dialog/dialog_configdocument.cpp \
    Dialog/dialog_configzo.cpp \
    Dialog/dialog_configgrid.cpp

HEADERS  += sitplan.h\
    MyTableView.h \
    Dialog/dialog_configdocument.h \
    Dialog/dialog_configzo.h \
    Dialog/dialog_configgrid.h \
    config.h


FORMS    += sitplan.ui \
    Dialog/dialog_configdocument.ui \
    Dialog/dialog_configzo.ui \
    Dialog/dialog_configgrid.ui

include(../qwt.pri)
include(../qwtpolar.pri)

RESOURCES += \
    ../resourse.qrc

RC_FILE += myicon.rc

INCLUDEPATH += D:/Qt/Qt5.2.1/poppler/include/
LIBS += -LD:/Qt/Qt5.2.1/poppler/bin/ -llibpoppler-qt5
