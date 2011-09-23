# ******************************************************************************
#  tao_sign.pro                                                     Tao project
# ******************************************************************************
# File Description:
# 
#    Qt program description for the Tao licence signing utility
#
#
#
#
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
# (C) 2011 Jérôme Forissier <jerome@taodyne.com> 
# (C) 2011 Taodyne SAS
# ******************************************************************************

include(../main.pri)

TEMPLATE = app
TARGET   = tao_sign
SOURCES += sign.cpp
HEADERS += ../tao/licence.h
CONFIG  += console static
CONFIG  -= app_bundle

INC = . ../tao ../tao/include ../tao/include/tao ../tao/xlr/xlr/include
DEPENDPATH += $$INC
INCLUDEPATH += $$INC
LIBS += -L../libxlr/\$(DESTDIR) -lxlr

# REVISIT Move into tao.pro
# "make install" will generate and copy a temporary licence (licence.taokey)
# or just an unsigned licence template (licence.taokey.notsigned), depending
# on project settings: LICENCE_VALIDITY, TAO_EDITION.

!isEmpty(LICENCE_VALIDITY):VALID=$$LICENCE_VALIDITY
isEmpty(TAO_EDITION):VALID=120  # Days
!isEmpty(VALID) {
  macx {
    EXPIRES=$$system(date -u -j -f \"%s\" $(expr $(date +%s) + $$VALID \\* 86400 ) \"+%d-%h-%Y\")
  } else {
    # Linux, MinGW: GNU date
    EXPIRES=$$system(bash -c \"date -d \\\"now + $$VALID days\\\" +%d-%h-%Y\")
  }
  SIGN=1
} else {
  EXPIRES="31-Dec-2099"  # See licence.cpp
}
contains(TAO_EDITION, Viewer):SIGN=1
!isEmpty(TAO_EDITION):EDITION_STR="$$TAO_EDITION "
LATEST_TAG=$$system(git describe --tags --abbrev=0)
LATEST_TAG=$$replace(LATEST_TAG, \\., \\.)
!build_pass:message(---)
!isEmpty(LATEST_TAG) {
  FEATURES = \"Tao Presentations $${EDITION_STR}$${LATEST_TAG}.*\"  # Allows any version based on current tag
  !build_pass {
    message(--- We will install the following licence:)
    message(---)
    message(--- expires $$EXPIRES)
    message(--- features $$FEATURES)
    isEmpty(SIGN) {
      message(--- License file will NOT be signed (template only))
    } else {
      message(--- License file will be signed (valid))
    }
  }
} else {
  !build_pass:message(--- No default licence file will be installed)
}
!build_pass:message(---)
QMAKE_SUBSTITUTES = licence.taokey.notsigned.in

!isEmpty(SIGN) {
  # Sign and install licence
  macx:SIGN_CMD  = export DYLD_LIBRARY_PATH=../libxlr ; ./tao_sign
  macx:DEP = $$TARGET
  linux-g++*:SIGN_CMD = export LD_LIBRARY_PATH=../libxlr ; ./tao_sign
  linux-g++*:DEP = $$TARGET
  win32:SIGN_CMD = export PATH=\$\$PATH:../libxlr/\$(DESTDIR); \$(DESTDIR_TARGET)
  win32:DEP = \$(DESTDIR_TARGET)

  licence.target = licence.taokey
  licence.commands = cp licence.taokey.notsigned licence.taokey ; $${SIGN_CMD} licence.taokey || rm licence.taokey ; cp licence.taokey \"$$APPINST\"
  licence.files = licence.taokey
  licence.path = $$APPINST
  licence.depends = $$DEP

  INSTALLS += licence
  QMAKE_EXTRA_TARGETS += licence
  QMAKE_CLEAN += licence.taokey
  QMAKE_DISTCLEAN += licence.taokey.notsigned
} else {
  # Install unsigned template
  licence.files = licence.taokey.notsigned
  licence.path = $$APPINST
  INSTALLS += licence
}
