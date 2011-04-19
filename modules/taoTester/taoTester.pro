
# ******************************************************************************
#  taoTester.pro                                                  Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Tao Test module
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2010 Catherine Burvelle <cathy@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************

MODINSTDIR = taoTester

include(../modules.pri)

release:QMAKE_CXXFLAGS -= -Werror  # Workaround QTBUG-14437 (GCC 4.4)

TBL_SOURCES = taoTester.tbl

QT += core gui opengl testlib

OTHER_FILES += \
    taoTester.tbl \
    taoTester.xl

HEADERS += \
    widgettests.h \
    taotester.h \
    save_test_dialog.h \
    traces.tbl

SOURCES += \
    widgettests.cpp \
    taotester.cpp \
    save_test_dialog.cpp

FORMS += \
    save_test_dialog.ui
