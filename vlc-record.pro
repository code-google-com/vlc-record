# -------------------------------------------------
# Project created by QtCreator 2009-12-27T18:33:08
# -------------------------------------------------
TARGET = vlc-record-inc
QT += network
CONFIG += debug_and_release \
    windows

# debug version will be built shared ...
CONFIG(debug, debug|release):CONFIG += shared
else:CONFIG += static
TEMPLATE = app
INCLUDEPATH += . \
    include
SOURCES += main.cpp \
    recorder.cpp \
    cinifile.cpp \
    csettingsdlg.cpp \
    ckartinaclnt.cpp \
    ckartinaxmlparser.cpp \
    cwaittrigger.cpp \
    cepgbrowser.cpp \
    caboutdialog.cpp \
    clogfile.cpp \
    cchanlogo.cpp \
    ctimerrec.cpp \
    cvlcctrl.cpp \
    ctranslit.cpp \
    cplayer.cpp
HEADERS += recorder.h \
    chanlistwidgetitem.h \
    cinifile.h \
    csettingsdlg.h \
    ckartinaclnt.h \
    ckartinaxmlparser.h \
    cwaittrigger.h \
    templates.h \
    cepgbrowser.h \
    caboutdialog.h \
    version_info.h \
    clogfile.h \
    chttptime.h \
    cchanlogo.h \
    defdef.h \
    ctimerrec.h \
    cvlcctrl.h \
    customization.h \
    ctranslit.h \
    include/vlc/deprecated.h \
    include/vlc/libvlc.h \
    include/vlc/libvlc_events.h \
    include/vlc/libvlc_media.h \
    include/vlc/libvlc_media_discoverer.h \
    include/vlc/libvlc_media_library.h \
    include/vlc/libvlc_media_list.h \
    include/vlc/libvlc_media_list_player.h \
    include/vlc/libvlc_media_list_view.h \
    include/vlc/libvlc_media_player.h \
    include/vlc/libvlc_structures.h \
    include/vlc/libvlc_vlm.h \
    include/vlc/mediacontrol.h \
    include/vlc/mediacontrol_structures.h \
    include/vlc/vlc.h \
    cplayer.h \
    cfavaction.h
FORMS += recorder.ui \
    csettingsdlg.ui \
    caboutdialog.ui \
    ctimerrec.ui \
    cplayer.ui

RESOURCES += vlc-record.qrc
RC_FILE = vlc-record.rc
TRANSLATIONS = lang_de.ts \
    lang_ru.ts
LIBS += -Llib \
    -lvlc

# for static build ...
static {
    DEFINES += DSTATIC
    DEFINES += DINCLUDEPLUGS
    QTPLUGIN += qico \
        qgif \
        qjpeg
}
win32:TARGET = vlc-record
else {
    static:TARGET = release/vlc-record
    shared:TARGET = debug/vlc-record
}

# -------------------------------------
# customization ...
# - make a define here and put needed
# values into customization.h
# -------------------------------------
# DEFINES += _CUST_RUSS_TELEK
# translation stuff ...
include (language.pri)
