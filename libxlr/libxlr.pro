# ******************************************************************************
#  libxlr.pro                                                       Tao project
# ******************************************************************************
# File Description:
# Qt build file for the XL Runtime library (libxlr)
# ******************************************************************************
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# *****************************************************************************
# This software is licensed under the GNU General Public License v3
# See file COPYING for details.
# *****************************************************************************


include (../main.pri)

TEMPLATE = lib
TARGET   = xlr
CONFIG  += dll
QT      -= core gui
#VERSION  = "1.0.0"

INC = . \
    ../tao/xlr/xlr/include \
    ../tao/xlr/xlr
INCLUDEPATH += $$INC
DEPENDPATH += $$INC

DEFINES += LIBXLR

HEADERS += \
    ../tao/xlr/xlr/compiler.h \
    ../tao/xlr/xlr/diff.h \
    ../tao/xlr/xlr/lcs.h \
    ../tao/xlr/xlr/include/action.h \
    ../tao/xlr/xlr/include/base.h \
    ../tao/xlr/xlr/include/flight_recorder.h \
    ../tao/xlr/xlr/include/basics.h \
    ../tao/xlr/xlr/include/bfs.h \
    ../tao/xlr/xlr/include/configuration.h \
    ../tao/xlr/xlr/include/context.h \
    ../tao/xlr/xlr/include/errors.h \
    ../tao/xlr/xlr/include/gc.h \
    ../tao/xlr/xlr/include/hash.h \
    ../tao/xlr/xlr/include/info.h \
    ../tao/xlr/xlr/include/main.h \
    ../tao/xlr/xlr/include/opcodes.h \
    ../tao/xlr/xlr/include/opcodes_declare.h \
    ../tao/xlr/xlr/include/opcodes_define.h \
    ../tao/xlr/xlr/include/opcodes_delete.h \
    ../tao/xlr/xlr/include/options.h \
    ../tao/xlr/xlr/include/parser.h \
    ../tao/xlr/xlr/include/renderer.h \
    ../tao/xlr/xlr/include/runtime.h \
    ../tao/xlr/xlr/include/scanner.h \
    ../tao/xlr/xlr/include/serializer.h \
    ../tao/xlr/xlr/include/sha1.h \
    ../tao/xlr/xlr/include/sha1_ostream.h \
    ../tao/xlr/xlr/include/syntax.h \
    ../tao/xlr/xlr/include/traces.h \
    ../tao/xlr/xlr/include/traces_base.h \
    ../tao/xlr/xlr/include/tree.h \
    ../tao/xlr/xlr/include/types.h \
    ../tao/xlr/xlr/include/utf8.h \
    ../tao/xlr/xlr/utf8_fileutils.h

SOURCES += \
    ../tao/xlr/xlr/action.cpp \
    ../tao/xlr/xlr/args.cpp \
    ../tao/xlr/xlr/cdecls.cpp \
    ../tao/xlr/xlr/compiler.cpp \
    ../tao/xlr/xlr/compiler-gc.cpp \
    ../tao/xlr/xlr/compiler-llvm.cpp \
    ../tao/xlr/xlr/context.cpp \
    ../tao/xlr/xlr/diff.cpp \
    ../tao/xlr/xlr/errors.cpp \
    ../tao/xlr/xlr/expred.cpp \
    ../tao/xlr/xlr/flight_recorder.cpp \
    ../tao/xlr/xlr/gc.cpp \
    ../tao/xlr/xlr/hash.cpp \
    ../tao/xlr/xlr/lcs.cpp \
    ../tao/xlr/xlr/main.cpp \
    ../tao/xlr/xlr/opcodes.cpp \
    ../tao/xlr/xlr/options.cpp \
    ../tao/xlr/xlr/parms.cpp \
    ../tao/xlr/xlr/parser.cpp \
    ../tao/xlr/xlr/renderer.cpp \
    ../tao/xlr/xlr/runtime.cpp \
    ../tao/xlr/xlr/scanner.cpp \
    ../tao/xlr/xlr/serializer.cpp \
    ../tao/xlr/xlr/sha1.cpp \
    ../tao/xlr/xlr/symbols.cpp \
    ../tao/xlr/xlr/syntax.cpp \
    ../tao/xlr/xlr/traces_base.cpp \
    ../tao/xlr/xlr/tree.cpp \
    ../tao/xlr/xlr/types.cpp \
    ../tao/xlr/xlr/unit.cpp

win32 {
      SOURCES += ../tao/xlr/xlr/winglob.cpp
      HEADERS += ../tao/xlr/xlr/winglob.h
}


CXXTBL_SOURCES += \
    ../tao/xlr/xlr/basics.cpp

OTHER_FILES = \
    ../tao/xlr/xlr/options.tbl \
    ../tao/xlr/xlr/traces.tbl \
    ../tao/xlr/xlr/include/basics.tbl

linux* {
  # Prevent linker from exporting symbols from the LLVM static libraries
  LIBS += -Wl,--exclude-libs,ALL
}

# We need bash, llvm-config[-2.9]
!system(bash -c \"bash --version >/dev/null\"):error("Can't execute bash")
system(bash -c \"llvm-config-2.9 --version >/dev/null 2>&1\") {
  !build_pass:message(Found llvm-config-2.9)
  LLVMCONFIG=llvm-config-2.9
} else {
  system(bash -c \"llvm-config --version >/dev/null 2>&1\") {
    !build_pass:message(Found llvm-config)
    LLVMCONFIG=llvm-config
  } else {
    error("Can't execute llvm-config-2.9 nor llvm-config")
  }
}
# LLVM dependencies
LLVM_VERSION = $$system(bash -c \"$$LLVMCONFIG --version | sed -e 's/[.a-z-]//g' \")
LLVM_LIBS = $$system(bash -c \"$$LLVMCONFIG --libs\")
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
