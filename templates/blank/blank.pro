# ******************************************************************************
#  blank.pro                                                        Tao project
# ******************************************************************************
# File Description:
# Project file for document template: blank

TEMPLATE = subdirs

include(../../main_defs.pri)

files.path  = $$APPINST/templates/blank
files.files = template.ini blank.ddd blank.png

INSTALLS += files
