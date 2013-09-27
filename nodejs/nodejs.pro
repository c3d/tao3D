# ******************************************************************************
# nodejs.pro                                                         Tao project
# ******************************************************************************
# File Description:
# Qt project file to download and install NodeJS
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2013 Jerome Forissier <jerome@taodyne.com>
# (C) 2013 Taodyne SAS
# ******************************************************************************

include(../main_defs.pri)

TEMPLATE = subdirs

macx {
  URL = http://nodejs.org/dist/v0.10.18/node-v0.10.18-darwin-x64.tar.gz
  MD5 = 0aa4270c3c6b2907041ac0d4e1d06731

  ARCHIVE = $$basename(URL)
  TMP = $${ARCHIVE}_
  fetch.target = $$ARCHIVE
  fetch.commands = curl -o $$TMP $$URL && md5 $$TMP | grep $$MD5 >/dev/null && mv $$TMP $$ARCHIVE
  QMAKE_EXTRA_TARGETS += fetch

  DIR = $$replace(ARCHIVE, .tar.gz, )
  install.path = $$APPINST/nodejs
  install.depends = $$ARCHIVE
  install.commands = tar zxf $$ARCHIVE --strip-components 1 -C \"$$APPINST/nodejs\"
  INSTALLS += install

  QMAKE_CLEAN += $$TMP
}

win32 {
  URL = http://nodejs.org/dist/v0.10.18/node.exe
  MD5 = c7e6c1de34c31c08e6e1d0e4f237db54

  fetch.target = node.exe
  fetch.commands = curl -o node.exe_ $$URL && md5sum node.exe_ | grep -q $$MD5 && mv node.exe_ node.exe
  QMAKE_EXTRA_TARGETS += fetch

  QMAKE_STRIP = :  # Prevent make install from stripping node.exe
  install.path = $$APPINST/nodejs/bin
  install.files = node.exe
  install.depends = node.exe
  INSTALLS += install

  QMAKE_CLEAN += node.exe_
}
