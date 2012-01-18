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
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

MODINSTDIR = carousel

include(../modules.pri)

OTHER_FILES = carousel.xl \
    carousel2.xl \

CRYPT_XL_SOURCES = carousel2.xl
include(../crypt_xl.pri)

INSTALLS    += thismod_icon
INSTALLS    -= thismod_bin

HEADERS += \
    doc/carousel.doxy.h

LICENSE_FILES = carousel.taokey.notsigned
include(../licenses.pri)

QMAKE_SUBSTITUTES = doc/Doxyfile.in
DOXYFILE = doc/Doxyfile
DOXYLANG = en,fr
include(../modules_doc.pri)
