# ******************************************************************************
# crypt_xl.pri                                                     Tao3D project
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
# (C) 2011,2013, Jérôme Forissier <jerome@taodyne.com>
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
# Encryption of XL source files
#
# Convert XL source files (.xl) into an encrypted form (.xl.crypt).
# Resulting files are automatically added to INSTALLS.
#
# Usage: In module_name/module_name.pro:
#   CRYPT_XL_SOURCES = filename.xl
#   include(crypt_xl.pri)

isEmpty(MODINSTPATH):error(Please include modules.pri before crypt_xl.pri)

!isEmpty(CRYPT_XL_SOURCES) {
  for(file, CRYPT_XL_SOURCES) {
    target = $${file}.crypt
    eval($${target}.path = \$\$MODINSTPATH)
    exists(../crypt/crypt.sh) {
      eval($${target}.commands = $$TAOTOPSRC/crypt/crypt.sh <$$file >$$target)
    } else {
      warning(Not encrypting: $$file)
      eval($${target}.commands = cp $$file $$target)
    }
    eval($${target}.files = $$target)
    eval($${target}.depends = $$file)
    eval($${target}.CONFIG = no_check_exist)
    QMAKE_EXTRA_TARGETS *= $${target}
    INSTALLS *= $${target}
    QMAKE_CLEAN *= $${target}

    SIGN_XL_SOURCES += $$target
  }
  include(sign_xl.pri)
}
