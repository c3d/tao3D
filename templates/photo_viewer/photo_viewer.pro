# ******************************************************************************
#  photo_viewewr.pro                                                Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Photo viewer

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/photo_viewer
files.files = template.ini photo_viewer.ddd photo_viewer.png

images.path = $$APPINST/templates/photo_viewer/images
images.files = images/*

photos.path = $$APPINST/templates/photo_viewer/photos
photos.files = photos/*

INSTALLS += files images photos
