# ******************************************************************************
#  materials.pro                                                     Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Materials

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/materials
files.files = template.ini materials.ddd materials.png

images.path = $$APPINST/templates/materials/images
images.files = images/*
 
INSTALLS += files images
