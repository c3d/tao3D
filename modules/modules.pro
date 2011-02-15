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

# The modules listed here are packaged with Tao and installed at the same time
# as the application ; they are not managed by Git (and therefore, each module
# MUST have its own version attribute in module.xl).

TEMPLATE = subdirs
# Modules built by default
SUBDIRS  = lorem_ipsum object_loader tao_visuals
# Optional modules, built with qmake modules=all
OTHER_SUBDIRS = hello_world taoTester

!exists(../main.pro) {
    # We're building inside the Tao SDK
    # SUBDIR should contain the example modules packaged with the Tao SDK
    SUBDIRS = object_loader
    OTHER_SUBDIRS =
}

# Process qmake command line variable: 'modules'
# Allows to add/remove modules to/from the default SUBDIRS value above
!isEmpty(modules) {
    options = $$modules
    RESULT = $$SUBDIRS
    for(opt, options) {
        equals(opt, "all") {
            RESULT = $$SUBDIRS $$OTHER_SUBDIRS
        }
        equals(opt, "none") {
            RESULT =
        }
        remove = $$find(opt, ^-.*)
        !isEmpty(remove) {
            remove ~= s/-//
            RESULT -= $$remove
        }
        add = $$find(opt, ^\\+.*)
        !isEmpty(add) {
            add ~= s/\\+//
            exists($$add) {
                RESULT += $$add
            } else {
                warning(Module directory not found: $$add -- not added)
            }
        }
    }
    SUBDIRS = $$RESULT
}

isEmpty(SUBDIRS) {
    message(Modules to build: (none))
} else {
    message(Modules to build: $$SUBDIRS)
}
