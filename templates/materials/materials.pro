# ******************************************************************************
#  materials.pro                                                     Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Materials

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/materials
files.files = template.ini materials.ddd materials.jpg

images.path = $$APPINST/templates/materials/images
images.files = images/*
 
INSTALLS += files images

SIGN_XL_SOURCES = materials.ddd
include(../sign_template.pri)
