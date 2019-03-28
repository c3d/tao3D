# ******************************************************************************
# translations.pri                                                 Tao3D project
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
# (C) 2017,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2011, Jérôme Forissier <jerome@taodyne.com>
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
# We need bash and either lupdate, lupdate-qt5 or lupdate-qt4
!system(bash -c \"bash --version >/dev/null\"):error("Can't execute bash")
system(bash -c \"lupdate -help >/dev/null 2>&1\") {
  !build_pass:message(Found lupdate)
  LUPDATE=lupdate
} else {
  system(bash -c \"lupdate-qt5 -help >/dev/null 2>&1\") {
    !build_pass:message(Found lupdate-qt5)
    LUPDATE=lupdate-qt5
  } else {
    system(bash -c \"lupdate-qt4 -help >/dev/null 2>&1\") {
      !build_pass:message(Found lupdate-qt4)
      LUPDATE=lupdate-qt4
    }
    else {
     error("Can't execute lupdate, lupdate-qt5 nor lupdate-qt4")
    }
  }
}

# Translations: convenience targets "make lupdate" and "make lrelease"
# Include this file AFTER relevant variables have been defined
lupdate.commands = $$LUPDATE -verbose $$SOURCES $$HEADERS $$FORMS $$NOWARN_SOURCES -ts $$TRANSLATIONS || lupdate -verbose $$SOURCES $$HEADERS $$FORMS $$CXXTBL_SOURCES $$NOWARN_SOURCES -ts $$TRANSLATIONS

lupdate.depends  = FORCE
QMAKE_EXTRA_TARGETS += lupdate
lrelease.commands = lrelease $$TRANSLATIONS
lrelease.depends  = FORCE
QMAKE_EXTRA_TARGETS += lrelease
