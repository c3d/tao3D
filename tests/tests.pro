# ******************************************************************************
# tests.pro                                                        Tao3D project
# ******************************************************************************
#
# File description:
# Qt build file that generate tests
#
#
#
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2010-2012, Catherine Burvelle <catherine@taodyne.com>
# (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
# (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
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
TEMPLATE = subdirs

tests.commands = ./runAllTest.sh TEST
refs.commands =  ./runAllTest.sh REF

clean_runs.commands = ./clean_runs.sh RUN
clean_refs.commands = ./clean_runs.sh REF
clean_all.commands  = ./clean_runs.sh ALL

QMAKE_EXTRA_TARGETS += tests refs clean_runs clean_refs clean_all

OTHER_FILES += \
    runAllTest.sh \
    clean_runs.sh \
    rc.expect \
    result_style.xl \
    fontRendering/fontRendering.ddd \
    fontRendering/fontRendering_test.xl \
    fontRendering/fontRendering_0.png \
    fontRendering/fontRendering_1.png \
    fontRendering/fontRendering_2.png \
    textJustification/textJustification.ddd \
    textJustification/textJustification_0.png \
    textJustification/textJustification_1.png \
    textJustification/textJustification_2.png \
    textJustification/textJustification_3.png \
    textJustification/textJustification_4.png \
    textJustification/textJustification_5.png \
    textJustification/textJustification_test.xl \
    fontTextured/fontTextured.ddd \
    fontTextured/fontTextured_0.png \
    fontTextured/fontTextured_test.xl \
    shapes/shapes.ddd \
    shapes/shapes_0.png \
    shapes/shapes_1.png \
    shapes/shapes_2.png \
    shapes/shapes_3.png \
    shapes/shapes_4.png \
    shapes/shapes_5.png \
    shapes/shapes_test.xl \
    shapes/tortue.jpg

