TARGET = compute_descriptors
include(../common.pri)

CONFIG += console

TEMPLATE = app

LIBS += -lboost_thread-mt \
        -lopencv_highgui \
        -lopencv_core \
        -lopencv_imgproc

SOURCES += main.cpp \
    imagefiles.cpp \
    descriptors/tinylab.cpp \
    descriptors/gist.cpp \
    descriptors/utilities.cpp

HEADERS += types.hpp \
    io.hpp \
    property.hpp \
    generator.hpp \
    cmdline.hpp \
    imagefiles.h \
    descriptors/tinylab.hpp \
    descriptors/gist.hpp \
    descriptors/utilities.hpp
