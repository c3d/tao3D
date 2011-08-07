# ******************************************************************************
#  custom_slides.pro                                               Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Movie 

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/custom_slides
files.files = template.ini custom_slides.ddd theme.xl custom_slides.png

images.path = $$APPINST/templates/custom_slides/images
images.files = images/*

INSTALLS += files images
