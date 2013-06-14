# ******************************************************************************
#  quizz.pro                                                     Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Tao quizz

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/quizz
files.files = template.ini Quizz.ddd quizz.png quizz-config.xl

images.path = $$APPINST/templates/quizz/images
images.files = images/*

INSTALLS += files # images

SIGN_XL_SOURCES = Quizz.ddd quizz-config.xl
include(../sign_template.pri)
