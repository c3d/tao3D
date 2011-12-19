# ******************************************************************************
#  audio_video.pro                                                  Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Audio Video module
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

MODINSTDIR = audio_video

include(../modules.pri)

DEFINES    += GLEW_STATIC
HEADERS     = audio_video.h
SOURCES     = audio_video.cpp
!macx:SOURCES += $${TAOTOPSRC}/tao/include/tao/GL/glew.c
TBL_SOURCES = audio_video.tbl
OTHER_FILES = audio_video.xl audio_video.tbl traces.tbl

QT       += core gui opengl phonon

# Icon: http://www.iconfinder.com/icondetails/18182/32/multimedia_package_icon
# Author: Everaldo Coelho (Crystal Project)
# License: LGPL
INSTALLS += thismod_icon

QMAKE_SUBSTITUTES = doc/Doxyfile.in
DOXYFILE = doc/Doxyfile
include(../modules_doc.pri)
