# ******************************************************************************
#  sparks.pro                                                        Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Sparks

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/sparks
files.files = template.ini sparks.ddd sparks.png

INSTALLS += files

SIGN_XL_SOURCES = sparks.ddd
include(../sign_template.pri)
