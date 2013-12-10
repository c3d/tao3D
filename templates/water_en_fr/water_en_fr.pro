# ******************************************************************************
#  water_en_fr.pro                                                  Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Multilingual demo (based on Water theme)

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/water_en_fr
files.files = template.ini water_en_fr.ddd water_en_fr.jpg

INSTALLS += files

SIGN_XL_SOURCES = water_en_fr.ddd
include(../sign_template.pri)
