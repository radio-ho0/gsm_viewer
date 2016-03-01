#-------------------------------------------------
#
# Project created by QtCreator 2015-11-18T09:02:51
#
#-------------------------------------------------

QT += core gui
QT += serialport
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gsmviewer
TEMPLATE = app


SOURCES += main.cpp\
        gsmviewer.cpp \
    gsmtrack.cpp \
    basequery.cpp

HEADERS  += gsmviewer.h \
    gsmtrack.h \
    basequery.h

FORMS    += gsmviewer.ui
