# ******************************************************************************
#  simple_lides.pro                                                 Tao project
# ******************************************************************************
# File Description:
# Project file for document template: simple slides

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/simple_slides
files.files = template.ini simple_slides.ddd simple_slides.png earth.bmp Corsica.jpg clouds.png

INSTALLS += files

OTHER_FILES += simple_slides.ddd simple_slides.png \
  earth.bmp Corsica.jpg clouds.png
