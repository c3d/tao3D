# ******************************************************************************
#  @@MODULE_DIR@@.pro                                               Tao project
# ******************************************************************************
# File Description:
# Qt build file for the @@IMPORT_NAME@@ module
#
# @@DESCRIPTION@@
#
# ******************************************************************************
#ifdef AUTHOR_IS_TAODYNE
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
#endif
# (C) @@COPYRIGHT_YEAR@@ @@AUTHOR@@
# ******************************************************************************

MODINSTDIR = @@MODULE_DIR@@

include(../modules.pri)

OTHER_FILES = @@MODULE_DIR@@.xl
#ifdef WITH_CRYPT
OTHER_FILES += @@MODULE_DIR@@2.xl

CRYPT_XL_SOURCES = @@MODULE_DIR@@2.xl
include(../crypt_xl.pri)
#endif

INSTALLS    += thismod_icon
INSTALLS    -= thismod_bin

#ifdef WITH_LICENSE
LICENSE_FILES = @@MODULE_DIR@@.taokey.notsigned
include(../licenses.pri)
#endif

#ifdef WITH_DOC
QMAKE_SUBSTITUTES = doc/Doxyfile.in
DOXYFILE = doc/Doxyfile
DOXYLANG = en,fr
include(../modules_doc.pri)
#endif
