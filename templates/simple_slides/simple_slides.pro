# ******************************************************************************
#  simple_slides.pro                                                Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Simple Slides

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/simple_slides
files.files = template.ini simple_slides.ddd theme.xl simple_slides.png

images.path = $$APPINST/templates/simple_slides/images
images.files = images/*

INSTALLS += files images
