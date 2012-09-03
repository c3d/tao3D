# ******************************************************************************
#  newsfeed.pro                                                     Tao project
# ******************************************************************************
# File Description:
# Project file for document template: newsfeed 

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/newsfeed
files.files = template.ini news.ddd newsfeed.png

INSTALLS += files
