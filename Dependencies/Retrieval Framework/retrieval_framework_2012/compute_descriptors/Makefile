#############################################################################
# Makefile for building: release/compute_descriptors
# Generated by qmake (2.01a) (Qt 4.6.2) on: Fri Jan 6 00:23:24 2012
# Project:  compute_descriptors.pro
# Template: app
# Command: /usr/bin/qmake -unix -o Makefile compute_descriptors.pro
#############################################################################

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -DNDEBUG -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS        = -pipe -O2 -Wall -W -D_REENTRANT $(DEFINES)
CXXFLAGS      = -pipe -O2 -O3 -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I/usr/share/qt4/mkspecs/linux-g++ -I. -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I/usr/include/qt4 -I../shared -Irelease/moc -I/usr/share/qt4/mkspecs/linux-g++ -I. -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I/usr/include/qt4 -I /home/pari/download/OpenCV-2.3.1/modules/core/include -I /home/pari/download/boost_1_48_0 -I /home/pari/download/OpenCV-2.3.1/modules/highgui/include -I /home/pari/download/OpenCV-2.3.1/modules/imgproc/include
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = $(SUBLIBS) -L /home/pari/download/OpenCV-2.3.1/release/lib -L/usr/lib -l boost_thread-mt -lopencv_highgui -lopencv_core -lopencv_imgproc -lQtGui -lQtCore -lpthread 
AR            = ar cqs
RANLIB        = 
QMAKE         = /usr/bin/qmake
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = release/obj/

####### Files

SOURCES       = main.cpp \
		../shared/imagefiles.cpp \
		../shared/descriptors/tinylab.cpp \
		../shared/descriptors/gist.cpp \
		../shared/descriptors/utilities.cpp 
OBJECTS       = release/obj/main.o \
		release/obj/imagefiles.o \
		release/obj/tinylab.o \
		release/obj/gist.o \
		release/obj/utilities.o
DIST          = /usr/share/qt4/mkspecs/common/g++.conf \
		/usr/share/qt4/mkspecs/common/unix.conf \
		/usr/share/qt4/mkspecs/common/linux.conf \
		/usr/share/qt4/mkspecs/qconfig.pri \
		/usr/share/qt4/mkspecs/features/qt_functions.prf \
		/usr/share/qt4/mkspecs/features/qt_config.prf \
		/usr/share/qt4/mkspecs/features/exclusive_builds.prf \
		/usr/share/qt4/mkspecs/features/default_pre.prf \
		../common.pri \
		/usr/share/qt4/mkspecs/features/release.prf \
		/usr/share/qt4/mkspecs/features/default_post.prf \
		/usr/share/qt4/mkspecs/features/warn_on.prf \
		/usr/share/qt4/mkspecs/features/qt.prf \
		/usr/share/qt4/mkspecs/features/unix/thread.prf \
		/usr/share/qt4/mkspecs/features/moc.prf \
		/usr/share/qt4/mkspecs/features/resources.prf \
		/usr/share/qt4/mkspecs/features/uic.prf \
		/usr/share/qt4/mkspecs/features/yacc.prf \
		/usr/share/qt4/mkspecs/features/lex.prf \
		compute_descriptors.pro
QMAKE_TARGET  = compute_descriptors
DESTDIR       = release/
TARGET        = release/compute_descriptors

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	@$(CHK_DIR_EXISTS) release/ || $(MKDIR) release/ 
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

Makefile: compute_descriptors.pro  /usr/share/qt4/mkspecs/linux-g++/qmake.conf /usr/share/qt4/mkspecs/common/g++.conf \
		/usr/share/qt4/mkspecs/common/unix.conf \
		/usr/share/qt4/mkspecs/common/linux.conf \
		/usr/share/qt4/mkspecs/qconfig.pri \
		/usr/share/qt4/mkspecs/features/qt_functions.prf \
		/usr/share/qt4/mkspecs/features/qt_config.prf \
		/usr/share/qt4/mkspecs/features/exclusive_builds.prf \
		/usr/share/qt4/mkspecs/features/default_pre.prf \
		../common.pri \
		/usr/share/qt4/mkspecs/features/release.prf \
		/usr/share/qt4/mkspecs/features/default_post.prf \
		/usr/share/qt4/mkspecs/features/warn_on.prf \
		/usr/share/qt4/mkspecs/features/qt.prf \
		/usr/share/qt4/mkspecs/features/unix/thread.prf \
		/usr/share/qt4/mkspecs/features/moc.prf \
		/usr/share/qt4/mkspecs/features/resources.prf \
		/usr/share/qt4/mkspecs/features/uic.prf \
		/usr/share/qt4/mkspecs/features/yacc.prf \
		/usr/share/qt4/mkspecs/features/lex.prf \
		/usr/lib/libQtGui.prl \
		/usr/lib/libQtCore.prl
	$(QMAKE) -unix -o Makefile compute_descriptors.pro
/usr/share/qt4/mkspecs/common/g++.conf:
/usr/share/qt4/mkspecs/common/unix.conf:
/usr/share/qt4/mkspecs/common/linux.conf:
/usr/share/qt4/mkspecs/qconfig.pri:
/usr/share/qt4/mkspecs/features/qt_functions.prf:
/usr/share/qt4/mkspecs/features/qt_config.prf:
/usr/share/qt4/mkspecs/features/exclusive_builds.prf:
/usr/share/qt4/mkspecs/features/default_pre.prf:
../common.pri:
/usr/share/qt4/mkspecs/features/release.prf:
/usr/share/qt4/mkspecs/features/default_post.prf:
/usr/share/qt4/mkspecs/features/warn_on.prf:
/usr/share/qt4/mkspecs/features/qt.prf:
/usr/share/qt4/mkspecs/features/unix/thread.prf:
/usr/share/qt4/mkspecs/features/moc.prf:
/usr/share/qt4/mkspecs/features/resources.prf:
/usr/share/qt4/mkspecs/features/uic.prf:
/usr/share/qt4/mkspecs/features/yacc.prf:
/usr/share/qt4/mkspecs/features/lex.prf:
/usr/lib/libQtGui.prl:
/usr/lib/libQtCore.prl:
qmake:  FORCE
	@$(QMAKE) -unix -o Makefile compute_descriptors.pro

dist: 
	@$(CHK_DIR_EXISTS) release/obj/compute_descriptors1.0.0 || $(MKDIR) release/obj/compute_descriptors1.0.0 
	$(COPY_FILE) --parents $(SOURCES) $(DIST) release/obj/compute_descriptors1.0.0/ && $(COPY_FILE) --parents ../shared/types.hpp ../shared/io.hpp ../shared/property.hpp ../shared/generator.hpp ../shared/cmdline.hpp ../shared/imagefiles.h ../shared/descriptors/tinylab.hpp ../shared/descriptors/gist.hpp ../shared/descriptors/utilities.hpp release/obj/compute_descriptors1.0.0/ && $(COPY_FILE) --parents main.cpp ../shared/imagefiles.cpp ../shared/descriptors/tinylab.cpp ../shared/descriptors/gist.cpp ../shared/descriptors/utilities.cpp release/obj/compute_descriptors1.0.0/ && (cd `dirname release/obj/compute_descriptors1.0.0` && $(TAR) compute_descriptors1.0.0.tar compute_descriptors1.0.0 && $(COMPRESS) compute_descriptors1.0.0.tar) && $(MOVE) `dirname release/obj/compute_descriptors1.0.0`/compute_descriptors1.0.0.tar.gz . && $(DEL_FILE) -r release/obj/compute_descriptors1.0.0


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

compiler_moc_header_make_all:
compiler_moc_header_clean:
compiler_rcc_make_all:
compiler_rcc_clean:
compiler_image_collection_make_all: qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) qmake_image_collection.cpp
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_uic_make_all:
compiler_uic_clean:
compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: 

####### Compile

release/obj/main.o: main.cpp ../shared/types.hpp \
		../shared/property.hpp \
		../shared/io.hpp \
		../shared/generator.hpp \
		../shared/registry.hpp \
		../shared/cmdline.hpp \
		../shared/imagefiles.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o release/obj/main.o main.cpp

release/obj/imagefiles.o: ../shared/imagefiles.cpp ../shared/imagefiles.h \
		../shared/property.hpp \
		../shared/types.hpp \
		../shared/io.hpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o release/obj/imagefiles.o ../shared/imagefiles.cpp

release/obj/tinylab.o: ../shared/descriptors/tinylab.cpp ../shared/types.hpp \
		../shared/descriptors/tinylab.hpp \
		../shared/generator.hpp \
		../shared/property.hpp \
		../shared/io.hpp \
		../shared/registry.hpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o release/obj/tinylab.o ../shared/descriptors/tinylab.cpp

release/obj/gist.o: ../shared/descriptors/gist.cpp ../shared/descriptors/gist.hpp \
		../shared/types.hpp \
		../shared/generator.hpp \
		../shared/property.hpp \
		../shared/io.hpp \
		../shared/registry.hpp \
		../shared/descriptors/gist_helper.hpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o release/obj/gist.o ../shared/descriptors/gist.cpp

release/obj/utilities.o: ../shared/descriptors/utilities.cpp ../shared/descriptors/utilities.hpp \
		../shared/types.hpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o release/obj/utilities.o ../shared/descriptors/utilities.cpp

####### Install

install:   FORCE

uninstall:   FORCE

FORCE:

