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

LIBXLR=.
include (../main.pri)

TEMPLATE = lib
TARGET   = xlr
CONFIG  += dll
QT      -= core gui
#VERSION  = "1.0.0"

INC = . \
    xlr/include \
    xlr/ \
    ..
INCLUDEPATH += $$INC
DEPENDPATH += $$INC

DEFINES += LIBXLR

QMAKE_CXXFLAGS += -w

HEADERS += \
     xlr/include/action.h \
     xlr/include/atomic.h \
     xlr/include/base.h \
     xlr/include/basics.h \
     xlr/include/bytecode.h \
     xlr/include/context.h \
     xlr/include/errors.h \
     xlr/include/evaluator.h \
     xlr/include/gc.h \
     xlr/include/info.h \
     xlr/include/interpreter.h \
     xlr/include/main.h \
     xlr/include/opcodes.h \
     xlr/include/options.h \
     xlr/include/parser.h \
     xlr/include/postorder.h \
     xlr/include/refcount.h \
     xlr/include/remote.h \
     xlr/include/renderer.h \
     xlr/include/runtime.h \
     xlr/include/save.h \
     xlr/include/scanner.h \
     xlr/include/serializer.h \
     xlr/include/syntax.h \
     xlr/include/time_functions.h \
     xlr/include/tree-clone.h \
     xlr/include/tree-walk.h \
     xlr/include/tree.h \
     xlr/include/types.h \
     xlr/include/utf8.h \
     xlr/include/utf8_fileutils.h \
     xlr/include/winglob.h \
     xlr/src/cdecls.h \
     xlr/src/compiler-args.h \
     xlr/src/compiler-expr.h \
     xlr/src/compiler-function.h \
     xlr/src/compiler-parms.h \
     xlr/src/compiler-prototype.h \
     xlr/src/compiler-unit.h \
     xlr/src/compiler.h \
     xlr/src/llvm-crap.h

SOURCES += \
    xlr/src/types.cpp \
    xlr/src/bytecode.cpp \
    xlr/src/compiler.cpp \
    xlr/src/interpreter.cpp \
    xlr/src/compiler-unit.cpp \
    xlr/src/context.cpp \
    xlr/src/compiler-prototype.cpp \
    xlr/src/options.cpp \
    xlr/src/tree.cpp \
    xlr/src/utf8_fileutils.cpp \
    xlr/src/scanner.cpp \
    xlr/src/compiler-function.cpp \
    xlr/src/llvm-crap.cpp \
    xlr/src/opcodes.cpp \
    xlr/src/compiler-expr.cpp \
    xlr/src/gc.cpp \
    xlr/src/compiler-args.cpp \
    xlr/src/cdecls.cpp \
    xlr/src/errors.cpp \
    xlr/src/runtime.cpp \
    xlr/src/syntax.cpp \
    xlr/src/serializer.cpp \
    xlr/src/action.cpp \
    xlr/src/parser.cpp \
    xlr/src/main.cpp \
    xlr/src/renderer.cpp \
    xlr/recorder/recorder.c \
    xlr/recorder/recorder_ring.c

win32 {
      SOURCES += xlr/src/winglob.cpp
      HEADERS += xlr/src/winglob.h
}

XL_MODULES += \
    xlr/include/basics.tbl \
    xlr/include/temperature.tbl \
    xlr/include/math.tbl \
    xlr/include/text.tbl \
    xlr/include/remote.tbl \
    xlr/include/time_functions.tbl \
    xlr/include/io.tbl

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
DEFINES += XL_VERSION=\\\"$$system(bash -c \"git describe --always --tags --dirty=-dirty\")\\\"
DEFINES += XL_BIN=\\\"\\\"
DEFINES += XL_LIB=\\\"\\\"
# Bug#1430
# LLVM_DEF adds NDEBUG when LLVM is a release build.
# We don't want this flag in our debug build.
CONFIG(debug, debug|release):DEFINES -= NDEBUG

target.path = $$LIBINST
INSTALLS    = target

# Compile with valgrind hooks by default
!build_pass:contains(DEFINES, NVALGRIND):message(Compiling libxlr without Valgrind hooks)

macx:QMAKE_LFLAGS_SONAME = -install_name$${LITERAL_WHITESPACE}@rpath/
