# ******************************************************************************
#  detach.pro                                                       Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Detach.exe utility (Windows only)
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
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
