#******************************************************************************
# tao.pro                                                           XLR project
#******************************************************************************
#
#  File Description:
#
#    Main Qt build file for Tao
#
#
#
#
#
#
#
#
#******************************************************************************
#This document is released under the GNU General Public License.
#See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
# (C) 2010 Catherine Burvelle <cathy@taodyne.com>
# (C) 2010 Taodyne SAS
#******************************************************************************
#* File       : $RCSFile$
#* Revision   : $Revision$
#* Date       : $Date$
#******************************************************************************

TEMPLATE = app
TARGET = 
DEPENDPATH += . ../xlr
INCLUDEPATH += . ../xlr
QT += opengl svg
CONFIG += warn_off

# Tell the XLR portion that we are building for Tao
DEFINES += TAO

macx {
    DEFINES += CONFIG_MACOSX
    XLRDIR = Contents/MacOS
}
win32 {
    DEFINES += CONFIG_WIN32
}
linux {
    DEFINES += CONFIG_LINUX
}

# Input
HEADERS += tao_widget.h tao_window.h                    \
../xlr/base.h			../xlr/options.h        \
../xlr/basics.h			../xlr/parser.h         \
../xlr/compiler.h		../xlr/renderer.h       \
../xlr/configuration.h		../xlr/runtime.h        \
../xlr/context.h		../xlr/scanner.h        \
../xlr/errors.h			../xlr/serializer.h     \
../xlr/hash.h			../xlr/sha1.h           \
../xlr/main.h			../xlr/sha1_ostream.h   \
../xlr/opcodes.h		../xlr/syntax.h         \
../xlr/opcodes_declare.h	../xlr/tree.h           \
../xlr/opcodes_define.h		../xlr/types.h

SOURCES += \
    tao_main.cpp \
    graphics.cpp \
    tao_widget.cpp \
    tao_window.cpp \
    ../xlr/tree.cpp \
    ../xlr/sha1.cpp \
    ../xlr/serializer.cpp \
    ../xlr/syntax.cpp \
    ../xlr/scanner.cpp \
    ../xlr/runtime.cpp \
    ../xlr/renderer.cpp \
    ../xlr/parser.cpp \
    ../xlr/options.cpp \
    ../xlr/opcodes.cpp \
    ../xlr/main.cpp \
    ../xlr/errors.cpp \
    ../xlr/context.cpp \
    ../xlr/compiler.cpp \
    ../xlr/basics.cpp

RESOURCES += framebufferobject.qrc

# LLVM dependencies
exists(/usr/local/bin/llvm-config) {
    LLVM_PATH = /usr/local/bin
}
exists(/opt/local/bin/llvm-config) {
    LLVM_PATH = /opt/local/bin
}
exists(/usr/bin/llvm-config) {
    LLVM_PATH = /usr/bin
}

LLVM_FLAGS = $$system($$LLVM_PATH/llvm-config --cppflags | sed -e s/-DNDEBUG//g)
LLVM_LIBS = $$system($$LLVM_PATH/llvm-config --ldflags --libs core jit native)
LLVM_INC = $$system($$LLVM_PATH/llvm-config --includedir)
LLVM_DEF = $$system($$LLVM_PATH/llvm-config --cppflags | grep -o .D_.* | sed s/-D//g)

INCLUDEPATH *= $$LLVM_INC

DEFAULT_FONT = /Library/Fonts/Arial.ttf
LIBS += $$LLVM_LIBS
DEFINES += $$LLVM_DEF

OTHER_FILES += ../xlr/xl.syntax \
    ../xlr/xl.stylesheet \
    ../xlr/short.stylesheet \
    ../xlr/html.stylesheet \
    ../xlr/debug.stylesheet \
    ../xlr/dbghtml.stylesheet \
    ../xlr/bytecode.stylesheet \
    ../xlr/builtins.xl

# Copy the support files to the target directory
xlr_support.path = $${DESTDIR}/$${XLRDIR}
xlr_support.files += $${OTHER_FILES}
QMAKE_BUNDLE_DATA += xlr_support
