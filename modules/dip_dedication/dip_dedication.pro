
MODINSTDIR = dip_dedication

include(../modules.pri)

OTHER_FILES = dip_dedication.xl
OTHER_FILES += dip_dedication2.xl

CRYPT_XL_SOURCES = dip_dedication2.xl
include(../crypt_xl.pri)

INSTALLS    += thismod_icon
INSTALLS    -= thismod_bin

LICENSE_FILES = dip_dedication.taokey.notsigned
include(../licenses.pri)

QMAKE_SUBSTITUTES = doc/Doxyfile.in
QMAKE_DISTCLEAN = doc/Doxyfile
DOXYFILE = doc/Doxyfile
DOXYLANG = en,fr
include(../modules_doc.pri)

HEADERS += \
    doc/dip_dedication.doxy.h
