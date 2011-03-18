# ******************************************************************************
#  templates.pro                                                    Tao project
# ******************************************************************************
# File Description:
# Project file for Tao templates
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

# "make install" will copy the templates to the staging directory

TEMPLATE = subdirs
SUBDIRS  = blank

# Some templates depend on module availability
include (../modules/module_list.pri)
contains (MODULES, slides):SUBDIRS += simple_slides
