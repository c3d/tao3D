# *****************************************************************************
#  regexp.pro                                                      Tao project 
# *****************************************************************************
# 
#   File Description:
# 
#     Module build for RegExp module
# 
# 
# 
# 
# *****************************************************************************
#  (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
#  (C) 2011 Taodyne SAS
# *****************************************************************************

MODINSTDIR = regexp

include(../modules.pri)

HEADERS     = regexp.h
SOURCES     = regexp.cpp
TBL_SOURCES = regexp.tbl
OTHER_FILES = regexp.xl traces.tbl regexp.tbl

INSTALLS    += thismod_icon

QT += core
QMAKE_SUBSTITUTES = doc/Doxyfile.in
QMAKE_DISTCLEAN = doc/Doxyfile
DOXYFILE = doc/Doxyfile
DOXYLANG = en,fr
include(../modules_doc.pri)
