# ******************************************************************************
# custom_slides.pro                                                Tao3D project
# ******************************************************************************
#
# File description:
# Project file for document template: Movie

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/custom_slides
files.files = template.ini custom_slides.ddd theme.xl custom_slides.png

images.path = $$APPINST/templates/custom_slides/images
images.files = images/*

INSTALLS += files images
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2011, Catherine Burvelle <catherine@taodyne.com>
# (C) 2011,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2011, Soulisse Baptiste <soulisse@polytech.unice.fr>
# ******************************************************************************
# This file is part of Tao3D
#
# Tao3D is free software: you can r redistribute it and/or modify
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
# File Description:
# Project file for document template: Movie

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/custom_slides
files.files = template.ini custom_slides.ddd theme.xl custom_slides.png

images.path = $$APPINST/templates/custom_slides/images
images.files = images/*

INSTALLS += files images
