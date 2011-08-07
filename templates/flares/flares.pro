# ******************************************************************************
#  flares.pro                                                       Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Flares

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/flares
files.files = template.ini flares.ddd flares.png

INSTALLS += files
