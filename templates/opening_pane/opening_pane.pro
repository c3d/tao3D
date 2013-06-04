# ******************************************************************************
#  opening_pane.pro                                                      Tao project
# ******************************************************************************
# File Description:
# Project file for document template: opening_pane

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/opening_pane
files.files = template.ini opening_pane.ddd opening_pane.xl flowers.xl opening_pane.jpg

images.path = $$APPINST/templates/opening_pane/images
images.files = images/*

INSTALLS += files images

SIGN_XL_SOURCES = opening_pane.ddd opening_pane.xl flowers.xl
include(../sign_template.pri)
