# ******************************************************************************
#  blank.pro                                                        Tao project
# ******************************************************************************
# File Description:
# Project file for document template: blank

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/blank
files.files = template.ini blank.ddd blank.json blank.png

INSTALLS += files

SIGN_XL_SOURCES = blank.ddd
include(../sign_template.pri)
