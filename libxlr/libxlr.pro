# ******************************************************************************
#  libxlr.pro                                                       Tao project
# ******************************************************************************
# File Description:
# Qt build file for the XL Runtime library (libxlr)
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************


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

HEADERS = \
    ../tao/xlr/xlr/compiler.h \
    ../tao/xlr/xlr/diff.h \
    ../tao/xlr/xlr/lcs.h \
    ../tao/xlr/xlr/include/action.h \
    ../tao/xlr/xlr/include/base.h \
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
    ../tao/xlr/xlr/include/utf8.h

SOURCES = \
    ../tao/xlr/xlr/compiler.cpp \
    ../tao/xlr/xlr/context.cpp \
    ../tao/xlr/xlr/diff.cpp \
    ../tao/xlr/xlr/errors.cpp \
    ../tao/xlr/xlr/gc.cpp \
    ../tao/xlr/xlr/lcs.cpp \
    ../tao/xlr/xlr/main.cpp \
    ../tao/xlr/xlr/opcodes.cpp \
    ../tao/xlr/xlr/options.cpp \
    ../tao/xlr/xlr/parser.cpp \
    ../tao/xlr/xlr/renderer.cpp \
    ../tao/xlr/xlr/runtime.cpp \
    ../tao/xlr/xlr/scanner.cpp \
    ../tao/xlr/xlr/serializer.cpp \
    ../tao/xlr/xlr/sha1.cpp \
    ../tao/xlr/xlr/syntax.cpp \
    ../tao/xlr/xlr/traces_base.cpp \
    ../tao/xlr/xlr/tree.cpp \
    ../tao/xlr/xlr/types.cpp

CXXTBL_SOURCES += \
    ../tao/xlr/xlr/basics.cpp

OTHER_FILES = \
    ../tao/xlr/xlr/options.tbl \
    ../tao/xlr/xlr/traces.tbl \
    ../tao/xlr/xlr/include/basics.tbl

# We need bash, llvm-config
!system(bash -c \"bash --version >/dev/null\"):error("Can't execute bash")
!system(bash -c \"llvm-config --version >/dev/null\"):error("Can't execute llvm-config")

# LLVM dependencies
LLVM_LIBS = $$system(bash -c \"llvm-config --libs core jit native\")
LLVM_LIBS += $$system(bash -c \"llvm-config --ldflags\")
LLVM_INC = $$system(bash -c \"llvm-config --includedir\")
LLVM_DEF = $$system(bash -c \"llvm-config --cppflags | sed \'s/-I[^ ]*//g\' | sed s/-D//g\")

INCLUDEPATH += $$LLVM_INC
LIBS += $$LLVM_LIBS
DEFINES += $$LLVM_DEF

target.path = $$LIBINST
INSTALLS    = target
