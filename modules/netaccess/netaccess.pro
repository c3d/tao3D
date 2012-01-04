# ******************************************************************************
#  netaccess.pro                                                  Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Hello World module
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************

MODINSTDIR = netaccess

include(../modules.pri)


INCLUDEPATH += $${TAOTOPSRC}/tao/include/tao/
HEADERS = netaccess.h

SOURCES = netaccess.cpp

TBL_SOURCES  = netaccess.tbl

OTHER_FILES = netaccess.xl netaccess.tbl traces.tbl
QT          += core \
               gui \
               opengl \
               network

INSTALLS    += thismod_icon

LICENSE_FILES = netaccess.taokey.notsigned
include(../licenses.pri)
