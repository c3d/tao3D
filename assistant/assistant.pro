# ******************************************************************************
#  assistant.pro                                                    Tao project
# ******************************************************************************
# File Description:
# Project file to install a copy of the Qt Assistant application with Tao
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

include(../main.pri)

TEMPLATE = subdirs
SUBDIRS =


macx {
  isEmpty(ASSISTANT):ASSISTANT=/Developer/Applications/Qt/Assistant.app
  !exists($$ASSISTANT) {
    warning(Qt Assistant application not found in $$ASSISTANT)
    warning(You may run configure or qmake with ASSISTANT=<path>)
    error(Qt Assistant is required)
  }
  ASSISTANT_APP=$$basename(ASSISTANT)
  EXE = $$ASSISTANT_APP/Contents/MacOS/Assistant

  instapp.path = $$APPINST
  instapp.commands = cp -R \"$$ASSISTANT\" \"$$APPINST\" ; sh ../fix_qt_refs_app \"$$APPINST/$$EXE\"
  instapp.depends = FORCE
  QMAKE_EXTRA_TARGETS = instapp
  INSTALLS += instapp

#  qtconf.path = $$APPINST/$$ASSISTANT_APP/Contents/Resources
#  qtconf.files = qt.conf
#  qtconf.depends = instapp
#  INSTALLS += qtconf

  translations.path = $$APPINST/$$ASSISTANT_APP/Contents
  translations.files = $$ASSISTANT/../translations/assistant_fr.qm
  translations.depends = instapp
  INSTALLS += translations
}
