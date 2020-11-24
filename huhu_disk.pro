#-------------------------------------------------
#
# Project created by QtCreator 2020-10-28T11:17:12
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = huhu_disk
TEMPLATE = app
CONFIG   += c++11

SOURCES += main.cpp \
    mywidget/mainwindow.cpp \
    mywidget/login.cpp \
    common/common.cpp \
    common/des.c \
    mywidget/buttongroup.cpp \
    mywidget/titlewidget.cpp \
    common/logininfoinstance.cpp \
    mywidget/myfiles.cpp \
    mywidget/ranklist.cpp \
    mywidget/sharelist.cpp \
    mywidget/trans.cpp \
    common/uploadtask.cpp \
    mywidget/filepropertyinfo.cpp \
    mywidget/dataprogress.cpp \
    common/uploadlayout.cpp \
    common/downloadlayout.cpp \
    common/downloadtask.cpp \
    mywidget/menu.cpp


HEADERS  += mywidget/mainwindow.h \
    mywidget/login.h \
    mywidget/titlewidget.h \
    common/common.h \
    common/des.h \
    mywidget/buttongroup.h \
    common/logininfoinstance.h \
    mywidget/myfiles.h \
    mywidget/ranklist.h \
    mywidget/sharelist.h \
    mywidget/trans.h \
    common/uploadtask.h \
    mywidget/filepropertyinfo.h \
    mywidget/dataprogress.h \
    common/uploadlayout.h \
    common/downloadlayout.h \
    common/downloadtask.h \
    mywidget/menu.h

FORMS    += ui/mainwindow.ui \
    ui/login.ui \
    ui/titlewidget.ui \
    ui/buttongroup.ui \
    ui/myfiles.ui \
    ui/ranklist.ui \
    ui/sharelist.ui \
    ui/trans.ui \
    mywidget/filepropertyinfo.ui \
    mywidget/dataprogress.ui

RESOURCES += \
    qsrc.qrc


