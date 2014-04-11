#-------------------------------------------------
#
# Project created by QtCreator 2014-04-07T21:13:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = taxman
TEMPLATE = app

QMAKE_CXX = clang++
QMAKE_CXXFLAGS += -std=c++11 -Werror -fstack-protector -Wformat-security -fvisibility=hidden -Wpointer-arith -Winit-self
QMAKE_LFLAGS += -Wl,-z,now -Wl,--discard-all -Wl,--no-undefined -Wl,--build-id=sha1 -rdynamic

QMAKE_CXXFLAGS_DEBUG += -g3 -ggdb3

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

VPATH += ./src
SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

unix {
	CONFIG += link_pkgconfig
	PKGCONFIG = opencv
}
