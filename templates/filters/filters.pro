# ******************************************************************************
#  filters.pro                                                    Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Filters

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/filters
files.files = template.ini filters.ddd filters.png

images.path = $$APPINST/templates/filters/images
images.files = images/*
 
INSTALLS += files images

SIGN_XL_SOURCES = filters.ddd
include(../sign_template.pri)
