# ******************************************************************************
# quiz.pro                                                         Tao3D project
# ******************************************************************************
#
# File description:
# Project file for document template: Tao quiz

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/quiz
files.files = template.ini Quiz.ddd quiz.png quiz-config.xl theme-quiz.xl

images.path = $$APPINST/templates/quiz/images
images.files = images/logo_taoquiz.png

INSTALLS += files images

SIGN_XL_SOURCES = Quiz.ddd quiz-config.xl theme-quiz.xl
include(../sign_template.pri)
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2013, Catherine Burvelle <catherine@taodyne.com>
# (C) 2019, Christophe de Dinechin <christophe@dinechin.org>
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
# Project file for document template: Tao quiz

TEMPLATE = subdirs

include(../templates.pri)

files.path  = $$APPINST/templates/quiz
files.files = template.ini Quiz.ddd quiz.png quiz-config.xl theme-quiz.xl

images.path = $$APPINST/templates/quiz/images
images.files = images/logo_taoquiz.png

INSTALLS += files images

SIGN_XL_SOURCES = Quiz.ddd quiz-config.xl theme-quiz.xl
include(../sign_template.pri)
