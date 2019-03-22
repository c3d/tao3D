# ******************************************************************************
# macosx.pro                                                       Tao3D project
# ******************************************************************************
#
# File description:
# Qt build file to generate a MacOSX package
#
#
#
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
# (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
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
