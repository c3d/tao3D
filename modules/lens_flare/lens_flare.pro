# ******************************************************************************
#  lorem_ipsum.pro                                                  Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Lorem ipsum module
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************

MODINSTDIR = lens_flare

include(../modules.pri)
INCLUDEPATH += $${TAOTOPSRC}/tao/include/tao/
HEADERS      = flare.h \
    lens_flare.h
SOURCES      = lens_flare.cpp flare.cpp
win32 {
  DEFINES     += GLEW_STATIC
  SOURCES     += $${TAOTOPSRC}/tao/include/tao/GL/glew.c
}
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
