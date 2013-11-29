# ******************************************************************************
#  charts.pro                                                    Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Charts

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/charts.ddd
files.files = template.ini charts.ddd charts.png charts.csv theme.xl

INSTALLS += files

SIGN_XL_SOURCES = charts.ddd theme.xl
include(../sign_template.pri)
