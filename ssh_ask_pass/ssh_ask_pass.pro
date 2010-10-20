# ******************************************************************************
#  ssh_ask_pass.pro                                                 Tao project
# ******************************************************************************
# File Description:
# Qt build file for the SshAskPass utility
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
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

target.path = $$APPINST
INSTALLS    = target
