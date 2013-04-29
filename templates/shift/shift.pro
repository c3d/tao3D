# ******************************************************************************
#  shift.pro                                                        Tao project
# ******************************************************************************
# File Description:
# Project file for document template: shift

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/shift
files.files = template.ini shift.ddd shift.xl shift.jpg

images.path = $$APPINST/templates/shift/images
images.files = images/*

INSTALLS += files images

SIGN_XL_SOURCES = shift.ddd shift.xl
include(../sign_template.pri)
