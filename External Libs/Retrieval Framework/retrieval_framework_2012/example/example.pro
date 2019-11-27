TARGET = example
TEMPLATE = app
include(../common.pri)

CONFIG += console

SOURCES += main.cpp imagefiles.cpp

HEADERS += types.hpp \
    io.hpp \
    property.hpp \
    imagefiles.h
