
MODINSTDIR = landscapes

OTHER_FILES = landscapes.xl

PICTURES = landscapes/*

include(../themes.pri)

landscapes.path = $$APPINST/templates/landscapes/landscapes
landscapes.files = landscapes/*

INSTALLS += landscapes

INSTALLS    += thismod_icon
INSTALLS    -= thismod_bin
