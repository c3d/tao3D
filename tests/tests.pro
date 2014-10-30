# ******************************************************************************
#  tests.pro                                                         Tao project
# ******************************************************************************
# File Description:
# Qt build file that generate tests
# ******************************************************************************
# This software is licensed under the GNU General Public License v3.
# See file COPYING for details.
# (C) 2010 Catherine Burvelle <catherine@taodyne.com>
# (C) 2010 Christophe de Dinechin <christophe@taodyne.com>
# (C) 2010 Taodyne SAS
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

