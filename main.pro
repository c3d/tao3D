# ******************************************************************************
#  main.pro                                                         Tao project
# ******************************************************************************
# File Description:
# Main Qt build file for Tao
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Christophe de Dinechin <christophe@taodyne.com>
# (C) 2010 Taodyne SAS
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
#     Insert, Arrange and Share menus.
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
#
# 2. To build:
#
#   $ make             # build everything
#   $ make install     # install Tao locally under ./install/
#   $ make sdk         # copy files needed for module development under ./sdk/
#   $ make clean
#   $ make distclean   # clean + remove Makefiles
#   $ make help        # this text
#
# Note: Packaging scripts are under ./packaging/
# --End Usage

# Include global definitions and rules.
include(main.pri)

TEMPLATE = subdirs
SUBDIRS  = libxlr tao modules ssh_ask_pass tao_sign tests doc templates \
           packaging libcryptopp keygen crypt

win32:SUBDIRS += detach

isEmpty(NO_HELP_VIEWER) {
  SUBDIRS += help_viewer
} else {
  !build_pass:message(Will not build help viewer application.)
}

tao.depends = libxlr libcryptopp
tao_sign.depends = libxlr libcryptopp tao
keygen.depends = libcryptopp tao
modules.depends = tao tao_sign crypt
tests.depends = tao
templates.depends = tao
xlconv.depends = libxlr
crypt.depends = libcryptopp

# The following is artificial, it's just so that we don't start building the
# doc until the main build has actually completed.
doc.depends = tao libxlr modules ssh_ask_pass

sdk.commands = \$(MAKE) -f Makefile.sdk
sdk.depends = FORCE
QMAKE_EXTRA_TARGETS += sdk

# Print the help text above (delimited by lines with 'Usage')
help.commands = @awk \'f{print p} /Usage/&&f++{exit} {p=\$\$0}\' $$_PRO_FILE_
QMAKE_EXTRA_TARGETS += help

# Extend distclean target to also delete directories created by "make install"
# and "make sdk"
distclean_inst_sdk.commands = rm -rf ./install ./sdk
distclean.depends = distclean_inst_sdk
QMAKE_EXTRA_TARGETS += distclean distclean_inst_sdk

kit.commands = \$(MAKE) -C packaging kit
kit.depends = FORCE
QMAKE_EXTRA_TARGETS += kit

QMAKE_SUBSTITUTES = fix_qt_refs_app.in

# Display configuration info
!build_pass {
  !isEmpty(NO_DOC):message(Documentation is disabled.)
}
