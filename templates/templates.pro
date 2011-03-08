# ******************************************************************************
#  templates.pro                                                    Tao project
# ******************************************************************************
# File Description:
# Project file for Tao templates
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

# "make install" will copy the templates to the staging directory

TEMPLATE = subdirs
SUBDIRS  = blank

# Some templates depend on module availability
include (../modules/module_list.pri)
contains (MODULES, slides):SUBDIRS += simple_slides
