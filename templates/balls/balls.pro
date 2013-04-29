# ******************************************************************************
#  balls.pro                                                        Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Balls

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/balls
files.files = template.ini balls.ddd balls.png

reflect.path = $$APPINST/templates/balls/images/reflect
reflect.files = images/reflect/*

refract.path = $$APPINST/templates/balls/images/refract
refract.files = images/refract/*

INSTALLS += files reflect refract

SIGN_XL_SOURCES = balls.ddd
include(../sign_template.pri)
