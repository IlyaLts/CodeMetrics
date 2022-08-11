#-------------------------------------------------
#
# Project created by QtCreator 2015-02-05T23:13:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

TARGET = CodeMetrics
TEMPLATE = app

SOURCES +=\
    DirsFirstProxyModel.cpp \
    FileSelectorModel.cpp \
        MainWindow.cpp \
    Main.cpp \
    ProjectsList.cpp

HEADERS  += MainWindow.h \
    DirsFirstProxyModel.h \
    FileSelectorModel.h \
    ProjectsList.h

FORMS    += MainWindow.ui

DISTFILES += \
    Icon.ico \
    Resources.rc \
    LICENSE.txt \
    README.md

RESOURCES += \
    CodeMetrics.qrc

RC_FILE = Resources.rc
