# ******************************************************************************
#  hello_world.pro                                                  Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Hello World module
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************

MODINSTDIR = hello_world

include(../modules.pri)

OTHER_FILES = module.xl

INSTALLS    -= thismod_bin
