# ******************************************************************************
#  pigs_fly.pro                                                     Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Pigs Fly

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/pigs_fly
files.files = template.ini pigs_fly.ddd pigs_fly.png

images.path = $$APPINST/templates/pigs_fly/images
images.files = images/*

plane.path  = $$APPINST/templates/pigs_fly/plane
plane.files = plane/*

piggy.path  = $$APPINST/templates/pigs_fly/piggy
piggy.files = piggy/*

INSTALLS += files images plane piggy
