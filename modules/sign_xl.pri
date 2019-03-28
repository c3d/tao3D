# ******************************************************************************
# sign_xl.pri                                                      Tao3D project
# ******************************************************************************
#
# File description:
#
#
#
#
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2013, Jérôme Forissier <jerome@taodyne.com>
# ******************************************************************************
# This file is part of Tao3D
#
# Tao3D is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Tao3D is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Tao3D, in a file named COPYING.
# If not, see <https://www.gnu.org/licenses/>.
# ******************************************************************************
# Signature of XL source files with a Taodyne key
#
# Create a signature file (.xl.sig) from XL source files (.xl).
# Resulting files are automatically added to INSTALLS.
#
# Usage: In module_name/module_name.pro:
#   SIGN_XL_SOURCES += filename.xl
#   include(sign_xl.pri)

isEmpty(SIGN_XL_INSTPATH):SIGN_XL_INSTPATH=$$MODINSTPATH
isEmpty(SIGN_XL_INSTPATH):error(Please define SIGN_XL_INSTPATH or include modules.pri before sign_xl.pri)

!isEmpty(SIGN_XL_SOURCES):isEmpty(NO_DOC_SIGNATURE) {
  for(file, SIGN_XL_SOURCES) {
    target = $${file}.sig
    eval($${target}.path = \$\$SIGN_XL_INSTPATH)
    exists(../tao_sign/tao_sign.sh) {
      eval($${target}.commands = $$TAOTOPSRC/tao_sign/tao_sign.sh -r $$file)
    }
    eval($${target}.files = $$target)
    eval($${target}.depends = $$file)
    eval($${target}.CONFIG = no_check_exist)
    QMAKE_EXTRA_TARGETS *= $${target}
    INSTALLS *= $${target}
    QMAKE_CLEAN *= $${target}
  }
}
