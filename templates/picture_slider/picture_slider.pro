# ******************************************************************************
#  photo_viewewr.pro                                                Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Photo viewer

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/picture_slider
files.files = template.ini picture_slider.ddd picture_slider.jpg

images.path = $$APPINST/templates/picture_slider/images
images.files = images/*

INSTALLS += files images

SIGN_XL_SOURCES = picture_slider.ddd
include(../sign_template.pri)
