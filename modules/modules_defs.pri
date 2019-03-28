# ******************************************************************************
# modules_defs.pri                                                 Tao3D project
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
# (C) 2010-2013, Jérôme Forissier <jerome@taodyne.com>
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
# Define useful variables for modules

isEmpty(TAO_SDK) {
  TAOTOPSRC = $$PWD/..
} else {
  TAOTOPSRC = $${TAO_SDK}
}

include(../main_defs.pri)

!exists(../tao.pro):IS_SDK=true

# Try to install into (in this order, depending on what variable is defined):
# 1. $$MODINSTPATH
# 2. $$MODINSTROOT/$$MODINSTDIR
# 3. <Tao application install dir, or user's dir>/$$MODINSTDIR
isEmpty(MODINSTPATH) {
  isEmpty(MODINSTROOT) {
    equals(IS_SDK, true) {
      # We don't have the full source, only the module SDK
      # In this case we want "make install" to install modules
      # directly into the user's module directory
      win32 {
        LOCALAPPDATA=$$system(bash -c \"cd $LOCALAPPDATA ; pwd\") # convert Win path to Mingw path
        MODINSTROOT = "$${LOCALAPPDATA}/Taodyne/Tao3D/modules"
      }
      macx:MODINSTROOT = $$(HOME)"/Library/Application Support/Taodyne/Tao3D/modules"
      linux*:MODINSTROOT = $$(HOME)"/.local/share/data/Taodyne/Tao3D/modules"
      isEmpty(MODINSTROOT):error(MODINSTROOT not defined)
    } else {
      # Building full Tao including its bundled modules
      isEmpty(APPINST):error(APPINST not defined)
      MODINSTROOT = $${APPINST}/modules
    }
  }
  isEmpty(MODINSTDIR):error(MODINSTDIR not defined)
  MODINSTPATH      = $${MODINSTROOT}/$$MODINSTDIR
}

isEmpty(FIX_QT_REFS):FIX_QT_REFS = $$PWD/fix_qt_refs
