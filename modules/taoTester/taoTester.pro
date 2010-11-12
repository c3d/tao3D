
# ******************************************************************************
#  taoTester.pro                                                  Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Tao Test module
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 2010 Catherine Burvelle <cathy@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************

MODINSTDIR = taoTester

include(../modules.pri)

TBL_SOURCES = taoTester.tbl

QT += core gui opengl testlib

OTHER_FILES += \
    taoTester.tbl \
    module.xl

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
