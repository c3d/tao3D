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
    xlr/xlr/include \
    ../libcryptopp \
    ../keygen
DEPENDPATH += $$INC
INCLUDEPATH += $$INC
LIBS += -L../libxlr/\$(DESTDIR) -lxlr -L../libcryptopp/\$(DESTDIR) -lcryptopp
QT += webkit \
    network \
    opengl \
    svg
CONFIG += help
QMAKE_SUBSTITUTES += version2.h.in
QMAKE_DISTCLEAN += version2.h

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
    LIBS += -limagehlp -lws2_32 # ws2_32 for ntohs()
}
linux-g++* {
    LIBS += -lXss -lGLU
}

# Input
HEADERS +=     activity.h \
    application.h \
    apply_changes.h \
    assistant.h \
    attributes.h \
    binpack.h \
    chooser.h \
    color.h \
    decryption.h \
    destination_folder_dialog.h \
    dir.h \
    display_driver.h \
    documentation.h \
    drag.h \
    drawing.h \
    error_message_dialog.h \
    examples_menu.h \
    font.h \
    font_file_manager.h \
    frame.h \
    gc_thread.h \
    git_backend.h \
    gl_keepers.h \
    glyph_cache.h \
    group_layout.h \
    info_trash_can.h \
    init_cleanup.h \
    inspectordialog.h \
    justification.h \
    justification.hpp \
    layout.h \
    licence.h \
    license_dialog.h \
    lighting.h \
    manipulator.h \
    menuinfo.h \
    module_info_dialog.h \
    module_manager.h \
    module_renderer.h \
    new_document_wizard.h \
    normalize.h \
    page_layout.h \
    path3d.h \
    preferences_dialog.h \
    preferences_pages.h \
    process.h \
    raster_text.h \
    render_to_file_dialog.h \
    repository.h \
    resource_mgt.h \
    selection.h \
    shapes.h \
    shapes3d.h \
    space_layout.h \
    splash_screen.h \
    statistics.h \
    svg.h \
    table.h \
    tao_main.h \
    tao_tree.h \
    templates.h \
    text_drawing.h \
    text_edit.h \
    texture.h \
    tool_window.h \
    transforms.h \
    tree_cloning.h \
    update_application.h \
    widget.h \
    widget_surface.h \
    window.h \
    include/tao/coords.h \
    include/tao/coords3d.h \
    include/tao/matrix.h \
    include/tao/module_api.h \
    include/tao/module_info.h \
    include/tao/tao_gl.h \
    include/tao/tao_info.h \
    include/tao/tao_utf8.h \
    include/tao/graphic_state.h

SOURCES +=     activity.cpp \
    application.cpp \
    apply_changes.cpp \
    assistant.cpp \
    attributes.cpp \
    binpack.cpp \
    chooser.cpp \
    destination_folder_dialog.cpp \
    dir.cpp \
    display_driver.cpp \
    documentation.cpp \
    drag.cpp \
    drawing.cpp \
    error_message_dialog.cpp \
    examples_menu.cpp \
    font.cpp \
    font_file_manager.cpp \
    frame.cpp \
    gc_thread.cpp \
    git_backend.cpp \
    gl_keepers.cpp \
    glyph_cache.cpp \
    group_layout.cpp \
    info_trash_can.cpp \
    init_cleanup.cpp \
    inspectordialog.cpp \
    layout.cpp \
    license_dialog.cpp \
    lighting.cpp \
    manipulator.cpp \
    menuinfo.cpp \
    module_api_p.cpp \
    module_info_dialog.cpp \
    module_manager.cpp \
    module_renderer.cpp \
    new_document_wizard.cpp \
    normalize.cpp \
    page_layout.cpp \
    path3d.cpp \
    preferences_dialog.cpp \
    preferences_pages.cpp \
    process.cpp \
    raster_text.cpp \
    render_to_file_dialog.cpp \
    repository.cpp \
    resource_mgt.cpp \
    selection.cpp \
    shapes.cpp \
    shapes3d.cpp \
    space_layout.cpp \
    splash_screen.cpp \
    statistics.cpp \
    svg.cpp \
    table.cpp \
    tao_main.cpp \
    templates.cpp \
    text_drawing.cpp \
    text_edit.cpp \
    texture.cpp \
    tool_window.cpp \
    transforms.cpp \
    tree_cloning.cpp \
    update_application.cpp \
    widget.cpp \
    widget_surface.cpp \
    window.cpp \
    graphic_state.cpp

win32 {
    HEADERS += dde_widget.h
    SOURCES += dde_widget.cpp
}

# Check compile-time options

contains(DEFINES, CFG_NOGIT) {
    !build_pass:message("Document history and sharing with Git is disabled")
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
        pull_from_dialog.h \
        push_dialog.h \
        remote_selection_frame.h \
        selective_undo_dialog.h \
        undo.h
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
        pull_from_dialog.cpp \
        push_dialog.cpp \
        remote_selection_frame.cpp \
        selective_undo_dialog.cpp \
        undo.cpp
    FORMS += \
        clone_dialog.ui \
        diff_dialog.ui \
        fetch_push_dialog.ui \
        history_dialog.ui \
        history_frame.ui \
        merge_dialog.ui \
        pull_from_dialog.ui \
        remote_selection_frame.ui
}
contains(DEFINES, CFG_NONETWORK) {
    !build_pass:message("File>Open Nework and opening URIs (docs, templates, modules) is disabled")
} else {
    HEADERS += \
        open_uri_dialog.h \
        uri.h
    SOURCES += \
        open_uri_dialog.cpp \
        uri.cpp
    FORMS += \
        open_uri_dialog.ui
}
contains(DEFINES, CFG_NOSTEREO) {
    !build_pass:message("Stereoscopic display support is disabled")
}
contains(DEFINES, CFG_NOSRCEDIT) {
    !build_pass:message("Document source editor is disabled")
} else {
    HEADERS += \
        xl_highlighter.h \
        xl_source_edit.h
    SOURCES += \
        xl_highlighter.cpp \
        xl_source_edit.cpp
}
contains(DEFINES, CFG_NORELOAD) {
    !build_pass:message("Automatic document reload is disabled")
}
contains(DEFINES, CFG_NOEDIT) {
    !build_pass:message("Editing functions are disabled (Edit, Insert, Format, Arrange, Share, Save, Save As, Save Fonts, Consolidate)")
}
contains(DEFINES, CFG_NOFULLSCREEN) {
    !build_pass:message("Full screen (slideshow) mode is disabled")
}
contains(DEFINES, CFG_NOMODULEUPDATE) {
    !build_pass:message("Module update is disabled")
}
contains(DEFINES, CFG_WITH_EULA) {
    !build_pass:message("EULA dialog is enabled")
    HEADERS += eula_dialog.h
    SOURCES += eula_dialog.cpp
}
CXXTBL_SOURCES += formulas.cpp graphics.cpp

NOWARN_SOURCES += decryption.cpp licence.cpp

!macx {
    HEADERS += include/tao/GL/glew.h \
        include/tao/GL/glxew.h \
        include/tao/GL/wglew.h
    SOURCES += include/tao/GL/glew.c
    DEFINES += GLEW_STATIC
}
macx {
    OBJECTIVE_SOURCES += font_file_manager_macos.mm
    !contains(DEFINES, CFG_NODISPLAYLINK):LIBS += -framework CoreVideo
    LIBS += -framework ApplicationServices -framework Foundation \
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

# Files loaded at runtime
SUPPORT_FILES = xlr/xlr/builtins.xl \
    tao_fr.xl \
    xl.syntax \
    C.syntax \
    xl.stylesheet \
    git.stylesheet \
    nocomment.stylesheet \
    debug.stylesheet

# Other files to show in the Qt Creator interface
OTHER_FILES +=  \
    licence.cpp \
    decryption.cpp \
    tao.xl.in \
    $${SUPPORT_FILES} \
    traces.tbl \
    graphics.tbl \
    attributes.tbl \
    shapes.tbl \
    shapes3d.tbl \
    manipulator.tbl \
    transforms.tbl \
    text_drawing.tbl \
    table.tbl \
    widget_surface.tbl \
    chooser.tbl \
    frame.tbl \
    lighting.tbl \
    Info.plist.in \
    html/module_info_dialog.html \
    html/module_info_dialog_fr.html \
    tao_fr.ts \
    welcome/welcome.ddd

FORMS += error_message_dialog.ui \
    inspectordialog.ui \
    render_to_file_dialog.ui

# Automatic embedding of Git version
QMAKE_CLEAN += version.h
PRE_TARGETDEPS += version.h
revtarget.target = version.h
revtarget.commands = ./updaterev.sh "$${TAO_EDITION}"
revtarget.depends = $$SOURCES \
    $$HEADERS \
    $$FORMS
QMAKE_EXTRA_TARGETS += revtarget

# Automatic embedding of changelog file (NEWS)
system(cp ../NEWS ./NEWS)
QMAKE_CLEAN += NEWS
changelog.target = NEWS
changelog.commands = cp ../NEWS .
changelog.depends = ../NEWS
QMAKE_EXTRA_TARGETS += changelog

# Pre-processing of tao.xl.in to obtain tao.xl
# preprocessor.pl comes from http://software.hixie.ch/utilities/unix/preprocessor/
!system(perl -e "exit"):error("Can't execute perl")
DEFS = $$join(DEFINES, " -D", " -D")
tao_xl.target = tao.xl
tao_xl.commands = perl preprocessor.pl $$DEFS tao.xl.in > tao.xl && cp tao.xl \"$$APPINST\"
tao_xl.files = tao.xl
tao_xl.path = $$APPINST
tao_xl.depends = tao.xl.in
INSTALLS += tao_xl
QMAKE_EXTRA_TARGETS += tao_xl
QMAKE_CLEAN += tao.xl

# What to install
xl_files.path  = $$APPINST
xl_files.files = $${SUPPORT_FILES}

welcome.path  = $$APPINST/welcome
welcome.files = welcome/*.png welcome/*.svg welcome/welcome.ddd
INSTALLS += welcome

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

# Create license directory
licdir.commands = mkdir -p \"$${APPINST}/licenses\"
licdir.path = .
licdir.depends = FORCE
INSTALLS += licdir

TRANSLATIONS = tao_fr.ts
include(../translations.pri)
translations.path = $$APPINST
translations.files = *.qm
INSTALLS += translations

qttranslations.path = $$APPINST
qttranslations.files = $$[QT_INSTALL_TRANSLATIONS]/qt_fr.qm $$[QT_INSTALL_TRANSLATIONS]/qt_help_fr.qm
INSTALLS += qttranslations

shaders.path = $$APPINST$
shaders.files = lighting.vs lighting.fs
INSTALLS += shaders
