# ******************************************************************************
# app.pro                                                          Tao3D project
# ******************************************************************************
#
# File description:
# Main Qt build file for Tao
#
#
#
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2011-2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
# (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
# (C) 2010-2015,2018-2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
# (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
# (C) 2011-2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
# (C) 2010, Christophe de Dinechin <christophe@dinechin.org>
# ******************************************************************************
# This file is part of Tao3D
#
# Tao3D is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Tao3D is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Tao3D, in a file named COPYING.
# If not, see <https://www.gnu.org/licenses/>.
# ******************************************************************************

LIBXLR=../libxlr
include(../main.pri)
include(../version.pri) # required to process Info.plist.in, tao.rc.in
include(../gitrev.pri)

VERSION = $$TAO_VERSION
!build_pass:message(Version is $$VERSION)

TEMPLATE = app
macx:TARGET  = $$APP_NAME
!macx:TARGET =  Tao

INC = . \
    include \
    include/tao \
    ../libxlr \
    ../libxlr/xlr/include \
    ../libxlr/xlr \
    ../libcryptopp \
    ../keygen

DEPENDPATH += $$INC
INCLUDEPATH += $$INC
LIBS += -L../libxlr/\$(DESTDIR) -lxlr -L../libcryptopp/\$(DESTDIR) -lcryptopp
QT += network \
    opengl \
    svg
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += help printsupport
} else {
    CONFIG += help
}
QMAKE_SUBSTITUTES += version2.h.in
QMAKE_DISTCLEAN += version2.h
!isEmpty(TAO_PLAYER):DEFINES *= TAO_PLAYER
!isEmpty(NO_DOC_SIGNATURE):DEFINES *= CFG_NO_DOC_SIGNATURE
!isEmpty(NO_WEBUI):DEFINES *= CFG_NO_WEBUI
DEFINES += APP_NAME=\"\\\"$$APP_NAME\\\"\"

macx {
    APPNAME_NOSPC=$$replace(APPNAME,' ',)
    CFBUNDLEEXECUTABLE=$$TARGET
    XLRDIR = Contents/MacOS
    ICON = tao.icns
    FILETYPES.files = tao-doc.icns
    FILETYPES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += FILETYPES
    QMAKE_SUBSTITUTES += Info.plist.in
    QMAKE_INFO_PLIST = Info.plist
    QMAKE_DISTCLEAN += Info.plist
    QMAKE_CFLAGS +=
}
win32 {
    QMAKE_SUBSTITUTES += tao.rc.in
    RC_FILE = tao.rc
    LIBS += -limagehlp -lws2_32 # ws2_32 for ntohs()
}
linux-g++* {
    LIBS += -lXss -lGLU -ldl -lX11
}

# Input
HEADERS +=     activity.h \
    application.h \
    apply_changes.h \
    assistant.h \
    attributes.h \
    binpack.h \
    chooser.h \
    crypto.h \
    destination_folder_dialog.h \
    dir.h \
    display_driver.h \
    documentation.h \
    drag.h \
    drawing.h \
    error_message_dialog.h \
    examples_menu.h \
    file_monitor.h \
    font.h \
    font_file_manager.h \
    frame.h \
    gc_thread.h \
    gl_keepers.h \
    glyph_cache.h \
    group_layout.h \
    http_ua.h \
    info_trash_can.h \
    init_cleanup.h \
    inspectordialog.h \
    justification.h \
    layout.h \
    license.h \
    license_dialog.h \
    lighting.h \
    manipulator.h \
    menuinfo.h \
    module_info_dialog.h \
    module_manager.h \
    module_renderer.h \
    nag_screen.h \
    normalize.h \
    page_layout.h \
    path3d.h \
    preferences_dialog.h \
    preferences_pages.h \
    preview.h \
    tao_process.h \
    qtlocalpeer.h \
    qtlockedfile.h \
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
    text_drawing.h \
    html_converter.h \
    texture.h \
    texture_cache.h \
    tool_window.h \
    transforms.h \
    tree_cloning.h \
    update_application.h \
    widget.h \
    widget_surface.h \
    window.h \
    opengl_state.h \
    opengl_save.h \
    include/tao/coords.h \
    include/tao/coords3d.h \
    include/tao/coords4d.h \
    include/tao/color.h \
    include/tao/matrix.h \
    include/tao/module_api.h \
    include/tao/module_info.h \
    include/tao/tao_gl.h \
    include/tao/tao_info.h \
    include/tao/tao_utf8.h \
    include/tao/graphic_state.h

SOURCES += \
    opengl_state.cpp \
    opengl_save.cpp \
    activity.cpp \
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
    file_monitor.cpp \
    font.cpp \
    font_file_manager.cpp \
    formulas.cpp \
    frame.cpp \
    gc_thread.cpp \
    gl_keepers.cpp \
    glyph_cache.cpp \
    graphics.cpp \
    group_layout.cpp \
    http_ua.cpp \
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
    nag_screen.cpp \
    normalize.cpp \
    page_layout.cpp \
    path3d.cpp \
    preferences_dialog.cpp \
    preferences_pages.cpp \
    preview.cpp \
    tao_process.cpp \
    qtlocalpeer.cpp \
    qtlockedfile.cpp \
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
    text_drawing.cpp \
    html_converter.cpp \
    texture.cpp \
    texture_cache.cpp \
    tool_window.cpp \
    transforms.cpp \
    tree_cloning.cpp \
    update_application.cpp \
    version.cpp \
    widget.cpp \
    widget_surface.cpp \
    window.cpp

win32 {
    HEADERS += dde_widget.h
    SOURCES += dde_widget.cpp qtlockedfile_win.cpp
}

unix {
    SOURCES += qtlockedfile_unix.cpp
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
contains(DEFINES, CFG_NOGIT):contains(DEFINES,CFG_NONETWORK) {
   # Nothing
} else {
    HEADERS += git_backend.h
    SOURCES += git_backend.cpp
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
contains(DEFINES, CFG_WITH_UPDATES) {
    !build_pass:message("Check for update on startup is enabled by default")
}
contains(DEFINES, CFG_NO_NEW_FROM_TEMPLATE) {
    !build_pass:message("[CFG_NO_NEW_FROM_TEMPLATE] File/New from Template is disabled")
} else {
    HEADERS += \
        new_document_wizard.h \
        templates.h
    SOURCES += \
        new_document_wizard.cpp \
        templates.cpp
}
contains(DEFINES, CFG_NO_QTWEBKIT) {
    !build_pass:message("[CFG_NO_QTWEBKIT] QtWebKit disabled: primitives url/url_texture will do nothing")
} else {
    QT += webkit
    greaterThan(QT_MAJOR_VERSION, 4) { QT += webkitwidgets }
}
contains(DEFINES, CFG_LICENSE_DOWNLOAD) {
    !build_pass:message("[CFG_LICENSE_DOWNLOAD] License download is enabled")
    HEADERS += \
        license_download.h \
        login_dialog.h
    SOURCES += \
        license_download.cpp \
        login_dialog.cpp
}
!exists(../keygen/doc_private_key_dsa.h):DEFINES+=CFG_NO_DOC_SIGNATURE
contains(DEFINES, CFG_NO_DOC_SIGNATURE) {
    !build_pass:message("[CFG_NO_DOC_SIGNATURE] Document signing and verification is disabled")
} else {
    HEADERS += \
        document_signature.h
    NOWARN_SOURCES += \
        document_signature.cpp
}
!exists(../keygen/private_key_rsa.h):DEFINES+=CFG_NO_CRYPT
contains(DEFINES, CFG_NO_CRYPT) {
    !build_pass:message("[CFG_NO_CRYPT] Document decryption disabled")
} else {
    NOWARN_SOURCES += \
         crypto.cpp
}

contains(DEFINES, CFG_NO_CRYPT):DEFINES+=CFG_NO_LICENSE CFG_LIBRE_EDITION
contains(DEFINES, CFG_NO_LICENSE) {
    !build_pass:message("[CFG_NO_LICENSE] License checks are disabled")
} else {
    NOWARN_SOURCES += \
         license.cpp
}

contains(DEFINES, CFG_NO_WEBUI) {
    !build_pass:message("[CFG_NO_WEBUI] Web-based editor disabled")
} else {
    HEADERS += webui.h
    SOURCES += webui.cpp
}

# XL_MODULES += formulas.tbl graphics.tbl attributes.tbl

win32 {
    !build_pass:message("Using GLEW")
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
        -Wl,-rpath,@executable_path/../Frameworks \
        -Wl,-rpath,$$[QT_INSTALL_LIBS]

    # Make sure libGLC references the Qt libraries bundled with the application
    # and not the ones that may be installed on the target system, otherwise
    # they may clash
    FIX_QT_REFS = ../modules/fix_qt_refs
    QMAKE_POST_LINK = $$FIX_QT_REFS "$(TARGET)" \"$$[QT_INSTALL_LIBS]\"
}
linux-g++* {
    HEADERS += vsync_x11.h
    SOURCES += vsync_x11.cpp
}
RESOURCES += tao.qrc

# Files loaded at runtime
SUPPORT_FILES = ../libxlr/xlr/xlr/builtins.xl \
    tao_fr.xl \
    xl.syntax \
    C.syntax \
    xl.stylesheet \
    git.stylesheet \
    nocomment.stylesheet \
    debug.stylesheet \
    error_filters.txt

# Other files to show in the Qt Creator interface
OTHER_FILES +=  \
    license.cpp \
    crypto.cpp \
    document_signature.cpp \
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
    welcome/welcome.ddd \
    opengl_state.tbl \
    no_welcome/welcome.ddd

FORMS += error_message_dialog.ui \
    inspectordialog.ui \
    render_to_file_dialog.ui

# Automatic embedding of Git version
QMAKE_CLEAN += version.h
PRE_TARGETDEPS += version.h
revtarget.target = version.h
revtarget.commands = ./updaterev.sh
revtarget.depends = $$SOURCES \
    $$HEADERS \
    $$FORMS
QMAKE_EXTRA_TARGETS += revtarget

# ../config.h is normally created by ../configure
# Make an empty one in case the project was configured by invoking
# qmake directly
PRE_TARGETDEPS += ../config.h
config_h.target = ../config.h
config_h.commands = echo \"/* Generated by app.pro */\" >../config.h
QMAKE_EXTRA_TARGETS += config_h

# Pre-processing of taodyne_ad.xl
QMAKE_CLEAN += taodyne_ad.h
PRE_TARGETDEPS += taodyne_ad.h
taodyne_ad.target = taodyne_ad.h
taodyne_ad.commands = sed -f taodyne_ad.sed < taodyne_ad.xl > taodyne_ad.h
taodyne_ad.depends = taodyne_ad.xl taodyne_ad.sed
QMAKE_EXTRA_TARGETS += taodyne_ad


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
tao_xl.commands = perl ../tools/preprocessor.pl $$DEFS tao.xl.in > tao.xl && cp tao.xl \"$$APPINST\"
tao_xl.files = tao.xl
tao_xl.path = $$APPINST
tao_xl.depends = tao.xl.in
INSTALLS += tao_xl
QMAKE_EXTRA_TARGETS += tao_xl
QMAKE_CLEAN += tao.xl

# What to install
xl_files.path  = $$APPINST
xl_files.files = $${SUPPORT_FILES}
CONFIG(debug, debug|release):xl_files.files += xlr/xlr/debug.stylesheet
INSTALLS    += xl_files

isEmpty(NO_DOC_SIGNATURE) {
  # Keep a local copy of xlr/xlr/builtins.xl (to create the .sig file in .)
  cp_builtins.commands = cp ../libxlr/xlr/xlr/builtins.xl .
  cp_builtins.target = builtins.xl
  cp_builtins.depends = ../libxlr/xlr/xlr/builtins.xl
  cp_builtins.CONFIG = no_check_exist
  QMAKE_CLEAN += builtins.xl
  QMAKE_EXTRA_TARGETS += cp_builtins

  # Install signed XL files (.xl.sig)
  SIGN_XL_SOURCES = builtins.xl tao.xl tao_fr.xl
  TAOTOPSRC=..
  MODINSTPATH=$$APPINST
  include(../modules/sign_xl.pri)
}

# Welcome document
welcome.path  = $$APPINST/welcome
isEmpty(NO_WELCOME) {
  welcome.files = welcome/*.png welcome/*.svg welcome/welcome.ddd
  isEmpty(NO_DOC_SIGNATURE):SIGN_XL_SOURCES = welcome/welcome.ddd
} else {
  !build_pass:message([NO_WELCOME] Welcome screen is disabled.)
  welcome.files = no_welcome/welcome.ddd
  isEmpty(NO_DOC_SIGNATURE):SIGN_XL_SOURCES = no_welcome/welcome.ddd
}
INSTALLS += welcome
isEmpty(NO_DOC_SIGNATURE) {
  # Sign welcome.ddd
  TAOTOPSRC=..
  SIGN_XL_INSTPATH=$$APPINST/welcome
  include(../modules/sign_xl.pri)
}

# Blank document
blank.path = $$APPINST
blank.commands = touch \"$$APPINST/blank.ddd\"
blank.depends = FORCE
INSTALLS += blank

isEmpty(NO_FONTS) {
  fonts.path  = $$APPINST/fonts
  fonts.files = fonts/*.ttf fonts/*.otf fonts/README
  !contains(DEFINES, CFG_NOSRCEDIT):fonts.files += fonts/unifont/unifont-5.1.20080907.ttf
  INSTALLS += fonts
} else {
    !build_pass:message("[NO_FONTS] No font file wil be installed.")
}

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

shaders.path = $$APPINST
shaders.files = lighting.vs lighting.fs
INSTALLS += shaders
