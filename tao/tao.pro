# ******************************************************************************
# tao.pro                                                            Tao project
# ******************************************************************************
# File Description:
# Main Qt build file for Tao
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
# (C) 2010 Catherine Burvelle <cathy@taodyne.com>
# (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************

include(../main.pri)
include(../version.pri) # required to process Info.plist.in, tao.rc.in

TEMPLATE = app
!macx:TARGET =  Tao
 macx:TARGET = "Tao Presentations"
INC = . \
    include \
    include/tao \
    xlr/xlr/include
DEPENDPATH += $$INC
INCLUDEPATH += $$INC
win32:LIBS += -L../libxlr/release -L../libxlr/debug  # REVISIT
LIBS += -L../libxlr -lxlr
QT += webkit \
    network \
    opengl \
    svg \
    phonon

QMAKE_CFLAGS += -Werror
QMAKE_CXXFLAGS += -Werror
QMAKE_CXXFLAGS_RELEASE += -g \
    \$(CXXFLAGS_\$%)

macx {
    CFBUNDLEEXECUTABLE=$$TARGET
    XLRDIR = Contents/MacOS
    ICON = tao.icns
    FILETYPES.files = tao-doc.icns
    FILETYPES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += FILETYPES
    QMAKE_SUBSTITUTES += Info.plist.in
    QMAKE_INFO_PLIST = Info.plist
    QMAKE_DISTCLEAN += Info.plist
    QMAKE_CFLAGS += -mmacosx-version-min=10.5 # Avoid warning with font_file_manager_macos.mm
}
win32 {
    QMAKE_SUBSTITUTES += tao.rc.in
    RC_FILE  = tao.rc
}
linux-g++* {
    LIBS += -lXss
}

# Input
HEADERS += widget.h \
    window.h \
    application.h \
    frame.h \
    svg.h \
    texture.h \
    include/tao/coords.h \
    include/tao/coords3d.h \
    color.h \
    gl_keepers.h \
    drawing.h \
    shapes.h \
    text_drawing.h \
    shapes3d.h \
    path3d.h \
    table.h \
    chooser.h \
    binpack.h \
    glyph_cache.h \
    attributes.h \
    lighting.h \
    transforms.h \
    layout.h \
    page_layout.h \
    space_layout.h \
    justification.h \
    justification.hpp \
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
    drag.h \
    error_message_dialog.h \
    group_layout.h \
    resource_mgt.h \
    tree_cloning.h \
    font_file_manager.h \
    splash_screen.h \
    documentation.h \
    new_document_wizard.h \
    preferences_dialog.h \
    preferences_pages.h \
    module_manager.h \
    text_edit.h \
    tao_main.h \
    tool_window.h \
    include/tao/module_api.h \
    include/tao/module_info.h \
    module_renderer.h \
    layout_cache.h \
    render_to_file_dialog.h \
    inspectordialog.h \
    raster_text.h \
    dir.h \
    templates.h \
    module_info_dialog.h

SOURCES += tao_main.cpp \
    widget.cpp \
    window.cpp \
    frame.cpp \
    svg.cpp \
    widget_surface.cpp \
    texture.cpp \
    drawing.cpp \
    shapes.cpp \
    text_drawing.cpp \
    shapes3d.cpp \
    path3d.cpp \
    table.cpp \
    chooser.cpp \
    binpack.cpp \
    glyph_cache.cpp \
    attributes.cpp \
    lighting.cpp \
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
    drag.cpp \
    error_message_dialog.cpp \
    group_layout.cpp \
    resource_mgt.cpp \
    tree_cloning.cpp \
    font_file_manager.cpp \
    splash_screen.cpp \
    documentation.cpp \
    new_document_wizard.cpp \
    preferences_dialog.cpp \
    preferences_pages.cpp \
    module_manager.cpp \
    text_edit.cpp \
    tool_window.cpp \
    module_api_p.cpp \
    module_renderer.cpp \
    layout_cache.cpp \
    render_to_file_dialog.cpp \
    inspectordialog.cpp \
    raster_text.cpp \
    dir.cpp \
    templates.cpp \
    module_info_dialog.cpp

# Check compile-time options

contains(DEFINES, CFG_NOGIT) {
    message("Document history and sharing with Git is disabled")
} else {
    HEADERS += \
        ansi_textedit.h \
        branch_selection_combobox.h \
        branch_selection_tool.h \
        checkout_dialog.h \
        clone_dialog.h \
        commit_selection_combobox.h \
        commit_table_model.h \
        commit_table_widget.h \
        diff_dialog.h \
        diff_highlighter.h \
        fetch_dialog.h \
        fetch_push_dialog_base.h \
        git_toolbar.h \
        history_dialog.h \
        history_frame.h \
        history_playback.h \
        history_playback_tool.h \
        merge_dialog.h \
        open_uri_dialog.h \
        pull_from_dialog.h \
        push_dialog.h \
        remote_selection_frame.h \
        selective_undo_dialog.h \
        undo.h \
        uri.h
    SOURCES += \
        ansi_textedit.cpp \
        branch_selection_combobox.cpp \
        branch_selection_tool.cpp \
        checkout_dialog.cpp \
        clone_dialog.cpp \
        commit_selection_combobox.cpp \
        commit_table_model.cpp \
        commit_table_widget.cpp \
        diff_dialog.cpp \
        diff_highlighter.cpp \
        fetch_dialog.cpp \
        fetch_push_dialog_base.cpp \
        git_toolbar.cpp \
        history_dialog.cpp \
        history_frame.cpp \
        history_playback.cpp \
        history_playback_tool.cpp \
        merge_dialog.cpp \
        open_uri_dialog.cpp \
        pull_from_dialog.cpp \
        push_dialog.cpp \
        remote_selection_frame.cpp \
        selective_undo_dialog.cpp \
        undo.cpp \
        uri.cpp
    FORMS += \
        pull_from_dialog.ui \
        remote_selection_frame.ui \
        clone_dialog.ui \
        merge_dialog.ui \
        history_dialog.ui \
        open_uri_dialog.ui \
        fetch_push_dialog.ui \
        history_frame.ui \
        diff_dialog.ui
}
contains(DEFINES, CFG_NOSTEREO) {
    message("Stereoscopic display support is disabled")
}
contains(DEFINES, CFG_NOSRCEDIT) {
    message("Document source editor is disabled")
} else {
    HEADERS += \
        xl_source_edit.h \
        xl_highlighter.h
    SOURCES += \
        xl_source_edit.cpp \
        xl_highlighter.cpp
}

CXXTBL_SOURCES += graphics.cpp \
    formulas.cpp

!macx {
    HEADERS += GL/glew.h \
        GL/glxew.h \
        GL/wglew.h
    SOURCES += GL/glew.c
    DEFINES += GLEW_STATIC
}
macx {
    OBJECTIVE_SOURCES += font_file_manager_macos.mm
    LIBS += -framework \
        ApplicationServices \
        -Wl,-macosx_version_min,10.5 \
        -Wl,-rpath,@executable_path/../Frameworks \
        -Wl,-rpath,$$QMAKE_LIBDIR_QT

    # Make sure libGLC references the Qt libraries bundled with the application
    # and not the ones that may be installed on the target system, otherwise
    # they may clash
    FIX_QT_REFS = ../modules/fix_qt_refs
    QMAKE_POST_LINK = $$FIX_QT_REFS "$(TARGET)" \"$$QMAKE_LIBDIR_QT\"
}
RESOURCES += tao.qrc

# Others
OTHER_FILES += xl.syntax \
    xl.stylesheet \
    short.stylesheet \
    html.stylesheet \
    debug.stylesheet \
    dbghtml.stylesheet \
    bytecode.stylesheet \
    xlr/xlr/builtins.xl \
    tao.xl \
    tao_fr.xl \
    welcome.ddd \
    git.stylesheet \
    traces.tbl \
    nocomment.stylesheet \
    graphics.tbl \
    Info.plist.in \
    html/module_info_dialog.html \
    html/module_info_dialog_fr.html

# Copy the support files to the target directory
xlr_support.path = $${DESTDIR}/$${XLRDIR}
xlr_support.files += $${OTHER_FILES}
QMAKE_BUNDLE_DATA += xlr_support

FORMS += error_message_dialog.ui \
    render_to_file_dialog.ui \
    inspectordialog.ui

# Automatic embedding of Git version
QMAKE_CLEAN += version.h
PRE_TARGETDEPS += version.h
revtarget.target = version.h
revtarget.commands = ./updaterev.sh
revtarget.depends = $$SOURCES \
    $$HEADERS \
    $$FORMS
QMAKE_EXTRA_TARGETS += revtarget

# What to install
xl_files.path  = $$APPINST
xl_files.files = xlr/xlr/builtins.xl \
    tao.xl \
    tao_fr.xl \
    xl.syntax \
    xl.stylesheet \
    git.stylesheet \
    welcome.ddd
CONFIG(debug, debug|release):xl_files.files += xlr/xlr/debug.stylesheet
fonts.path  = $$APPINST/fonts
fonts.files = fonts/*
INSTALLS    += xl_files fonts
macx {
  # Workaround install problem: on Mac, the standard way of installing (the 'else'
  # part of this block) starts by recursively deleting $$target.path/Tao.app.
  # This is bad since we have previously stored libraries there :(
  app.path    = $$INSTROOT
  app.extra   = \$(INSTALL_DIR) \"$${TARGET}.app\" $$INSTROOT
  INSTALLS   += app
} else {
  target.path = $$INSTROOT
  INSTALLS   += target
}


TRANSLATIONS = tao_fr.ts

lupdate.commands = lupdate -verbose tao.pro
lupdate.depends  = FORCE
QMAKE_EXTRA_TARGETS += lupdate

lrelease.commands = lrelease tao.pro
lrelease.depends  = FORCE
QMAKE_EXTRA_TARGETS += lrelease

translations.path = $$APPINST
translations.files = *.qm
INSTALLS += translations
