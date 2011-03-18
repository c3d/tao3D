# ******************************************************************************
#  tests.pro                                                         Tao project
# ******************************************************************************
# File Description:
# Qt build file that generate tests
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2010 Catherine Burvelle <catherine@taodyne.com>
# (C) 2010 Christophe de Dinechin <christophe@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************
TEMPLATE = subdirs

tests.commands = ./runAllTest.sh

QMAKE_EXTRA_TARGETS += tests

OTHER_FILES += \
    runAllTest.sh \
    result_style.xl \
    fontRendering/fontRendering.ddd \
    fontRendering/fontRendering_test.ddd \
    fontRendering/fontRendering_before.png \
    fontRendering/fontRendering_0.png \
    fontRendering/fontRendering_1.png \
    fontRendering/fontRendering_after.png \
    textJustification/textJustification.ddd \
    textJustification/textJustification_0.png \
    textJustification/textJustification_1.png \
    textJustification/textJustification_2.png \
    textJustification/textJustification_3.png \
    textJustification/textJustification_after.png \
    textJustification/textJustification_before.png \
    textJustification/textJustification_test.ddd \
    fontTextured/fontTextured.ddd \
    fontTextured/fontTextured_after.png \
    fontTextured/fontTextured_test.ddd \
    textEdition/textEdition.ddd-orig
