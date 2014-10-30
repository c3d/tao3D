# ******************************************************************************
#  detach.pro                                                       Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Detach.exe utility (Windows only)
# ******************************************************************************
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# *****************************************************************************
# This software is licensed under the GNU General Public License v3
# See file COPYING for details.
# ******************************************************************************


include(../main.pri)

TEMPLATE = app
TARGET   = Detach
CONFIG  += console
CONFIG  -= app_bundle
QT      -= core gui
SOURCES += main.cpp

target.path = $$APPINST
INSTALLS    = target
