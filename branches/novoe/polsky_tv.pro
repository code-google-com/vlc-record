DEFINES += _TASTE_POLSKY_TV
RESOURCES += polsky_tv.qrc
HEADERS += tastes/defines_polsky_tv.h
RC_FILE = polsky_tv.rc
TRANSLATIONS = lang_de.ts \
               lang_pl.ts

# stuff for QJSON ...
DEFINES     += _USE_QJSON

# include common project (must be last) ...
include (common.pri)