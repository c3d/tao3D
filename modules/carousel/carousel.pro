# ******************************************************************************
#  carousel.pro                                                Tao project
# ******************************************************************************
# File Description:
#
#    Qt build file for the Carousel module
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# See file COPYING for details.
# (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

MODINSTDIR = carousel

include(../modules.pri)

OTHER_FILES = carousel.xl

INSTALLS    += thismod_icon
INSTALLS    -= thismod_bin

HEADERS += \
    doc/carousel.doxy.h

QMAKE_SUBSTITUTES = doc/Doxyfile.in
DOXYFILE = doc/Doxyfile
DOXYLANG = en,fr
include(../modules_doc.pri)
