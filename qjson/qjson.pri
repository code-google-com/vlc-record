SOURCES += qjson/json_scanner.cpp \
           qjson/parser.cpp \
           qjson/json_parser.cc

           # code we don't need right now ...
           # qjson/serializerrunnable.cpp \
           # qjson/parserrunnable.cpp \
           # qjson/qobjecthelper.cpp \
           # qjson/serializer.cpp \

HEADERS += qjson/FlexLexer.h \
           qjson/json_scanner.h \
           qjson/parser.h \
           qjson/parser_p.h \
           qjson/qjson_debug.h \
           qjson/qjson_export.h

           # code we don't need right now ...
           # qjson/serializerrunnable.h \
           # qjson/parserrunnable.h \
           # qjson/qobjecthelper.h \
           # qjson/serializer.h \

INCLUDEPATH += qjson
