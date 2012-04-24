# ******************************************************************************
#  opening_pane.pro                                                      Tao project
# ******************************************************************************
# File Description:
# Project file for document template: opening_pane

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/monjori
files.files = template.ini opening_pane.ddd opening_pane.png

INSTALLS += files
