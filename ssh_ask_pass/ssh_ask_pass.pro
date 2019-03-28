# ******************************************************************************
# ssh_ask_pass.pro                                                 Tao3D project
# ******************************************************************************
#
# File description:
# Qt build file for the SshAskPass utility
#
#
#
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2010, Catherine Burvelle <catherine@taodyne.com>
# (C) 2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
# ******************************************************************************
# This file is part of Tao3D
#
# Tao3D is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Tao3D is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Tao3D, in a file named COPYING.
# If not, see <https://www.gnu.org/licenses/>.
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
