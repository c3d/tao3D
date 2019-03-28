# ******************************************************************************
# simple_slides.pro                                                Tao3D project
# ******************************************************************************
#
# File description:
# Project file for document template: Simple Slides

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/simple_slides
files.files = template.ini simple_slides.ddd theme.xl simple_slides.jpg

images.path = $$APPINST/templates/simple_slides/images
images.files = images/*

INSTALLS += files images

SIGN_XL_SOURCES = simple_slides.ddd theme.xl
include(../sign_template.pri)
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
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
# File Description:
# Project file for document template: Simple Slides

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/simple_slides
files.files = template.ini simple_slides.ddd theme.xl simple_slides.jpg

images.path = $$APPINST/templates/simple_slides/images
images.files = images/*

INSTALLS += files images

SIGN_XL_SOURCES = simple_slides.ddd theme.xl
include(../sign_template.pri)
