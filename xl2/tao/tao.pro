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
TARGET = Tao
VERSION = "0.0.3"
DEPENDPATH += . \
    ../xlr
INCLUDEPATH += . \
    ../xlr
QT += webkit \
    network \
    opengl \
    svg \
    phonon
QMAKE_CXXFLAGS_RELEASE += -g

# Tell the XLR portion that we are building for Tao
DEFINES += TAO \
    DEBUG
macx { 
    DEFINES += CONFIG_MACOSX
    XLRDIR = Contents/MacOS
    ICON = tao.icns
    QMAKE_INFO_PLIST = Info.plist
}
win32:DEFINES += CONFIG_MINGW

# For debug
# win32:CONFIG += console
linux-g++:DEFINES += CONFIG_LINUX

# Input
HEADERS += widget.h \
    gl2ps.h \
    window.h \
    application.h \
    frame.h \
    svg.h \
    texture.h \
    coords.h \
    coords3d.h \
    color.h \
    gl_keepers.h \
    drawing.h \
    shapes.h \
    text_drawing.h \
    shapes3d.h \
    path3d.h \
    objloader.h \
    table.h \
    binpack.h \
    glyph_cache.h \
    attributes.h \
    transforms.h \
    layout.h \
    page_layout.h \
    space_layout.h \
    apply_changes.h \
    normalize.h \
    activity.h \
    selection.h \
    manipulator.h \
    menuinfo.h \
    widget_surface.h \
    process.h \
    repository.h \
    git_backend.h \
    tao_utf8.h \
    tao_tree.h \
    font.h \
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
    ../xlr/gc.h \
    ../xlr/opcodes_define.h \
    ../xlr/types.h \
    ../xlr/diff.h \
    ../xlr/lcs.h \
    ../xlr/bfs.h \
    drag.h \
    pull_from_dialog.h \
    remote_selection_frame.h \
    publish_to_dialog.h \
    undo.h \
    clone_dialog.h \
    ansi_textedit.h \
    ../xlr/opcodes_delete.h \
    error_message_dialog.h \
    group_layout.h \
    resource_mgt.h \
    tree_cloning.h \
    font_file_manager.h
SOURCES += tao_main.cpp \
    gl2ps.c \
    coords.cpp \
    coords3d.cpp \
    graphics.cpp \
    widget.cpp \
    window.cpp \
    formulas.cpp \
    frame.cpp \
    svg.cpp \
    widget_surface.cpp \
    texture.cpp \
    drawing.cpp \
    shapes.cpp \
    text_drawing.cpp \
    shapes3d.cpp \
    path3d.cpp \
    objloader.cpp \
    table.cpp \
    binpack.cpp \
    glyph_cache.cpp \
    attributes.cpp \
    transforms.cpp \
    layout.cpp \
    page_layout.cpp \
    space_layout.cpp \
    apply_changes.cpp \
    normalize.cpp \
    activity.cpp \
    selection.cpp \
    manipulator.cpp \
    gl_keepers.cpp \
    menuinfo.cpp \
    process.cpp \
    repository.cpp \
    git_backend.cpp \
    application.cpp \
    font.cpp \
    ../xlr/tree.cpp \
    ../xlr/gc.cpp \
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
    drag.cpp \
    pull_from_dialog.cpp \
    remote_selection_frame.cpp \
    publish_to_dialog.cpp \
    undo.cpp \
    clone_dialog.cpp \
    ansi_textedit.cpp \
    error_message_dialog.cpp \
    group_layout.cpp \
    resource_mgt.cpp \
    tree_cloning.cpp \
    font_file_manager.cpp
!win32 { 
    HEADERS += GL/glew.h \
        GL/glxew.h \
        GL/wglew.h
    SOURCES += glew.c
}
macx { 
    OBJECTIVE_SOURCES += font_file_manager_macos.mm
    LIBS += -framework \
        ApplicationServices
}
RESOURCES += tao.qrc

# We need bash, llvm-config
!system(bash -c \"bash --version >/dev/null\"):error("Can't execute bash")
!system(bash -c \"llvm-config --version >/dev/null\"):error("Can't execute llvm-config")

# LLVM dependencies
LLVM_LIBS = $$system(bash -c \"llvm-config --libs core jit native\")
LLVM_LIBS += $$system(bash -c \"llvm-config --ldflags\")
LLVM_INC = $$system(bash -c \"llvm-config --includedir\")
LLVM_DEF = $$system(bash -c \"llvm-config --cppflags | sed \'s/-I[^ ]*//g\' | sed s/-D//g\")

# Consolidated flags and libs
INCLUDEPATH += $$LLVM_INC
LIBS += $$LLVM_LIBS
DEFINES += $$LLVM_DEF

# Others
OTHER_FILES += xl.syntax \
    xl.stylesheet \
    short.stylesheet \
    html.stylesheet \
    debug.stylesheet \
    dbghtml.stylesheet \
    bytecode.stylesheet \
    builtins.xl \
    tutorial.ddd \
    git.stylesheet \
    srcview.stylesheet \
    srcview.css

# Copy the support files to the target directory
xlr_support.path = $${DESTDIR}/$${XLRDIR}
xlr_support.files += $${OTHER_FILES}
QMAKE_BUNDLE_DATA += xlr_support
FORMS += pull_from_dialog.ui \
    remote_selection_frame.ui \
    publish_to_dialog.ui \
    clone_dialog.ui \
    error_message_dialog.ui

# Automatic embedding of Git version
QMAKE_CLEAN += version.h
PRE_TARGETDEPS += version.h
revtarget.target = version.h
revtarget.commands = ./updaterev.sh
revtarget.depends = $$SOURCES $$HEADERS $$FORMS
QMAKE_EXTRA_TARGETS += revtarget
