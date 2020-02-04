#-------------------------------------------------
#
# Project created by QtCreator 2018-09-28T08:37:50
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = falcon2
TEMPLATE = app

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig debug
PKGCONFIG += mpv

SOURCES += main.cpp \
    mpvwidget.cpp \
    mainscreen.cpp \
    acr122u.cpp


HEADERS  += \
    mpvwidget.h \
    mainscreen.h \
    acr122u.h


FORMS    +=


LIBS += -lpcsclite
