# ******************************************************************************
#  blue_gold.pro                                                     Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Blue gold

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/blue_gold
files.files = template.ini blue_gold.ddd blue_gold.png theme.xl

images.path = $$APPINST/templates/blue_gold/images
images.files = images/*
 
INSTALLS += files images

