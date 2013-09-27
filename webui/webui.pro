# ******************************************************************************
# webui.pro                                                          Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Tao Presentations Web User Interface
# ******************************************************************************
# GNU General Public License Usage
# This file may be used under the terms of the GNU General Public License
# version 3.0 as published by the Free Software Foundation and appearing in the
# file LICENSE included in the packaging of this file.
#
# Please review the following information to ensure the GNU General Public
# License version 3.0 requirements will be met:
# http://www.gnu.org/copyleft/gpl.html.
#
# (C) 2013 Jerome Forissier <jerome@taodyne.com>
# (C) 2013 Taodyne SAS
# ******************************************************************************

!build_pass:!system(bash -c \"npm -v\" >/dev/null) {
  error(npm not found. Did you install NodeJS? \
        [\'sudo apt-get install nodejs\' or \'sudo port install nodejs\' \
        or from http://nodejs.org/])
}
!build_pass:!exists(webui/www/ext-4) {
  message("ExtJS not found under webui/www/ext-4.")
  message("Please, do one of the following:")
  message("- Get ExtJS with 'git clone':")
  message(" cd webui/webui/www && git clone io.taodyne.com:~jerome/Work/extjs ext-4")
  message("- Or, download ExtJS from http://www.sencha.com/products/extjs/download")
  message("- Or disable this sub-project when configuring Tao:")
  message(" ./configure NO_WEBUI=1")
  error("Exiting.")
}

include(../main.pri)

TEMPLATE = subdirs

install.path = $$APPINST/webui
install.commands = mkdir -p \"$$APPINST/webui\" ; make -C webui -f Makefile.all install DEST=\"$$APPINST/webui/\"

INSTALLS += install
