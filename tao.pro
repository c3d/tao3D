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
    xlr/xlr
INCLUDEPATH += . \
    xlr/xlr
QT += webkit \
    network \
    opengl \
    svg \
    phonon
QMAKE_CFLAGS += -Werror
QMAKE_CXXFLAGS += -Werror
QMAKE_CXXFLAGS_RELEASE += -g

# Tell the XLR portion that we are building for Tao
DEFINES += TAO \
    DEBUG
macx { 
    DEFINES += CONFIG_MACOSX
    XLRDIR = Contents/MacOS
    ICON = tao.icns
    QMAKE_INFO_PLIST = Info.plist
    QMAKE_CFLAGS += -mmacosx-version-min=10.5  # Avoid warning with font_file_manager_macos.mm
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
    chooser.h \
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
    xlr/xlr/utf8.h \
    xlr/xlr/base.h \
    xlr/xlr/options.h \
    xlr/xlr/basics.h \
    xlr/xlr/parser.h \
    xlr/xlr/compiler.h \
    xlr/xlr/renderer.h \
    xlr/xlr/configuration.h \
    xlr/xlr/runtime.h \
    xlr/xlr/context.h \
    xlr/xlr/scanner.h \
    xlr/xlr/errors.h \
    xlr/xlr/serializer.h \
    xlr/xlr/hash.h \
    xlr/xlr/sha1.h \
    xlr/xlr/main.h \
    xlr/xlr/sha1_ostream.h \
    xlr/xlr/opcodes.h \
    xlr/xlr/syntax.h \
    xlr/xlr/opcodes_declare.h \
    xlr/xlr/tree.h \
    xlr/xlr/gc.h \
    xlr/xlr/opcodes_define.h \
    xlr/xlr/types.h \
    xlr/xlr/diff.h \
    xlr/xlr/lcs.h \
    xlr/xlr/bfs.h \
    drag.h \
    pull_from_dialog.h \
    remote_selection_frame.h \
    publish_to_dialog.h \
    undo.h \
    clone_dialog.h \
    ansi_textedit.h \
    xlr/xlr/opcodes_delete.h \
    error_message_dialog.h \
    group_layout.h \
    resource_mgt.h \
    tree_cloning.h \
    font_file_manager.h \
    splash_screen.h \
    branch_selection_combobox.h \
    branch_selection_toolbar.h \
    fetch_dialog.h \
    merge_dialog.h \
    commit_selection_combobox.h \
    history_dialog.h \
    revert_to_dialog.h \
    selective_undo_dialog.h \
    documentation.h \
    uri.h \
    open_uri_dialog.h \
    new_document_wizard.h
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
    chooser.cpp \
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
    xlr/xlr/tree.cpp \
    xlr/xlr/gc.cpp \
    xlr/xlr/sha1.cpp \
    xlr/xlr/serializer.cpp \
    xlr/xlr/syntax.cpp \
    xlr/xlr/scanner.cpp \
    xlr/xlr/runtime.cpp \
    xlr/xlr/renderer.cpp \
    xlr/xlr/parser.cpp \
    xlr/xlr/options.cpp \
    xlr/xlr/opcodes.cpp \
    xlr/xlr/main.cpp \
    xlr/xlr/errors.cpp \
    xlr/xlr/context.cpp \
    xlr/xlr/compiler.cpp \
    xlr/xlr/basics.cpp \
    xlr/xlr/types.cpp \
    xlr/xlr/diff.cpp \
    xlr/xlr/lcs.cpp \
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
    font_file_manager.cpp \
    splash_screen.cpp \
    branch_selection_combobox.cpp \
    branch_selection_toolbar.cpp \
    fetch_dialog.cpp \
    merge_dialog.cpp \
    commit_selection_combobox.cpp \
    history_dialog.cpp \
    revert_to_dialog.cpp \
    selective_undo_dialog.cpp \
    documentation.cpp \
    uri.cpp \
    open_uri_dialog.cpp \
    new_document_wizard.cpp
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
    error_message_dialog.ui \
    fetch_dialog.ui \
    merge_dialog.ui \
    history_dialog.ui \
    open_uri_dialog.ui

# Automatic embedding of Git version
QMAKE_CLEAN += version.h
PRE_TARGETDEPS += version.h
revtarget.target = version.h
revtarget.commands = ./updaterev.sh
revtarget.depends = $$SOURCES \
    $$HEADERS \
    $$FORMS
QMAKE_EXTRA_TARGETS += revtarget
