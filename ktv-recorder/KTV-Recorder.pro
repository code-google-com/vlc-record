#-------------------------------------------------
#
# Project created by QtCreator 2011-01-09T01:35:01
#
#-------------------------------------------------

QT       += core gui \
	    sql \
            network \
            xml \
            xmlpatterns

TEMPLATE = app

TARGET = KTV-Recorder

INCLUDEPATH += .

RESOURCES += \
    vlc-record.qrc \
    lcd.qrc

RC_FILE = vlc-record.rc

TRANSLATIONS = lang_de.ts \
               lang_ru.ts \

DEFINES += INCLUDE_LIBVLC

SOURCES += main.cpp\
        mainwindow.cpp \
    cepgbrowser.cpp \
    caboutdialog.cpp \
    csettingsdlg.cpp \
    cchannelsepgdlg.cpp \
    cvlcrecdb.cpp \
    cplaystate.cpp \
    cshowinfo.cpp \
    cwaittrigger.cpp \
    cchanlogo.cpp \
    clcddisplay.cpp \
    ctranslit.cpp \
    cvodbrowser.cpp \
    ckartinaxmlparser.cpp \
    cdirstuff.cpp \
    cpixloader.cpp \
    cstreamloader.cpp \
    ctimerrec.cpp \
    cvlcctrl.cpp \
    ckartinaclnt.cpp \
    cplayer.cpp \
    cvideoframe.cpp \
    clogfile.cpp \
    cinstruction.cpp \
    cshortcutgrabber.cpp \
    qchanlistdelegate.cpp \
    qftsettings.cpp \
    qshortcuttable.cpp

HEADERS  += mainwindow.h \
    customization.h \
    version_info.h \
    clogfile.h \
    cepgbrowser.h \
    caboutdialog.h \
    csettingsdlg.h \
    cchannelsepgdlg.h \
    cvlcrecdb.h \
    tables.h \
    cplaystate.h \
    cshortcutex.h \
    ctimerex.h \
    cshowinfo.h \
    playstates.h \
    cwaittrigger.h \
    cchanlogo.h \
    clcddisplay.h \
    ctranslit.h \
    cfavaction.h \
    cvodbrowser.h \
    ckartinaxmlparser.h \
    defdef.h \
    templates.h \
    cdirstuff.h \
    cpixloader.h \
    cstreamloader.h \
    ctimerrec.h \
    cvlcctrl.h \
    ckartinaclnt.h \
    cplayer.h \
    cvideoframe.h \
    cinstruction.h \
    cshortcutgrabber.h \
    qchanlistdelegate.h \
    qftsettings.h \
    qshortcuttable.h

FORMS    += forms/mainwindow.ui \
    forms/ctimerrec.ui \
    forms/csettingsdlg.ui \
    forms/caboutdialog.ui \
    forms/cplayer.ui \
    forms/cinstruction.ui \
    forms/cchannelsepgdlg.ui \
    forms/qftsettings.ui

win32:INCLUDEPATH += include

LIBS += -lvlc
win32:LIBS += -Llib

# translation stuff ...
# include (language.pri)
