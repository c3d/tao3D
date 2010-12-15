# ******************************************************************************
#  main_defs.pri                                                    Tao project
# ******************************************************************************
# File Description:
# Global project definitions for Tao (Tao-specific variables only)
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************


INSTROOT = $$PWD/install
macx {
APPINST = $$INSTROOT/Tao.app/Contents/MacOS
LIBINST = $$INSTROOT/Tao.app/Contents/Frameworks
} else {
APPINST = $$INSTROOT
LIBINST = $$INSTROOT
}

