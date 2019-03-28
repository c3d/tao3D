# ******************************************************************************
# lens_flare.pro                                                   Tao3D project
# ******************************************************************************
#
# File description:
# Qt build file for the Lorem ipsum module
#
#
#
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
# (C) 2011, Soulisse Baptiste <baptiste.soulisse@taodyne.com>
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

MODINSTDIR = lens_flare

include(../modules.pri)
INCLUDEPATH += $${TAOTOPSRC}/app/include/tao/
HEADERS      = flare.h \
    lens_flare.h
SOURCES      = lens_flare.cpp flare.cpp
TBL_SOURCES  = lens_flare.tbl
OTHER_FILES  = lens_flare.xl lens_flare.tbl traces.tbl
QT          += core \
               gui \
               opengl

flares_img.path  = $$MODINSTPATH/images
flares_img.files = images/*.jpg

INSTALLS    += flares_img thismod_icon

TRANSLATIONS = lens_flare_fr.ts
include($${TAOTOPSRC}/translations.pri)
translations.path = $$MODINSTPATH
translations.files = *.qm
INSTALLS += translations

QMAKE_SUBSTITUTES = doc/Doxyfile.in
DOXYFILE = doc/Doxyfile
DOXYLANG = en,fr
include(../modules_doc.pri)
