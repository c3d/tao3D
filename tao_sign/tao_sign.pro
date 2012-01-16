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
QT      -= gui

# Turn off warnings caused by Crypto++ headers (unused parameters)
CONFIG += warn_off
QMAKE_CXXFLAGS -= -Werror

INC = . ../tao ../tao/include ../tao/include/tao ../tao/xlr/xlr/include \
      ../libcryptopp ../keygen
DEPENDPATH += $$INC
INCLUDEPATH += $$INC
LIBS += -L../libxlr/\$(DESTDIR) -lxlr -L../libcryptopp/\$(DESTDIR) -lcryptopp

# Convenience script to run signing program
macx:SIGN_CMD  = export DYLD_LIBRARY_PATH=$$PWD/../libxlr ; $$PWD/tao_sign \\\"\\\$$@\\\"
linux-g++*:SIGN_CMD = export LD_LIBRARY_PATH=$$PWD/../libxlr ; $$PWD/tao_sign \\\"\\\$$@\\\"
win32 {
  HERE = $$system(bash -c 'pwd | sed \'s@\\([a-zA-Z]\\):@/\\1@\'')
  # This is one of the most convoluted script/make/qmake line I've ever written...
  SIGN_CMD = export PATH=\\\"\\\$$PATH:$$HERE/../libxlr/\\\"\$(DESTDIR); \\\"$$HERE/\\\"\$(DESTDIR_TARGET) \\\"\\\$$@\\\"
}
QMAKE_CLEAN += tao_sign.sh
QMAKE_POST_LINK = echo \"$$SIGN_CMD\" > tao_sign.sh && chmod +x tao_sign.sh  # Does not really belong to post-link, but it works


# REVISIT Move into tao.pro
# "make install" will generate and copy a temporary licence (licence.taokey)
# or just an unsigned licence template (licence.taokey.notsigned), depending
# on project settings: LICENSE_VALIDITY, TAO_EDITION.

include(expires.pri)
!isEmpty(EXPIRES_LINE):SIGN=1
contains(TAO_EDITION, Discovery):SIGN=1
!isEmpty(TAO_EDITION):EDITION_STR="$$TAO_EDITION "
LATEST_TAG=$$system(git describe --tags --abbrev=0)
LATEST_TAG=$$replace(LATEST_TAG, \\., \\.)
!build_pass:message(---)
!isEmpty(LATEST_TAG) {
  FEATURES = \"Tao Presentations $${EDITION_STR}$${LATEST_TAG}.*\"  # Allows any version based on current tag
  !build_pass {
    message(--- We will install the following licence:)
    message(---)
    !isEmpty(EXPIRES):message(--- expires $$EXPIRES)
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
QMAKE_SUBSTITUTES += licence.taokey.notsigned.in


!isEmpty(SIGN) {
  # Sign and install licence
  macx:DEP = $$TARGET
  linux-g++*:DEP = $$TARGET
  win32:DEP = \$(DESTDIR_TARGET)

  licence.target = licence.taokey
  licence.commands = cp licence.taokey.notsigned licence.taokey ; ./tao_sign.sh licence.taokey || rm licence.taokey ; cp licence.taokey \"$$APPINST/licenses\"
  licence.files = licence.taokey
  licence.path = $$APPINST/licenses
  licence.depends = $$DEP

  INSTALLS += licence
  QMAKE_EXTRA_TARGETS += licence
  QMAKE_CLEAN += licence.taokey
  QMAKE_DISTCLEAN += licence.taokey.notsigned
} else {
  # Install unsigned template
  licence.files = licence.taokey.notsigned
  licence.path = $$APPINST/licenses
  INSTALLS += licence
}
