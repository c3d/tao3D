# ******************************************************************************
#  ssh_ask_pass.pro                                                 Tao project
# ******************************************************************************
# File Description:
# Qt build file for the SshAskPass utility
# ******************************************************************************
# This software is licensed under the GNU General Public License v3.
# See file COPYING for details.
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************


include(../main.pri)

TEMPLATE = app
TARGET   = SshAskPass
SOURCES += main.cpp\
           ssh_ask_pass_dialog.cpp
HEADERS += ssh_ask_pass_dialog.h
FORMS   += ssh_ask_pass_dialog.ui

win32:CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4) { QT += widgets }

target.path = $$APPINST
INSTALLS    = target

macx:QMAKE_POST_LINK = sh ../fix_qt_refs_app  "$(TARGET)"
