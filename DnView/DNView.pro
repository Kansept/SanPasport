#-------------------------------------------------
#
# Project created by QtCreator 2012-07-22T08:38:28
#
#-------------------------------------------------

QT += core gui
QT += sql widgets
QT += printsupport
QT += svg xml
QT += widgets axcontainer

CONFIG += qwt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DNView
TEMPLATE = app


SOURCES += main.cpp\
           tools/dialogconvert.cpp \
           tools/dialogpreferens.cpp \
           tools/window_projection_pkaemo4.cpp \
    tools/dialog_groupchange.cpp \
    DnView.cpp

HEADERS += \
           tools/dialogconvert.h \
           tools/dialogpreferens.h \
           tools/window_projection_pkaemo4.h \
           userdelegate.h \
    tools/dialog_groupchange.h \
    version.h \
    DnView.h \
    ../MyWidget/DotLineEdit.h

FORMS   += \
           tools/dialogconvert.ui \
           tools/dialogpreferens.ui \
           tools/window_projection_pkaemo4.ui \
    tools/dialog_groupchange.ui \
    DnView.ui

include(../qwt.pri)
include(../qwtpolar.pri)

    SOURCES += qdata.cpp
    HEADERS += qdata.h

RC_FILE += myicon.rc

RESOURCES += \
    ../resourse.qrc

OTHER_FILES += \
    ../demo_qwtpolar-build-Qt_4_8_4-_______/settings.conf \



win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../prto/release/ -lprto
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../prto/debug/ -lprto
else:unix:!macx: LIBS += -L$$OUT_PWD/../prto/ -lprto

INCLUDEPATH += $$PWD/../prto
DEPENDPATH += $$PWD/../prto
