# ******************************************************************************
# slides.pro                                                       Tao3D project
# ******************************************************************************
#
# File description:
#
#     Qt build file for the Slides module
#
#
#
#
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2012, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
# (C) 2010, Catherine Burvelle <catherine@taodyne.com>
# (C) 2012,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
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

MODINSTDIR = slides

include(../modules.pri)

OTHER_FILES = slides.xl default-theme.xl module.doxy.h

PICTURES = \
    images/earth.bmp \
    images/clouds.png \
    $$NULL

pics.path  = $$MODINSTPATH/images
pics.files = $$PICTURES

default.path = $$MODINSTPATH
default.files = default-theme.xl

# icon from http://www.iconfinder.com/icondetails/15430/32/keynote_presentation_stand_icon
INSTALLS    += thismod_icon pics default
INSTALLS    -= thismod_bin

QMAKE_SUBSTITUTES = doc/Doxyfile.in
DOXYFILE = doc/Doxyfile
DOXYLANG = en,fr
include(../modules_doc.pri)

SIGN_XL_SOURCES = default-theme.xl
include(../sign_xl.pri)
