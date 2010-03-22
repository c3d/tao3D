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
QT += webkit \
    network \
    opengl \
    svg

# CONFIG += release
CONFIG += debug \
    console

# Tell the XLR portion that we are building for Tao
DEFINES += TAO \
    DEBUG

macx {
    DEFINES += CONFIG_MACOSX
    XLRDIR = Contents/MacOS
    ICON = tao.icns
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
    application.h \
    frame.h \
    svg.h \
    texture.h \
    coords.h \
    coords3d.h \
    gl_keepers.h \
    text_flow.h \
    drawing.h \
    shapes_drawing.h \
    apply-changes.h \
    activity.h \
    selection.h \
    shapename.h \
    treeholder.h \
    menuinfo.h \
    widget-surface.h \
    process.h \
    repository.h \
    git_backend.h \
    ../xlr/utf8.h \
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
    ../xlr/types.h \
    ../xlr/diff.h \
    ../xlr/lcs.h \
    ../xlr/bfs.h \
    drag.h
SOURCES += tao_main.cpp \
    coords.cpp \
    coords3d.cpp \
    graphics.cpp \
    widget.cpp \
    window.cpp \
    frame.cpp \
    svg.cpp \
    widget-surface.cpp \
    texture.cpp \
    text_flow.cpp \
    drawing.cpp \
    shapes_drawing.cpp \
    apply-changes.cpp \
    activity.cpp \
    selection.cpp \
    shapename.cpp \
    gl_keepers.cpp \
    treeholder.cpp \
    menuinfo.cpp \
    process.cpp \
    repository.cpp \
    git_backend.cpp \
    application.cpp \
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
    ../xlr/basics.cpp \
    ../xlr/diff.cpp \
    ../xlr/lcs.cpp \
    drag.cpp
!win32 { 
    HEADERS += GL/glew.h \
        GL/glxew.h \
        GL/wglew.h
    SOURCES += glew.c
}
RESOURCES += tao.qrc

# We need bash, llvm-config and pkg-config
!system(bash -c \"bash --version >/dev/null\"):error("Can't execute bash")
!system(bash -c \"llvm-config --version >/dev/null\"):error("Can't execute llvm-config")
!system(bash -c \"pkg-config --version >/dev/null\"):error("Can't execute pkg-config")

# Pango / Cairo
PANGOCAIRO_LIBS = $$system(bash -c \"pkg-config --libs pangocairo \")
PANGOCAIRO_INC = $$system(bash -c \"pkg-config --cflags-only-I pangocairo | sed s/-I//g\")
PANGOCAIRO_FLAGS = $$system(bash -c \"pkg-config --cflags-only-other pangocairo\")

# LLVM dependencies
LLVM_LIBS = $$system(bash -c \"llvm-config --libs core jit native\")
LLVM_LIBS += $$system(bash -c \"llvm-config --ldflags\")
LLVM_INC = $$system(bash -c \"llvm-config --includedir\")
LLVM_DEF = $$system(bash -c \"llvm-config --cppflags | sed \'s/-I[^ ]*//g\' | sed s/-D//g\")

# Consolidated flags and libs
INCLUDEPATH += $$PANGOCAIRO_INC \
    $$LLVM_INC
LIBS += $$PANGOCAIRO_LIBS \
    $$LLVM_LIBS
DEFINES += $$LLVM_DEF

# Extra flags for CC and CXX
QMAKE_CFLAGS_RELEASE += $$PANGOCAIRO_FLAGS
QMAKE_CFLAGS_DEBUG += $$PANGOCAIRO_FLAGS
QMAKE_CXXFLAGS_RELEASE += $$PANGOCAIRO_FLAGS
QMAKE_CXXFLAGS_DEBUG += $$PANGOCAIRO_FLAGS

# Others
DEFAULT_FONT = /Library/Fonts/Arial.ttf
OTHER_FILES += xl.syntax \
    xl.stylesheet \
    short.stylesheet \
    html.stylesheet \
    debug.stylesheet \
    dbghtml.stylesheet \
    bytecode.stylesheet \
    builtins.xl \
    graphics.tbl \
    git.stylesheet

# Copy the support files to the target directory
xlr_support.path = $${DESTDIR}/$${XLRDIR}
xlr_support.files += $${OTHER_FILES}
QMAKE_BUNDLE_DATA += xlr_support
FORMS += 
