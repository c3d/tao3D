# ******************************************************************************
#  shading.pro                                                       Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Shading

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/shading
files.files = template.ini shading.ddd shading.png models/*
 
INSTALLS += files
