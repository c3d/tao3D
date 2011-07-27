# ******************************************************************************
#  movie.pro                                                       Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Movie 

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/movie
files.files = template.ini movie.ddd movie.png

images.path = $$APPINST/templates/movie/images
images.files = images/*

INSTALLS += files images
