# ******************************************************************************
#  doc.pro                                                          Tao project
# ******************************************************************************
# File Description:
# Qt build file for tao documentation
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 2011 Catherine Burvelle <catherine@taodyne.com>
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

#include (../main.pri)

TEMPLATE = subdirs

macx {

  QMAKE_SUBSTITUTES = Doxyfile.in 

  include (../modules/module_list.pri)
  MOD_PATHS=$$join(MODULES, " ../modules/", "../modules/")

  dox.target = doc
  dox.commands = doxygen 

  clean.commands = /bin/rm -rf html/ 

  QMAKE_EXTRA_TARGETS += dox clean

} else {

  message("Documentation is only build on MacOSX systems.

}
