#-------------------------------------------------
#
# Project created by QtCreator 2014-04-07T21:13:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = taxman
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11 -Werror -Wno-unused-variable -Wformat-security -fvisibility=hidden -Wpointer-arith -Winit-self

QMAKE_CXXFLAGS_DEBUG += -g3 -ggdb3

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

VPATH += ./src
SOURCES += main.cpp\
        mainwindow.cpp \
	image_processing/algorithms.cpp \
	image_processing/binarizewolfjolion.cpp \
	qclickablegraphicsview.cpp

HEADERS  += mainwindow.h \
	image_processing/algorithms.h \
	image_processing/binarizewolfjolion.h \
	qclickablegraphicsview.h

INCLUDEPATH += ./src

FORMS    += mainwindow.ui

unix {
	QMAKE_CXX = g++
    CONFIG += link_pkgconfig
    PKGCONFIG = opencv
}

win32 {
    QMAKE_CXX = g++
    INCLUDEPATH += C:/opencv/build/include
    LIBS += -L"C:/opencv/sources/release/install/x64/mingw/bin"
    LIBS += -lopencv_core248 -lopencv_video248 -lopencv_videostab248 -lopencv_imgproc248 -lopencv_highgui248
}
