# ******************************************************************************
#  simple_lides.pro                                                 Tao project
# ******************************************************************************
# File Description:
# Project file for document template: simple slides

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/pythagorean_theorem
files.files = template.ini pythagorean.ddd pythagorean.png

INSTALLS += files
