# ******************************************************************************
#  blank.pro                                                        Tao project
# ******************************************************************************
# File Description:
# Project file for document template: blank

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/blank
files.files = template.ini blank.ddd blank.png

INSTALLS += files
