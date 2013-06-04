# ******************************************************************************
#  sinuous.pro                                                       Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Sinuous

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/sinuous
files.files = template.ini sinuous.ddd sinuous.png

INSTALLS += files

SIGN_XL_SOURCES = sinuous.ddd
include(../sign_template.pri)
