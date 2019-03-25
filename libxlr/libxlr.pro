# ******************************************************************************
# libxlr.pro                                                       Tao3D project
# ******************************************************************************
#
# File description:
# Qt build file for the XL Runtime library (libxlr)
#
#
#
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2011, Catherine Burvelle <catherine@taodyne.com>
# (C) 2010-2011,2014,2017,2019, Christophe de Dinechin <christophe@dinechin.org>
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


include (../main.pri)

TEMPLATE = lib
TARGET   = xlr
CONFIG  += dll
QT      -= core gui
#VERSION  = "1.0.0"

INC = . \
    xlr/xlr/include \
    xlr/            \
    xlr/xlr
INCLUDEPATH += $$INC
DEPENDPATH += $$INC

DEFINES += LIBXLR

HEADERS += \
    xlr/xlr/compiler.h \
    xlr/xlr/diff.h \
    xlr/xlr/lcs.h \
    xlr/xlr/include/action.h \
    xlr/xlr/include/base.h \
    xlr/xlr/include/basics.h \
    xlr/xlr/include/bfs.h \
    xlr/xlr/include/configuration.h \
    xlr/xlr/include/context.h \
    xlr/xlr/include/errors.h \
    xlr/xlr/include/gc.h \
    xlr/xlr/include/hash.h \
    xlr/xlr/include/info.h \
    xlr/xlr/include/main.h \
    xlr/xlr/include/opcodes.h \
    xlr/xlr/include/opcodes_declare.h \
    xlr/xlr/include/opcodes_define.h \
    xlr/xlr/include/opcodes_delete.h \
    xlr/xlr/include/options.h \
    xlr/xlr/include/parser.h \
    xlr/xlr/include/renderer.h \
    xlr/xlr/include/runtime.h \
    xlr/xlr/include/scanner.h \
    xlr/xlr/include/serializer.h \
    xlr/xlr/include/sha1.h \
    xlr/xlr/include/sha1_ostream.h \
    xlr/xlr/include/syntax.h \
    xlr/xlr/include/traces.h \
    xlr/xlr/include/traces_base.h \
    xlr/xlr/include/tree.h \
    xlr/xlr/include/types.h \
    xlr/xlr/include/utf8.h \
    xlr/xlr/utf8_fileutils.h

SOURCES += \
    xlr/xlr/action.cpp \
    xlr/xlr/args.cpp \
    xlr/xlr/cdecls.cpp \
    xlr/xlr/compiler.cpp \
    xlr/xlr/compiler-gc.cpp \
    xlr/xlr/compiler-llvm.cpp \
    xlr/xlr/context.cpp \
    xlr/xlr/diff.cpp \
    xlr/xlr/errors.cpp \
    xlr/xlr/expred.cpp \
    xlr/xlr/gc.cpp \
    xlr/xlr/hash.cpp \
    xlr/xlr/lcs.cpp \
    xlr/xlr/main.cpp \
    xlr/xlr/opcodes.cpp \
    xlr/xlr/options.cpp \
    xlr/xlr/parms.cpp \
    xlr/xlr/parser.cpp \
    xlr/xlr/renderer.cpp \
    xlr/xlr/runtime.cpp \
    xlr/xlr/scanner.cpp \
    xlr/xlr/serializer.cpp \
    xlr/xlr/sha1.cpp \
    xlr/xlr/symbols.cpp \
    xlr/xlr/syntax.cpp \
    xlr/xlr/traces_base.cpp \
    xlr/xlr/tree.cpp \
    xlr/xlr/types.cpp \
    xlr/xlr/unit.cpp \
    xlr/recorder/recorder.c \
    xlr/recorder/recorder_ring.c

win32 {
      SOURCES += xlr/xlr/winglob.cpp
      HEADERS += xlr/xlr/winglob.h
}


CXXTBL_SOURCES += \
    xlr/xlr/basics.cpp

OTHER_FILES = \
    xlr/xlr/options.tbl \
    xlr/xlr/traces.tbl \
    xlr/xlr/include/basics.tbl

# We need bash, llvm-config
!system(bash -c \"bash --version >/dev/null\"):error("Can't execute bash")
system(bash -c \"./find-llvm-config >/dev/null 2>&1\") {
  LLVMCONFIG=$$system(bash -c \"./find-llvm-config $$LLVMCONFIG\")
  !build_pass:message(Found $$LLVMCONFIG)
} else {
  error("Can't find a suitable llvm-config (not by lack of trying)")
}
# LLVM dependencies
LLVM_VERSION = $$system(bash -c \"$$LLVMCONFIG --version | sed -e 's/[.a-z-]//g' \")
LLVM_LIBS = $$system(bash -c \"$$LLVMCONFIG --libs\")
# --system-libs is required after 3.5, but not there before 3.4
# So LLVM not only makes source code incompatible, they managed to make
# incompatible configuration command lines as well!
LLVM_LIBS += $$system(bash -c \"$$LLVMCONFIG --system-libs 2> /dev/null || true\")
LLVM_LIBS += $$system(bash -c \"$$LLVMCONFIG --ldflags\")
LLVM_LIBS += -lncurses        # Don't ask (after 3.50)
LLVM_INC = $$system(bash -c \"$$LLVMCONFIG --includedir\")
LLVM_DEF = $$system(bash -c \"$$LLVMCONFIG --cppflags | sed \'s/-I[^ ]*//g\' | sed s/-D//g\") LLVM_VERSION=$$LLVM_VERSION

!build_ass::message(Found LLVM version $$LLVM_VERSION)

INCLUDEPATH += $$LLVM_INC
LIBS += $$LLVM_LIBS
DEFINES += $$LLVM_DEF
# Bug#1430
# LLVM_DEF adds NDEBUG when LLVM is a release build.
# We don't want this flag in our debug build.
CONFIG(debug, debug|release):DEFINES -= NDEBUG

target.path = $$LIBINST
INSTALLS    = target

# Compile with valgrind hooks by default
!build_pass:contains(DEFINES, NVALGRIND):message(Compiling libxlr without Valgrind hooks)

macx:QMAKE_LFLAGS_SONAME = -install_name$${LITERAL_WHITESPACE}@rpath/
