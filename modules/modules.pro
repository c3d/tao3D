# ******************************************************************************
#  modules.pro                                                      Tao project
# ******************************************************************************
# File Description:
# Main build file for Tao modules
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2010-2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2010-2011 Taodyne SAS
# ******************************************************************************


TEMPLATE = subdirs

include(module_list.pri)
SUBDIRS  = $$MODULES

isEmpty(SUBDIRS) {
    message(Modules to build: (none))
} else {
    message(Modules to build: $$SUBDIRS)
}
