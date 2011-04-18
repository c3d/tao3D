# ******************************************************************************
#  modules.pri                                                      Tao project
# ******************************************************************************
# File Description:
# Common project include file to build Tao modules
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************

include(modules_defs.pri)
include(../main.pri)

TEMPLATE = lib
TARGET   = $$basename(MODINSTPATH)
CONFIG  += dll
QT      -= core gui

INC = . $${TAOTOPSRC}/tao/xlr/xlr/include $${TAOTOPSRC}/tao/include
INCLUDEPATH += $$INC
DEPENDPATH  += $$INC
win32:LIBS += -L$${TAOTOPSRC}/libxlr/release -L$${TAOTOPSRC}/libxlr/debug  # REVISIT
LIBS += -L$${TAOTOPSRC}/libxlr -lxlr
# A module's native library can install its dependencies in the same directory
# Windows and MacOS look there by default, Linux does not
linux-g++*:LIBS += -Wl,-rpath=.

# How to generate *_wrap.cpp from *.tbl
# Usage:
#   TBL_SOURCES = my_module.tbl
isEmpty(TBL_WRAP):TBL_WRAP = $$PWD/tbl_wrap
tbl_wrap.name = TBL_WRAP ${QMAKE_FILE_IN}
tbl_wrap.commands = $$TBL_WRAP -o ${QMAKE_FILE_BASE}_wrap.cpp ${QMAKE_FILE_NAME}
tbl_wrap.variable_out = SOURCES
tbl_wrap.output = ${QMAKE_FILE_BASE}_wrap.cpp
tbl_wrap.input = TBL_SOURCES
tbl_wrap.clean = ${QMAKE_FILE_BASE}_wrap.cpp
QMAKE_EXTRA_COMPILERS += tbl_wrap

# Default module installation rules
macx:MODULE  = lib$${TARGET}.dylib
linux-g++*:MODULE = lib$${TARGET}.so
win32 {
    CONFIG(debug, debug|release):DD=debug
    CONFIG(release, debug|release):DD=release
    MODULE = $${DD}/$${TARGET}.dll
}
thismod_xl.path   = $$MODINSTPATH
thismod_xl.files  = $${TARGET}.xl
INSTALLS += thismod_xl
thismod_bin.path  = $${MODINSTPATH}/lib
# Workaround http://bugreports.qt.nokia.com/browse/QTBUG-5558
# thismod_bin.files = $$MODULE
macx {
  thismod_bin.extra = \$(INSTALL_PROGRAM) $$MODULE \"$$thismod_bin.path\" ; $$FIX_QT_REFS \"$$thismod_bin.path/$$MODULE\" \"$$QMAKE_LIBDIR_QT\"
} else {
  thismod_bin.extra = \$(INSTALL_PROGRAM) $$MODULE \"$$thismod_bin.path\"
}
INSTALLS += thismod_bin
thismod_icon.path  = $$MODINSTPATH
thismod_icon.files = icon.png
