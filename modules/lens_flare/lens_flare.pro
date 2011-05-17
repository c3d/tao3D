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

DEFINES     += GLEW_STATIC

include(../modules.pri)
INCLUDEPATH += $${TAOTOPSRC}/tao/include/tao/
HEADERS      = flare.h \
    lens_flare.h
SOURCES      = lens_flare.cpp flare.cpp $${TAOTOPSRC}/tao/include/tao/GL/glew.c
TBL_SOURCES  = lens_flare.tbl
OTHER_FILES  = lens_flare.xl lens_flare.tbl traces.tbl
QT          += core \
               gui \
               opengl

flares_img.path  = $$MODINSTPATH/images
flares_img.files = images/sun_flare0.jpg images/sun_flare1.jpg images/sun_flare2.jpg images/sun_flare3.jpg images/sun_flare4.jpg images/sun_flare5.jpg \
                   images/white_flare0.jpg images/white_flare1.jpg images/white_flare2.jpg images/white_flare3.jpg images/white_flare4.jpg images/white_flare5.jpg \
                   images/flare0.jpg images/flare1.jpg images/flare2.jpg images/flare3.jpg images/flare4.jpg images/flare5.jpg images/flare6.jpg

INSTALLS    += flares_img thismod_icon
