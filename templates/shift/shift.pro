# ******************************************************************************
#  shift.pro                                                        Tao project
# ******************************************************************************
# File Description:
# Project file for document template: shift

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/shift
files.files = template.ini shift.ddd shift.png

images.path = $$APPINST/templates/shift/images
images.files = images/*

INSTALLS += files images
