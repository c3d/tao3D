# ******************************************************************************
#  main.pro                                                         Tao project
# ******************************************************************************
# File Description:
# Main Qt build file for Tao
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
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
#   Options:
#
#   DEFINES += CFG_NOGIT
#     Build without Git support for Tao documents (Git is still used for module
#     update)
#   DEFINES += CFG_NOSTEREO
#     Build without support for stereoscopic displays
#   DEFINES += CFG_NOSRCEDIT
#     Build without document source editor
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
SUBDIRS  = libxlr tao modules ssh_ask_pass tests
win32:SUBDIRS += detach

tao.depends = libxlr
modules.depends = tao
tests.depends = tao

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
