# ******************************************************************************
#  opening_pane.pro                                                      Tao project
# ******************************************************************************
# File Description:
# Project file for document template: opening_pane

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/opening_pane
files.files = template.ini opening_pane.ddd long-scroll-md.png

images.path = $$APPINST/templates/opening_pane/images
images.files = images/*

INSTALLS += files images
