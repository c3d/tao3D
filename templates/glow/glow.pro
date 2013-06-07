# ******************************************************************************
#  glow.pro                                                          Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Glow

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/glow
files.files = template.ini glow.ddd glow.png

INSTALLS += files

SIGN_XL_SOURCES = glow.ddd
include(../sign_template.pri)
