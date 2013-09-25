# ******************************************************************************
#  quiz.pro                                                          Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Tao quiz

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/quiz
files.files = template.ini Quiz.ddd quiz.png quiz-config.xl

images.path = $$APPINST/templates/quiz/images
images.files = images/*

INSTALLS += files # images

SIGN_XL_SOURCES = Quiz.ddd quiz-config.xl
include(../sign_template.pri)
