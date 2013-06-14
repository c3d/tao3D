# ******************************************************************************
#  newsfeed.pro                                                     Tao project
# ******************************************************************************
# File Description:
# Project file for document template: newsfeed 

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/newsfeed
files.files = template.ini news.ddd newsfeed.jpg

INSTALLS += files

SIGN_XL_SOURCES = news.ddd
include(../sign_template.pri)
