# ******************************************************************************
#  templates.pri                                                    Tao project
# ******************************************************************************
# File Description:
# Project include file for Tao templates
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

include (../main_defs.pri)
# A template must be able to install itself under <dest>/templates/<dir>
# when intermediate directory "templates" does not exist yet
win32:QMAKE_MKDIR = mkdir -p
