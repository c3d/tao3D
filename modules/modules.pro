# ******************************************************************************
#  modules.pro                                                      Tao project
# ******************************************************************************
# File Description:
# Main build file for Tao modules
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************


TEMPLATE = subdirs

include(module_list.pri)
SUBDIRS  = $$MODULES

!exists(../main.pro) {
    # We're building inside the Tao SDK
    # SUBDIR should contain the example modules packaged with the Tao SDK
    SUBDIRS = object_loader
    OTHER_SUBDIRS =
}

isEmpty(SUBDIRS) {
    message(Modules to build: (none))
} else {
    message(Modules to build: $$SUBDIRS)
}
