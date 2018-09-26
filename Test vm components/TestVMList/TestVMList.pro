#-------------------------------------------------
#
# Project created by QtCreator 2018-04-29T09:06:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TestVMList
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        mainwindow.h

FORMS += \
        mainwindow.ui

unix:!macx: LIBS += -L$$PWD/../../vmLibs/libs/ -lvmUtils -lvmStringRecord -lvmNumbers \
												-lvmNotify -lvmCalculator -lvmWidgets -lvmTaskPanel

DEPENDPATH += $$PWD/../../vmLibs/libs $$PWD/../../vmLibs/common $$PWD/../../vmLibs/lib-src
INCLUDEPATH += $$PWD/../../vmLibs/lib-src $$PWD/../../vmLibs/common

DEFINES += DEBUG QT_USE_QSTRINGBUILDER QT_USE_FAST_CONCATENATION QT_USE_FAST_OPERATOR_PLUS

QMAKE_CXXFLAGS_RELEASE += -O3 -march=native -fomit-frame-pointer -funroll-loops -Ofast
#QMAKE_CXXFLAGS_DEBUG += -g -fvar-tracking-assignments-toggle
QMAKE_CXXFLAGS_DEBUG += -g
#QMAKE_CXXFLAGS += -g -fvar-tracking-assignments-toggle
QMAKE_CXXFLAGS += -Werror -Wall -Wextra -pedantic -std=c++14