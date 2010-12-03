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
SUBDIRS  = hello_world lorem_ipsum taoTester
