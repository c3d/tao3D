# ******************************************************************************
# tao.pro                                                           XLR project
# ******************************************************************************
# File Description:
# Main Qt build file for Tao
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
# (C) 2010 Catherine Burvelle <cathy@taodyne.com>
# (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************
TEMPLATE = app
TARGET = 
DEPENDPATH += . \
    ../xlr
INCLUDEPATH += . \
    ../xlr
QT += opengl \
    svg
CONFIG += warn_off

# CONFIG += release
# Tell the XLR portion that we are building for Tao
DEFINES += TAO \
    DEBUG
macx { 
    DEFINES += CONFIG_MACOSX
    XLRDIR = Contents/MacOS
}
win32 {
    DEFINES += CONFIG_MINGW
}
linux-g++ {
    DEFINES += CONFIG_LINUX
}

# Input
HEADERS += widget.h \
    window.h \
    frame.h \
    svg.h \
    texture.h \
    utf8.h \
    coords3d.h \
    gl_keepers.h \
    text_flow.h \
    ../xlr/base.h \
    ../xlr/options.h \
    ../xlr/basics.h \
    ../xlr/parser.h \
    ../xlr/compiler.h \
    ../xlr/renderer.h \
    ../xlr/configuration.h \
    ../xlr/runtime.h \
    ../xlr/context.h \
    ../xlr/scanner.h \
    ../xlr/errors.h \
    ../xlr/serializer.h \
    ../xlr/hash.h \
    ../xlr/sha1.h \
    ../xlr/main.h \
    ../xlr/sha1_ostream.h \
    ../xlr/opcodes.h \
    ../xlr/syntax.h \
    ../xlr/opcodes_declare.h \
    ../xlr/tree.h \
    ../xlr/opcodes_define.h \
    ../xlr/types.h
SOURCES += tao_main.cpp \
    graphics.cpp \
    widget.cpp \
    window.cpp \
    frame.cpp \
    svg.cpp \
    texture.cpp \
    text_flow.cpp \
    gl_keepers.cpp \
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
RESOURCES += tao.qrc

# LLVM dependencies
LLVM_LIBS = $$system(bash -c \"llvm-config --libs core jit native\")
LLVM_LIBS += $$system(bash -c \"llvm-config --ldflags\")
LLVM_INC = $$system(bash -c \"llvm-config --includedir\")
LLVM_DEF = $$system(bash -c \"llvm-config --cppflags | sed \'s/-I[^ ]*//g\' | sed s/-D//g\")
INCLUDEPATH *= $$LLVM_INC
DEFAULT_FONT = /Library/Fonts/Arial.ttf
LIBS += $$LLVM_LIBS
DEFINES += $$LLVM_DEF
OTHER_FILES += xl.syntax \
    xl.stylesheet \
    short.stylesheet \
    html.stylesheet \
    debug.stylesheet \
    dbghtml.stylesheet \
    bytecode.stylesheet \
    builtins.xl \
    graphics.tbl

# Copy the support files to the target directory
xlr_support.path = $${DESTDIR}/$${XLRDIR}
xlr_support.files += $${OTHER_FILES}
QMAKE_BUNDLE_DATA += xlr_support
