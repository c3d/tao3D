# ******************************************************************************
# tao.pro                                                          Tao3D project
# ******************************************************************************
#
# File description:
#
#    Main Qt build file for Tao
#
#
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
# (C) 2010-2011,2013-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
# ******************************************************************************
# This file is part of Tao3D
#
# Tao3D is free software: you can r redistribute it and/or modify
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

# Usage:
#
# 1. Generate Makefiles with:
#
#   (MacOSX)  qmake -r -spec macx-g++  [options]
#   (Windows) qmake -r -spec win32-g++ [options]
#   (Linux)   qmake -r -spec linux-g++ [options]
#
#   or with the configuration script in the top-level directory:
#
#   ./configure [options]
#
#   Options:
#
#   DEFINES+=CFG_NOGIT
#     Build without Git support for Tao documents (Git is still used for module
#     update)
#   DEFINES+=CFG_NONETWORK
#     Build without the "File>Open Network..." menu and without URI handling.
#     It is impossible to download a remote document, template or module.
#   DEFINES+=CFG_NOSRCEDIT
#     Build without document source editor
#   DEFINES+=CFG_NOMODPREF
#     Do not include the module page in the preference dialog
#   DEFINES+=NVALGRIND
#     Build libxlr without valgrind hooks. Normally not needed, even for release
#     builds because according to the valgrind documentation, the overhead is
#     negligible.
#   DEFINES+=XLR_GC_LIFO
#     (Debug) Tell the libxlr memory allocator to use a LIFO policy, i.e., do not
#     reuse freed objects immediately. May help valgrind detect more errors.
#   DEFINES+=CFG_NODISPLAYLINK
#     (MacOSX) Do not use a Core Video display link to refresh the display, but
#     a QBasicTimer (like other platforms).
#   DEFINES+=CFG_NORELOAD
#     Disable automatic reload when files change (.ddd, .xl)
#   DEFINES+=CFG_NOEDIT
#     Disable functions related to document edition: remove the Edit, Format,
#     Insert, Arrange and Share menus; remove Save, Save As, Save Fonts,
#     Sign for Player from the File menu.
#   DEFINES+=CFG_NOFULLSCREEN
#     Removes the View>Full screen menu and the related command chooser
#     entries. Zap the slide_show and toggle_slide_show primitives (they just
#     return false).
#   DEFINES+=CFG_TIMED_FULLSCREEN
#     Leave fullscreen mode automatically after 10 minutes with no user
#     interaction (mouse move or key press).
#   DEFINES+=CFG_WITH_EULA
#     Show the End-User License Agreement on startup, if not already accepted
#     for the current version.
#   DEFINES+=CFG_NO_NEW_FROM_TEMPLATE
#     Removes menus: File>New from Template..., Help>Themes and Help>Examples.
#   DEFINES+=CFG_NO_QTWEBKIT
#     Do not use QtWebKit. The url and url_texture primitives become no-ops.
#   DEFINES+=CFG_UNLICENSED_MAX_PAGES=<N>
#     Tao Player and Tao Studio (non-Libre editions) will limit the number of
#     pages in a document to <N>, unless the document has a valid signature.
#   DEFINES+=CFG_NO_LICENSE
#     For the Libre edition: Compile without license check code
#   DEFINES+=CFG_NO_CRYPT
#     For the Libre edition: Compile without signature/encryption code & keys
#   DEFINES+=CFG_LIBRE_EDITION
#     For the Libre edition: Show GPL dialog box at startup
#
#   modules=none
#     Do not build any Tao module
#   modules=all
#     Build all Tao modules (default and optional ones)
#   modules=+my_module
#     Add my_module to default module list
#   modules=-my_module
#     Remove my_modules from default module list
#   modules="all -my_module"
#     Build all modules except my_module
#   modules="none +my_module"
#     Build only my_module
#
#   templates=none
#   templates=all
#   templates=+my_template
#   templates=-my_template
#   templates=all -my_template
#   templates=none +my_template
#     Select which templates to install. Similar to 'modules' above.
#     Note: templates installed by theme modules (modules/themes/*) cannot be
#     selected with this variable. They can only be disabled altogether,
#     when templates contains the word "none".
#
#   NO_SDK=1
#     Do not include the module SDK in the package.
#   NO_DOC=1
#     Do not build online documentation.
#   NO_HELP_VIEWER=1
#     Do not build the help viewer application (the Help/Documentation menu is
#     suppressed at run time when the viewer is not there).
#   NO_WELCOME=1
#     Include a minimalistic welcome screen.
#   NO_FONTS=1
#     Do not install font files.
#   NO_DOC_SIGNATURE=1
#     Do not include the code to sign or validate document signature.
#   NO_WEBUI=1
#     Do not build the web-based document editor (webui).
#
# 2. To build:
#
#   $ make             # build everything
#   $ make install     # build and install Tao locally under ./install/
#   $ make sdk         # copy files needed for module development under ./sdk/
#   $ make clean
#   $ make distclean   # clean + remove Makefiles
#   $ make help        # this text
#
# Note: Packaging scripts are under ./packaging/
# --End Usage

# For Qt5.1, avoid massive warningification
greaterThan(QT_MAJOR_VERSION, 4) { cache() }

# Include global definitions and rules.
include(main.pri)

TEMPLATE = subdirs
SUBDIRS  = libxlr app modules ssh_ask_pass tests doc templates \
           packaging libcryptopp

win32:SUBDIRS += detach

isEmpty(NO_HELP_VIEWER) {
  SUBDIRS += help_viewer
} else {
  !build_pass:message(Will not build help viewer application.)
}

isEmpty(NO_WEBUI) {
  SUBDIRS += webui nodejs
} else {
  !build_pass:message(Will not build web user interface.)
}

app.depends = libxlr libcryptopp
tao_sign.depends = libxlr libcryptopp
modules.depends = app
tests.depends = app
templates.depends = app
xlconv.depends = libxlr

# The following is artificial, it's just so that we don't start building the
# doc until the main build has actually completed.
doc.depends = app libxlr modules ssh_ask_pass

sdk.commands = \$(MAKE) -f Makefile.sdk
sdk.depends = FORCE
QMAKE_EXTRA_TARGETS += sdk

# Print the help text above (delimited by lines with 'Usage')
help.commands = @awk \'f{print p} /Usage/&&f++{exit} {p=\$\$0}\' $$_PRO_FILE_
QMAKE_EXTRA_TARGETS += help

# Extend distclean target to also delete directories created by "make install"
# and "make sdk"
distclean_inst_sdk.commands = rm -rf ./install ./sdk
distclean_rm_config_h = rm -r ./config.h
distclean.depends = distclean_inst_sdk distclean_rm_config_h
QMAKE_EXTRA_TARGETS += distclean distclean_inst_sdk distclean_rm_config_h

kit.commands = \$(MAKE) -C packaging kit
kit.depends = FORCE
QMAKE_EXTRA_TARGETS += kit

# Run sanity tests after build in release mode
test.commands = \$(MAKE) -C tests/sanity check
test.depends = app modules templates
ref.commands = \$(MAKE) -C tests/sanity ref
ref.depends = app modules templates

QMAKE_EXTRA_TARGETS += test ref

TAO_CURRENT_PWD = $$PWD
QMAKE_SUBSTITUTES = fix_qt_refs_app.in

# Display configuration info
!build_pass {
  !isEmpty(NO_DOC):message(Documentation is disabled.)
  message(Configured with $$CONFIGURE_OPTIONS)
}
