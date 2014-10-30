# ******************************************************************************
#  main_defs.pri                                                    Tao project
# ******************************************************************************
# File Description:
# Global project definitions for Tao (Tao-specific variables only)
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# See file COPYING for details.
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************

include(version.pri)

INSTROOT = $$PWD/install
macx {
APPINST = "$$INSTROOT/$${APP_NAME}.app/Contents/MacOS"
LIBINST = "$$INSTROOT/$${APP_NAME}.app/Contents/Frameworks"
} else {
APPINST = $$INSTROOT
LIBINST = $$INSTROOT
}
CONFIG(release, debug|release):RELEASE=1
