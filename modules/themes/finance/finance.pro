
MODINSTDIR = finance

OTHER_FILES = finance.xl

PICTURES = images/*

include(../themes.pri)

images.path = $$APPINST/templates/finance/images
images.files = images/*

INSTALLS += images

INSTALLS    += thismod_icon
INSTALLS    -= thismod_bin
