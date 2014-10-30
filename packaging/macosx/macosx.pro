# ******************************************************************************
# macosx.pro                                                        Tao project
# ******************************************************************************
# File Description:
# Qt build file to generate a MacOSX package
# ******************************************************************************
# This software is licensed under the GNU General Public License v3.
# See file COPYING for details.
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

TEMPLATE = subdirs

kit.commands   = $(MAKE) -f Makefile.macosx
prepare.commands   = $(MAKE) -f Makefile.macosx prepare
clean.commands = $(MAKE) -f Makefile.macosx clean
distclean.depends = clean

QMAKE_EXTRA_TARGETS = kit prepare clean distclean

include (../../main_defs.pri)
ARCH=x86_64
contains(CONFIG, x86):ARCH=x86
QMAKE_SUBSTITUTES = Makefile.config.in
