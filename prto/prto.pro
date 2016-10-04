#-------------------------------------------------
#
# Project created by QtCreator 2014-10-31T08:26:08
#
#-------------------------------------------------

QT       -= gui

TARGET = prto
TEMPLATE = lib

DEFINES += PRTO_LIBRARY

SOURCES += \
    prto.cpp \
    task.cpp

HEADERS +=\
    prto_global.h \
    prto.h \
    task.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
