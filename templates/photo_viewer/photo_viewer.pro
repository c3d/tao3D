# ******************************************************************************
#  photo_viewewr.pro                                                Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Photo viewer

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/photo_viewer
files.files = template.ini photo_viewer.ddd photo_viewer.jpg

photos.path = $$APPINST/templates/photo_viewer/photos
photos.files = photos/*

INSTALLS += files photos

SIGN_XL_SOURCES = photo_viewer.ddd
include(../sign_template.pri)
