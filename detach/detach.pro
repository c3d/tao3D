# ******************************************************************************
#  detach.pro                                                       Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Detach.exe utility (Windows only)
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
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
