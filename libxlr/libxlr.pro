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

INCLUDEPATH += .

DEFINES += LIBXLR

HEADERS = \
    ../tao/xlr/xlr/utf8.h \
    ../tao/xlr/xlr/base.h \
    ../tao/xlr/xlr/options.h \
    ../tao/xlr/xlr/basics.h \
    ../tao/xlr/xlr/parser.h \
    ../tao/xlr/xlr/compiler.h \
    ../tao/xlr/xlr/renderer.h \
    ../tao/xlr/xlr/configuration.h \
    ../tao/xlr/xlr/runtime.h \
    ../tao/xlr/xlr/context.h \
    ../tao/xlr/xlr/scanner.h \
    ../tao/xlr/xlr/errors.h \
    ../tao/xlr/xlr/serializer.h \
    ../tao/xlr/xlr/hash.h \
    ../tao/xlr/xlr/sha1.h \
    ../tao/xlr/xlr/main.h \
    ../tao/xlr/xlr/sha1_ostream.h \
    ../tao/xlr/xlr/opcodes.h \
    ../tao/xlr/xlr/syntax.h \
    ../tao/xlr/xlr/opcodes_declare.h \
    ../tao/xlr/xlr/tree.h \
    ../tao/xlr/xlr/gc.h \
    ../tao/xlr/xlr/opcodes_define.h \
    ../tao/xlr/xlr/types.h \
    ../tao/xlr/xlr/diff.h \
    ../tao/xlr/xlr/lcs.h \
    ../tao/xlr/xlr/bfs.h

SOURCES = \
    ../tao/xlr/xlr/context.cpp \
    ../tao/xlr/xlr/compiler.cpp \
    ../tao/xlr/xlr/diff.cpp \
    ../tao/xlr/xlr/errors.cpp \
    ../tao/xlr/xlr/gc.cpp \
    ../tao/xlr/xlr/lcs.cpp \
    ../tao/xlr/xlr/main.cpp \
    ../tao/xlr/xlr/options.cpp \
    ../tao/xlr/xlr/opcodes.cpp \
    ../tao/xlr/xlr/parser.cpp \
    ../tao/xlr/xlr/renderer.cpp \
    ../tao/xlr/xlr/runtime.cpp \
    ../tao/xlr/xlr/scanner.cpp \
    ../tao/xlr/xlr/serializer.cpp \
    ../tao/xlr/xlr/sha1.cpp \
    ../tao/xlr/xlr/syntax.cpp \
    ../tao/xlr/xlr/tree.cpp \
    ../tao/xlr/xlr/types.cpp

CXXTBL_SOURCES += \
    ../tao/xlr/xlr/basics.cpp

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
