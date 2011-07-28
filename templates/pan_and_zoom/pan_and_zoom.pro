# ******************************************************************************
#  movie.pro                                                       Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Movie 

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/pan_and_zoom
files.files = template.ini pan_and_zoom.ddd pan_and_zoom.png

images.path = $$APPINST/templates/movie/images
images.files = images/*

INSTALLS += files images
