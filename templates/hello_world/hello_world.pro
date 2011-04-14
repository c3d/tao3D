# ******************************************************************************
#  hello_world.pro                                                  Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Hello World 

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/hello_world
files.files = template.ini hello_world.ddd hello_world.png

images.path = $$APPINST/templates/hello_world/images
images.files = images/*
 
INSTALLS += files images
