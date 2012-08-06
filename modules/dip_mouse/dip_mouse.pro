
MODINSTDIR = dip_mouse

include(../modules.pri)

OTHER_FILES = dip_mouse.xl
OTHER_FILES += dip_mouse2.xl

CRYPT_XL_SOURCES = dip_mouse2.xl
include(../crypt_xl.pri)

INSTALLS    += thismod_icon
INSTALLS    -= thismod_bin

LICENSE_FILES = dip_mouse.taokey.notsigned
include(../licenses.pri)

QMAKE_SUBSTITUTES = doc/Doxyfile.in
QMAKE_DISTCLEAN = doc/Doxyfile
DOXYFILE = doc/Doxyfile
DOXYLANG = en,fr
include(../modules_doc.pri)
