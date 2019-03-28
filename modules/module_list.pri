# ******************************************************************************
# module_list.pri                                                  Tao3D project
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
# (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
# (C) 2011-2013, Catherine Burvelle <catherine@taodyne.com>
# (C) 2011-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2011-2014, Jérôme Forissier <jerome@taodyne.com>
# (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
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
# Defines the list of modules to build.
# Defaults can be changed with "qmake modules=..." (see below)

# Default module list

# Default modules = all sub-directories
LSOUT=$$system(ls)
for(f, LSOUT):exists($${f}/$${f}.pro):DEFAULT_MODULES += $$f

# Add all themes
THEMES=$$system(ls themes)
for(f, THEMES):exists(themes/$${f}/$${f}.pro):DEFAULT_MODULES += themes/$$f

# Add all shaders
THEMES=$$system(ls shaders)
for(f, THEMES):exists(shaders/$${f}/$${f}.pro):DEFAULT_MODULES += shaders/$$f

# No other modules
OTHER_MODULES =

# Process qmake command line variable: 'modules'
# Allows to add/remove modules to/from the default value above
# Resulting list is stored in MODULES
#   modules=none
#     Do not build any Tao module
#   modules=all
#     Build all Tao modules ($$DEFAULT_MODULES and $$OTHER_MODULES)
#   modules=+my_module
#     Add my_module to default module list
#   modules=-my_module
#     Remove my_modules from default module list
#   modules="all -my_module"
#     Build all modules except my_module
#   modules="none +my_module"
#     Build only my_module
MODULES = $$DEFAULT_MODULES
!isEmpty(modules) {
    options = $$modules
    for(opt, options) {
        equals(opt, "all") {
            MODULES = $$DEFAULT_MODULES $$OTHER_MODULES
        }
        equals(opt, "none") {
            MODULES =
        }
        remove = $$find(opt, ^-.*)
        !isEmpty(remove) {
            remove ~= s/-//
            MODULES -= $$remove
        }
        add = $$find(opt, ^\\+.*)
        !isEmpty(add) {
            add ~= s/\\+//
            exists($$add) {
                MODULES += $$add
            } else {
                warning(Module directory not found: $$add -- not added)
            }
        }
    }
}
