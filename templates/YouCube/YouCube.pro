# ******************************************************************************
#  YouCube.pro                                                      Tao project
# ******************************************************************************
# File Description:
# Project file for document template: YouCube 

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/YouCube
files.files = template.ini YouCube.ddd YouCube.png

images.path = $$APPINST/templates/YouCube/images
images.files = images/*

INSTALLS += files images

SIGN_XL_SOURCES = YouCube.ddd
include(../sign_template.pri)
