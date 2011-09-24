# ******************************************************************************
#  main.pri                                                         Tao project
# ******************************************************************************
# File Description:
# Global project definitions for Tao
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************


include(main_defs.pri)

DEFINES            += DEBUG
macx:DEFINES       += CONFIG_MACOSX
win32:DEFINES      += CONFIG_MINGW
linux-g++*:DEFINES += CONFIG_LINUX

QMAKE_CFLAGS           += -Werror
QMAKE_CXXFLAGS         += -Werror
QMAKE_CXXFLAGS_DEBUG   += -ggdb
!win32:QMAKE_CXXFLAGS_RELEASE += -ggdb

# Don't strip symbols on Windows to help crash analysis
win32:QMAKE_LFLAGS_RELEASE -= -Wl,-s

# Avoid linker warnings with Qt >= 4.7.4
# (Qt commit 0c4ed66e87ef6f76d5b0d67905b587c31ad03a18)
win32-g++:QMAKE_LFLAGS *= -Wl,-enable-auto-import

# Adding 'c++tbl' option with lowered optimization level
c++tbl.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_BASE}.o
c++tbl.commands = $(CXX) \
    -c \
    $(CXXFLAGS:-O2=) \
    $(INCPATH) \
    ${QMAKE_FILE_NAME} \
    -o \
    ${QMAKE_FILE_OUT}
c++tbl.dependency_type = TYPE_C
c++tbl.input = CXXTBL_SOURCES
QMAKE_EXTRA_COMPILERS += c++tbl

# Build some files with warnings disabled
c++nowarn.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_BASE}.o
c++nowarn.commands = $(CXX) \
    -c \
    $(CXXFLAGS) -w \
    $(INCPATH) \
    ${QMAKE_FILE_NAME} \
    -o \
    ${QMAKE_FILE_OUT}
c++nowarn.dependency_type = TYPE_C
c++nowarn.input = NOWARN_SOURCES
QMAKE_EXTRA_COMPILERS += c++nowarn

# No -p by default on Windows-mingw. Our make install needs -p.
win32:QMAKE_MKDIR = mkdir -p

QMAKE_EXTRA_TARGETS += doc
