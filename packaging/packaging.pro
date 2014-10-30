# ******************************************************************************
# packaging.pro                                                     Tao project
# ******************************************************************************
# File Description:
# Qt build file for the packaging directory
# ******************************************************************************
# This software is licensed under the GNU General Public License v3.
# See file COPYING for details.
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

TEMPLATE = subdirs
macx:SUBDIRS = macosx
win32:SUBDIRS = win
linux-g++*:SUBDIRS = linux

kit.commands = \$(MAKE) -C $$SUBDIRS kit
kit.depends = FORCE
QMAKE_EXTRA_TARGETS += kit
