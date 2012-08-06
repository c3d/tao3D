
MODINSTDIR = dip_null

include(../modules.pri)

OTHER_FILES = dip_null.xl
OTHER_FILES += dip_null2.xl

CRYPT_XL_SOURCES = dip_null2.xl
include(../crypt_xl.pri)

INSTALLS    += thismod_icon
INSTALLS    -= thismod_bin

LICENSE_FILES = dip_null.taokey.notsigned
include(../licenses.pri)

QMAKE_SUBSTITUTES = doc/Doxyfile.in
QMAKE_DISTCLEAN = doc/Doxyfile
DOXYFILE = doc/Doxyfile
DOXYLANG = en,fr
include(../modules_doc.pri)
