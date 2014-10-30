# ******************************************************************************
#  templates.pro                                                    Tao project
# ******************************************************************************
# File Description:
# Project file for Tao templates
# ******************************************************************************
# This software is licensed under the GNU General Public License v3.
# See file COPYING for details.
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

# "make install" will copy the templates to the staging directory

TEMPLATE = subdirs

include(template_list.pri)
SUBDIRS  = $$TEMPLATES

isEmpty(SUBDIRS) {
    message(Templates to install: (none))
} else {
    message(Templates to install: $$SUBDIRS)
}
