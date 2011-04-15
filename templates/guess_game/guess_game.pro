# ******************************************************************************
#  guess_game.pro                                                  Tao project
# ******************************************************************************
# File Description:
# Project file for document template: Guess Game

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/guess_game
files.files = template.ini guess_game.ddd guess_game.png

images.path = $$APPINST/templates/guess_game/images
images.files = images/*

INSTALLS += files images

OTHER_FILES += guess_game.ddd \
    guess_game.png \
    images/France.svg \
    images/UnitedKingdom.svg
