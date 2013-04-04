#-------------------------------------------------
#
# Project created by QtCreator 2013-04-03T11:11:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QEasyCust
TEMPLATE = app

CONFIG += static

SOURCES += main.cpp\
        qeasycustdlg.cpp

HEADERS  += qeasycustdlg.h \
    file_templates.h

FORMS    += qeasycustdlg.ui

static {
    QTPLUGIN += qico
    win32:QMAKE_LFLAGS += -static-libgcc
    LIBS +=-Lc:/Qt/4.5.3/static/plugins/imageformats
}

RESOURCES += \
    icons.qrc

RC_FILE += QEasyCust.rc
