#-------------------------------------------------
#
# Project created by QtCreator 2015-02-05T23:13:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CodeMetrics
TEMPLATE = app

SOURCES +=\
        MainWindow.cpp \
    Main.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

DISTFILES += \
    Icon.ico \
    Resources.rc \
    LICENSE.txt \
    README.md

RESOURCES += \
    CodeMetrics.qrc

RC_FILE = Resources.rc
