# ******************************************************************************
#  main.pri                                                         Tao project
# ******************************************************************************
# File Description:
# Global project definitions for Tao
# ******************************************************************************
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# *****************************************************************************
# This software is licensed under the GNU General Public License v3
# See file COPYING for details.
# *****************************************************************************


include(main_defs.pri)

CONFIG             += rtti exceptions
DEFINES            += DEBUG
# Make assert() a no-op in release mode
CONFIG(release, debug|release):DEFINES *= NDEBUG
macx:DEFINES       += CONFIG_MACOSX
win32:DEFINES      += CONFIG_MINGW
linux-g++*:DEFINES += CONFIG_LINUX

QMAKE_CFLAGS           += -Werror
QMAKE_CXXFLAGS         += -Werror
QMAKE_CXXFLAGS_DEBUG   += -ggdb
# Mac: this will store full debug info in .o files.
# No impact on exe size but allows full debug of release build.
macx:QMAKE_CXXFLAGS_RELEASE += -ggdb
# To obtain meaningful stack traces in GDB, don't strip binaries
linux-g++*:QMAKE_STRIP = :

# Avoid linker warnings with Qt >= 4.7.4
# (Qt commit 0c4ed66e87ef6f76d5b0d67905b587c31ad03a18)
win32-g++:QMAKE_LFLAGS *= -Wl,-enable-auto-import

# Adding 'cxxtbl' option with lowered optimization level
cxxtbl.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_BASE}.o
cxxtbl.commands = $$QMAKE_CXX \
    -c \
    $(CXXFLAGS:-O2=) \
    $(INCPATH) \
    ${QMAKE_FILE_IN} \
    -o \
    ${QMAKE_FILE_OUT}
silent:cxxtbl.commands = @echo compiling "${QMAKE_FILE_IN}" && $$cxxtbl.commands
cxxtbl.dependency_type = TYPE_C
cxxtbl.input = CXXTBL_SOURCES
QMAKE_EXTRA_COMPILERS += cxxtbl

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
# "make clean" on windows cleans both debug and release files. Since we build
# debug OR release but not both, this results in ugly (and ignored) error
# messages. Avoid them.
win32:QMAKE_DEL_FILE = rm -f

QMAKE_EXTRA_TARGETS += doc
