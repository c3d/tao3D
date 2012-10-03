# ******************************************************************************
#  rising_text.pro                                                      Tao project
# ******************************************************************************
# File Description:
# Project file for document template: rising_text

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/rising_text
files.files = template.ini rising_text.ddd rising_text.jpg rising_text.xl

images.path = $$APPINST/templates/rising_text/images
images.files = images/*

INSTALLS += files images
