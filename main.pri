# ******************************************************************************
# main.pri                                                         Tao3D project
# ******************************************************************************
#
# File description:
# Global project definitions for Tao
#
#
#
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2010, Catherine Burvelle <catherine@taodyne.com>
# (C) 2010-2011,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
# ******************************************************************************
# This file is part of Tao3D
#
# Tao3D is free software: you can r redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Tao3D is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Tao3D, in a file named COPYING.
# If not, see <https://www.gnu.org/licenses/>.
# ******************************************************************************


include(main_defs.pri)

CONFIG             += rtti exceptions
DEFINES            += DEBUG=1
# Make assert() a no-op in release mode
CONFIG(release, debug|release):DEFINES *= NDEBUG
macx:DEFINES       += CONFIG_MACOSX
win32:DEFINES      += CONFIG_MINGW
linux-g++*:DEFINES += CONFIG_LINUX

QMAKE_CFLAGS           += -Wall
QMAKE_CXXFLAGS         += -Wall
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
