# ******************************************************************************
#  slides.pro                                                       Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Slides module
#
# The Slides module defines commands and themes to write presentations 
#
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************

MODINSTDIR = slides

include(../modules.pri)

OTHER_FILES = slides.xl default-theme.xl keyboard-theme.xl seyes-theme.xl basic-theme.xl blue-theme.xl brown-theme.xl triangles-theme.xl rectangles-theme.xl lucky-theme.xl module.doxy.h

PICTURES = \
    images/keyboard.jpg \
    images/seyes.jpg \
    images/blue-main.png \
    images/blue-section.png \
    images/blue-story.png \
    images/brown-main.png \
    images/brown-section.png \
    images/brown-story.png \
    images/earth.bmp \
    images/clouds.png \
    $$NULL

pics.path  = $$MODINSTPATH/images
pics.files = $$PICTURES

default.path = $$MODINSTPATH
default.files = default-theme.xl keyboard-theme.xl seyes-theme.xl basic-theme.xl blue-theme.xl brown-theme.xl triangles-theme.xl rectangles-theme.xl lucky-theme.xl 

# icon from http://www.iconfinder.com/icondetails/15430/32/keynote_presentation_stand_icon
INSTALLS    += thismod_icon pics default 
INSTALLS    -= thismod_bin

QMAKE_SUBSTITUTES = doc/Doxyfile.in
DOXYFILE = doc/Doxyfile
DOXYLANG = en,fr
include(../modules_doc.pri)
