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
# Modules built by default
SUBDIRS  = hello_world lorem_ipsum
# Optional modules, built with qmake modules=all
OTHER_SUBDIRS = taoTester

# Process qmake command line variable: 'modules'
# Allows to add/remove modules to/from the default SUBDIRS value above
!isEmpty(modules) {
    options = $$modules
    RESULT = $$SUBDIRS
    for(opt, options) {
        contains(opt, "all") {
            RESULT = $$SUBDIRS $$OTHER_SUBDIRS
        }
        contains(opt, "none") {
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

message(Modules to build: $$SUBDIRS)
